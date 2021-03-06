#include "query.h"

#include <vector>
#include <sstream>
#include <algorithm>
#include <queue>
#include <iterator>
#include <functional>
#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <cstdlib>

#include <z3++.h>

//#define LOG_REFINEMENT_LAYER_GRAPH

namespace SpeedyFOIL {

namespace {

std::vector<int> extract_bv_values(z3::expr& E) {
	std::vector<int> v;
	std::string app_name = E.decl().name().str();
	if(app_name == "and"){
		// tuple with multiple fields, arity > 1
		//std::cout << "extract_bv_value: " << E  << std::endl;
			int m_args = E.num_args();
			for (int j = 0; j < m_args; ++j) {
				z3::expr var_val = E.arg(j);
				z3::expr val = var_val.arg(1);
				v.push_back( val.get_numeral_int() );
			}
	}
	else {
		// tuple with only one field, arity = 1
		//std::cout << "extract_bv_values, app_name= " << app_name << std::endl;

		z3::expr val = E.arg(1);
		v.push_back(val.get_numeral_int() );
	}



	return v;
}

const char* str(int x) {
	std::stringstream ss;
	ss << "v" << x;
	return ss.str().c_str();
}

} // end of anonymous namespace


FixedPoint::FixedPoint(ContextManager* pCM) : pContext(&(pCM->C)), defaultS(pContext->str_symbol("defaultS")) {
	z3_fp = Z3_mk_fixedpoint( *pContext );
	Z3_fixedpoint_inc_ref(*pContext,z3_fp);

	recent = Z3_L_UNDEF;
	// register relation definitions
	for (auto pr : pCM->funcMap) {
		Z3_fixedpoint_register_relation(*pContext, z3_fp, pr.second);
	}

	pCM->appendEDBConstr(z3_fp);

	prog_id = -1;

}

void FixedPoint::add_rules(std::vector<z3::expr>& rules) {
	for (z3::expr& r : rules) {
		Z3_fixedpoint_add_rule(*pContext, z3_fp, r, defaultS);
	}
}

void FixedPoint::add_rule(z3::expr rule) {
	Z3_fixedpoint_add_rule(*pContext, z3_fp, rule, defaultS);
}

bool FixedPoint::query(Z3_ast Q) {
	recent = Z3_fixedpoint_query( *pContext, z3_fp, Q);
	if(recent == Z3_L_UNDEF) {
		std::cerr << "query result is UNDEF" << std::endl;
	}
	return recent == Z3_L_TRUE;
}

z3::expr FixedPoint::get_answer(){
	if(recent != Z3_L_TRUE) {
		std::cerr << "ERROR: cannot get_answer unless recent status is true" << std::endl;
	}

	Z3_ast ast_res = Z3_fixedpoint_get_answer( *pContext, z3_fp);
	z3::expr detailed_res( *pContext, ast_res);

	return detailed_res;
}

z3::expr QueryEngine::build_func_constr(z3::context& context,
		std::map<int, z3::expr>& z3_vars,
		std::pair<Relation, std::vector<int>>& pair) {

	Relation rel = pair.first;

	auto it = cm_ptr->funcMap.find(rel);
	if (it == cm_ptr->funcMap.end()) {
		std::cerr << "ERROR, QueryEngine::helper got unknown relation : "
				<< rel->Name << std::endl;
	}

	z3::func_decl f = it->second;

	z3::expr_vector params(context);
	for (int x : pair.second) {
		auto it2 = z3_vars.find(x);
		if (it2 == z3_vars.end()) {
			std::cerr << "ERROR, QueryEngine::helper got undefined var: "
					<< str(x) << std::endl;
		}

		params.push_back(it2->second);
	}

	return f(params);
}

void QueryEngine::parse_and_update(z3::expr& E, int idx, int prog_id, bool cancelVote) {
	//std::cout << "parse_and_update, E=" << E << std::endl;
	std::string app_name = E.decl().name().str();
	if (app_name == "or") {
		int n_args = E.num_args();
		for (int i = 0; i < n_args; ++i) {
			z3::expr e = E.arg(i);
			std::vector<int> tp = extract_bv_values(e);

			// prepend idx
			tp.insert(tp.begin(), idx);
			if(cancelVote){
				//--vote_stats[tp];
				vote_stats[tp].erase(prog_id);
			}
			else{
				//++vote_stats[tp];
				vote_stats[tp].insert(prog_id);
			}

			//std::cout <<"tp: ";
			//std::copy(tp.begin(), tp.end(), std::ostream_iterator<int>(std::cout, ","));
			//std::cout << std::endl;
		}
	} else if (app_name == "and" || app_name == "=") {
		std::vector<int> tp = extract_bv_values(E);
		tp.insert(tp.begin(), idx);
		if(cancelVote){
			//--vote_stats[tp];
			vote_stats[tp].erase(prog_id);
		}
		else{
			//++vote_stats[tp];
			vote_stats[tp].insert(prog_id);
		}
	} else {
		std::cerr << "unknown application name: " << app_name << std::endl;
	}
}


z3::expr QueryEngine::construct_query(Relation rel){
	z3::context& context = cm_ptr->C;
	z3::sort bv_sort = context.bv_sort(cm_ptr->MaxBits);

	z3::func_decl f = cm_ptr->funcMap.find(rel)->second;
	const int arity = rel->Arity;

	z3::expr_vector ex_vs(context);
	for (int x=0; x < arity; ++x) {
		z3::expr vx = context.constant(str(x), bv_sort);
		ex_vs.push_back(vx);
	}

	return z3::exists(ex_vs, f(ex_vs));
}

void QueryEngine::queryIDBs(FixedPoint& fp, bool cancelVote) {

	const size_t sz = dp_ptr->idbRules.size();

	for(int idb_index = 0; idb_index < sz; ++idb_index){
		Relation rel = dp_ptr->idbRules[ idb_index ].rel.pRel;
		if ( fp.query( construct_query(rel) ) ){
			z3::expr detailed_res = fp.get_answer();
			parse_and_update(detailed_res, idb_index, fp.prog_id, cancelVote);
		}
		else {
			if(!cancelVote){
				++warn_ct;
			}
		}
	}

}


FixedPoint QueryEngine::prepare(const DatalogProgram & dp) {
	z3::context& context = cm_ptr->C;
	z3::sort bv_sort = context.bv_sort(cm_ptr->MaxBits);

	std::vector<z3::expr> z3_rs;

	std::vector<ConcreteRule> rs = dp_ptr->getConcreteRules( dp );

	for (ConcreteRule& r : rs) {
		// define vars
		std::set<int> vars;
		for (auto pr : r) {
			for (int x : pr.second) {
				vars.insert(x);
			}
		}

		std::map<int, z3::expr> z3_vars;
		z3::expr_vector all_vs(context);
		for (int x : vars) {
			//z3::expr va = c.constant("a", bv3);
			z3::expr vx = context.constant(str(x), bv_sort);
			z3_vars.insert( std::make_pair(x, vx) );
			all_vs.push_back(vx);
		}

		z3::expr head = build_func_constr(context, z3_vars, r[0]);

		//queries.insert(r[0]);

		z3::expr_vector ev(context);
		const size_t n = r.size();
		for (int i = 1; i < n; ++i) {
			z3::expr e = build_func_constr(context, z3_vars, r[i]);
			ev.push_back(e);
		}
		// z3::expr r2 = z3::forall(va, vb, vc, z3::implies( z3::mk_and(ev), path(va,vc) ));

		z3::expr rule = z3::forall(all_vs, z3::implies(z3::mk_and(ev), head));

		z3_rs.push_back(rule);
	}

	FixedPoint fp(cm_ptr);
	fp.add_rules(z3_rs);
	fp.set_prog_id(dp.prog_id);

	return fp;
}

bool QueryEngine::execute(const DatalogProgram& dp) {

    //std::cout << "execute, prog_id = " << dp.prog_id << std::endl;
  
	bool status = true;

	FixedPoint fp = prepare(dp);
	const int before = warn_ct;
	queryIDBs(fp);
	const int after = warn_ct;
	if(before != after && !random_mode && !(dp_ptr->enableX)) {
		fail_to_derive.insert( dp.prog_id );
		//std::cout << "will cancel vote for program: \n" << dp_ptr->str(dp) << std::endl;
		queryIDBs(fp, true);

		status = false;
	}

	return status;
}

z3::expr QueryEngine::convert_question(std::vector<int>& Q){
	const int idb_index = Q[0];
	Relation rel = dp_ptr->idbRules[idb_index].rel.pRel;
	auto it = cm_ptr->funcMap.find(rel);
	if (it == cm_ptr->funcMap.end()) {
		std::cerr << "ERROR, QueryEngine::test got unknown relation : "
				<< rel->Name << std::endl;
	}
	z3::func_decl f = it->second;


	z3::context& context = cm_ptr->C;
	z3::expr_vector params( context);
	const size_t sz = Q.size();
	for(int i=1; i< sz; ++i) {
		z3::expr val = context.bv_val( Q[i] ,cm_ptr->MaxBits);
		params.push_back(val);
	}

	return f(params);
}

bool QueryEngine::test(const DatalogProgram& dp, z3::expr Q) {
	FixedPoint fp = prepare(dp);
	return fp.query( Q );
}

bool QueryEngine::test_converge_old() {
	vote_stats.clear();
	int ct = 0;
	std::vector<const DatalogProgram*> vp;
	for(const DatalogProgram& p : dp_ptr->Ss) {
		vp.push_back( &p);
	}
	for(const DatalogProgram& p : dp_ptr->Gs) {
		vp.push_back( &p);
	}

	for(const DatalogProgram* p : vp) {
		++ct;
		execute(*p);

		for(auto pr : vote_stats) {
			if(pr.second.empty()) {
				continue;
			}
			if(pr.second.size() != ct){
				return false;
			}
		}
	}

	return true;
}
std::vector<int> QueryEngine::random_pick_old(){
	std::vector<int> res;

	// test converge
	if( test_converge() ) {
		return res;
	}

	while (res.size() == 0) {
		const int idb_index = std::rand() % dp_ptr->idbRules.size();
		const IDBTR& idb = dp_ptr->idbRules[idb_index];
		const int S = idb.rel.getSpace();
		const int K = std::rand() % S;

		auto pr = std::make_pair(idb_index, K);
		if (random_picked.find(pr) == random_picked.end()) {
			random_picked.insert(pr);
			res = idb.rel.getKthTuple(K);
			res.insert(res.begin(), idb_index);
		}
	}



	return res;
}

bool QueryEngine::test_converge() {
	int ct = -1;

	for(const auto & it : vote_stats ) {
		if(it.second.empty()) {
			continue;
		}
		if(ct == -1){
			ct = it.second.size();
		}
		if(ct != it.second.size()){
			return false;
		}
	}
	return true;
}

std::vector<int> QueryEngine::random_pick(){
	std::vector<int> res;
	std::vector< std::vector<int> > pool;

	// test converge
	if( test_converge() ) {
		return res;
	}

	for(const auto & it : vote_stats ) {
		if(it.second.empty()) {
			continue;
		}
		pool.push_back(it.first);
	}

	if(pool.size()) {
		const int index = std::rand() % pool.size();
		return pool[index];
	}

	return res;
}

void QueryEngine::execute_one_round_helper(std::vector<DatalogProgram>& progs) {
	auto it = progs.begin();
	while(it != progs.end()) {
		execute(*it);
		++it;
	}

	int cancel_ct = 0;
	std::vector<DatalogProgram> dps;
	for(DatalogProgram& x :progs) {
		if(fail_to_derive.find(x.prog_id) == fail_to_derive.end()) {
			dps.push_back( std::move(x) );
		}
		else{
			++cancel_ct;
		}
	}

	progs =std::move(dps);

	std::cout << "cancel votes for " << cancel_ct << " programs, now size: " << progs.size() << std::endl;
}

bool QueryEngine::validate_with_full_IDBs() {
	bool succ = true;
	for (auto pr : vote_stats) {
      if(pr.second.empty() ) {
        continue;
      }
		const std::vector<int>& Q = pr.first;
		if(! dp_ptr->ask(Q)){
			std::cout << dp_ptr->nice_display(Q) << " is predicated to be true, but actually false\n";
			succ = false;
		}
	}
	const size_t sz = dp_ptr->idbValues.size();
	for(int i=0; i < sz; ++i) {
		const IDBValue& iv = dp_ptr->idbValues[i];
		for(const std::vector<int>& v : iv.pos) {
			std::vector<int> Q(v.begin(), v.end());
			Q.insert(Q.begin(), i);

			auto it = vote_stats.find(Q);
			if(it == vote_stats.end() || it->second.empty()) {
				std::cout << dp_ptr->nice_display(Q) << " is missing by the committee\n";
				succ = false;
			}
		}
	}
	return succ;
}

std::vector<int> QueryEngine::execute_one_round() {

	fail_to_derive.clear();
	//vote_stats.clear();
	warn_ct = 0;

#ifdef LOG_REFINEMENT_LAYER_GRAPH
	std::vector<int> v;
		v.push_back( it->prog_id );
	layers.push_back(std::move(v));
#endif


	//execute_one_round_helper(dp_ptr->Gs);
	//execute_one_round_helper(dp_ptr->Ss);


	//std::cout << "warn_ct = " << warn_ct << std::endl;
	//std::cout << "\ntuple stats: \n";
	//for (auto pr : vote_stats) {
	//	std::cout << pr.second.size() << " votes for "
	//			<< dp_ptr->nice_display(pr.first) << std::endl;
    //  std::copy(pr.second.begin(), pr.second.end(), std::ostream_iterator<int>(std::cout, ","));
    //  std::cout << std::endl;
	//}


	const int total_votes = (int) (dp_ptr->Gs.size() + dp_ptr->Ss.size());
	const int ideal = total_votes  / 2;
	int best = 1<<30;

	std::cout <<"total_votes: " << total_votes << std::endl;
	//std::cout <<"ideal=" << ideal << ", best=" << best << std::endl;

	//std::vector< std::pair<int, int> > order;
	//int index = 0;

	std::set<int> votes;
	std::vector<int> question;
	for (auto pr : vote_stats) {
      if(pr.second.empty()) {
        continue;
      }
      
		votes.insert(pr.second.size());

		int dis = pr.second.size() - ideal;
		if(dis < 0) dis =-dis;

		//order.push_back( std::make_pair(dis, index) );
		//++index;

		if(dis < best) {

			//std::cout << "update best: dis=" << dis << ", best=" << best << std::endl;
			question = pr.first;
			best = dis;
			//std::cout << "Question becomes: " ;
			//std::copy(question.begin(), question.end(), std::ostream_iterator<int>(std::cout, ", ") );
			//std::cout << std::endl;

		}
	}


	std::cout << "best distance: " << best << std::endl;

	if(votes.size() == 1) {
		if( *votes.begin() == total_votes ) {

			std::cout << "only one size of vote: " << *votes.begin() << ", so converged." << std::endl;

			if( validate_with_full_IDBs() ) {
				std::cout << "validation result: Accept" << std::endl;
			}
			else{
				std::cout << "validation result: Wrong Answer" << std::endl;
			}

			// converged !!!
			return std::vector<int>();
		}
	}


	/*
	std::sort(order.begin(), order.end());
	// find the first positive one
	for(auto pr : order){
		int index = pr.second;
		auto it = vote_stats.begin();
		while(index --) {
			++it;
		}
		std::vector<int> v = it->first;

		if(dp_ptr->ask(v )){
			question = v;
			break;
		}
	}*/


	//std::cout << "Question: " ;
	//std::copy(question.begin(), question.end(), std::ostream_iterator<int>(std::cout, ", ") );
	//std::cout << std::endl;

	return question;
}


bool QueryEngine::test2(const DatalogProgram& dp, const std::vector<int>& Q) {
	auto it = vote_stats.find(Q);
	if(it == vote_stats.end() || it->second.find(dp.prog_id) == it->second.end()) {
		return false;
	}
	return true;
}

std::pair<bool,bool> QueryEngine::test3(DatalogProgram& x, bool positive, std::vector<int>& Q, std::vector<std::vector<int> >& Qs) {
	bool contain_pos = true;
	bool contain_neg = false;
	if(positive) {
		contain_pos = test2(x, Q);
		if (contain_pos) {
			for (std::vector<int>& vi : Qs) {
				if (test2(x, vi)) {
					contain_neg = true;
					break;
				}
			}
		}
	}
	else{
		contain_neg = test2(x, Q);
		if(!contain_neg) {
			for (std::vector<int>& vi : Qs) {
				if (!test2(x, vi)) {
					contain_pos = false;
					break;
				}
			}
		}
	}

	return std::make_pair(contain_pos, contain_neg);
}

void QueryEngine::eliminate_and_refine2(std::vector<DatalogProgram>& A,
		std::vector<DatalogProgram>& B, bool positive,
		std::vector<int>& Q, std::vector<std::vector<int> >& Qs) {

	std::vector<DatalogProgram> dps;
	int remove_ct = 0;

	for (DatalogProgram& x : A) {
		auto res = test3(x, positive, Q, Qs);
		if (res.first && ! res.second) {
			dps.push_back(std::move(x));
		} else {
			++remove_ct;

			// cancel vote
			for(auto& it : vote_stats) {
				it.second.erase( x.prog_id );
			}
		}
	}

	A = std::move(dps);

	int refine_ct = 0;
	for (DatalogProgram& x : B) {
		auto res = test3(x, positive, Q, Qs);
		if (res.first && ! res.second) {
			dps.push_back(std::move(x));
		}
		else{
			// cancel vote
			for(auto& it : vote_stats) {
				it.second.erase( x.prog_id );
			}

			std::queue<DatalogProgram> Queue;
			Queue.push(std::move(x));

			while (!Queue.empty()) {
				++refine_ct;
				DatalogProgram p ( std::move(Queue.front() ));
				Queue.pop();

				bool specialize = !positive;
				std::vector<DatalogProgram> vs = dp_ptr->refineProgWithTarget(p,
						specialize, Q[0]);
				for (DatalogProgram& y : vs) {

					//try to vote
					if(!execute(y)) {
						continue;
					}

					auto res = test3(y, positive, Q, Qs);
					if(positive) {
						if(res.second) {
							// ignore, cancel vote
							for(auto& it : vote_stats) {
								it.second.erase( y.prog_id );
							}

						}
						else if(res.first) {
							dps.push_back(std::move(y));
						}
						else{
							// cancel vote, keep refining
							for(auto& it : vote_stats) {
								it.second.erase( y.prog_id );
							}

							Queue.push(std::move(y));
						}
					}
					else{
						if(res.first) {
							if(res.second){
								// cancel vote, keep refining
								for(auto& it : vote_stats) {
									it.second.erase( y.prog_id );
								}

								Queue.push(std::move(y));
							}
							else{
								dps.push_back(std::move(y));
							}
						}
						else{
							// ignore, cancel vote
							for(auto& it : vote_stats) {
								it.second.erase( y.prog_id );
							}
						}
					}
				}
			}
		}
	}

	B = std::move(dps);

	std::cout << "removed_ct=" << remove_ct << ", refine_ct = " << refine_ct
			<< ", size: " << dp_ptr->Gs.size() << std::endl;

}


void QueryEngine::eliminate_and_refine(std::vector<DatalogProgram>& A,
		std::vector<DatalogProgram>& B, bool positive, z3::expr& pos_qs, z3::expr& neg_qs,
		std::vector<int>& Q) {
	// any general program that cannot cover Q should be eliminated.

	std::vector<DatalogProgram> dps;
	int remove_ct = 0;


	//std::cout << "A.size() = " << A.size() <<

	for (DatalogProgram& x : A) {
		if (test(x, pos_qs) && !test(x, neg_qs)) {
			dps.push_back(std::move(x));
		} else {
			++remove_ct;

			// cancel vote
			FixedPoint fp = prepare(x);
			queryIDBs(fp, true);

			//std::string s = dp_ptr->str(x);
			//long long h = str_hash(s);
			//removedPrograms[h].insert(s);
		}
	}

	A = std::move(dps);

	//std::cout << "Number of programs to eliminate: "
	//		<< remove_ct << ", out of " << A.size()
	//		<< std::endl;
	//std::cout << "dps.size = " << dps.size() << std::endl;
	//std::cout << "A new size: " << A.size() << std::endl;


	// generalize specific program that cannot cover  Q
#ifdef LOG_REFINEMENT_LAYER_GRAPH
	int x_id;
	const int L = layers.size();
#endif

	int refine_ct = 0;
	for (DatalogProgram& x : B) {

#ifdef LOG_REFINEMENT_LAYER_GRAPH
		x_id = x.prog_id;
#endif

		if (test(x, pos_qs) && ! test(x,neg_qs) ) {
			dps.push_back(std::move(x));
		} else {

			// cancel vote
			FixedPoint fp = prepare(x);
			queryIDBs(fp, true);


			//std::cout <<"positive=" << positive << ", going to refine \n" << dp_ptr->str(x);
			//std::cout << "before: refine_ct = " << refine_ct << std::endl;

			std::queue<DatalogProgram> Queue;
			Queue.push(std::move(x));

			while (!Queue.empty()) {
				++refine_ct;

				//std::cout <<"Queue.size = " << Queue.size() << std::endl;

				DatalogProgram p ( std::move(Queue.front() ));
				Queue.pop();

				// here, we actually don't need to test if deleted or not
				// as init/refineProg will never return a visited program

				//std::cout << "\nrefine: \n" << dp_ptr->str(p) << std::endl;
				//std::cout << "refined results: \n";

				bool specialize = !positive;
				std::vector<DatalogProgram> vs = dp_ptr->refineProgWithTarget(p,
						specialize, Q[0]);
				for (DatalogProgram& y : vs) {

#ifdef LOG_REFINEMENT_LAYER_GRAPH
					edges[x_id].insert(std::make_pair(L, y.prog_id));
#endif

					const bool contain_pos = test(y, pos_qs);
					const bool contain_neg = test(y, neg_qs);

					if(positive) {
						// positive example, we now generalize specific program
						if(contain_neg) {
							// ignore
						}
						else if(contain_pos) {
							// vote for the first time
							if(execute(y)) {
								// get expected generalization
								dps.push_back(std::move(y));
							}

						}
						else {
							// keep refining
							Queue.push(std::move(y));
						}
					}
					else {
						// negative example, we now specialize
						if(contain_pos){
							if(contain_neg) {
								// keep refining
								Queue.push( std::move(y) );
							}
							else {
								// vote for the first time
								if(execute(y)) {
									// get expected generalization
									dps.push_back(std::move(y));
								}
							}
						}
					}

					//if (test(y, pos_qs) && !test(y, neg_qs)) {
					//	dps.push_back(std::move(y));
					//} else {
					//	Queue.push(std::move(y));
					//}
				}
			}

			//std::cout << "after: refine_ct = " << refine_ct << std::endl;

		}
	}

	B = std::move(dps);

	std::cout << "removed_ct=" << remove_ct << ", refine_ct = " << refine_ct
			<< ", size: " << dp_ptr->Gs.size() << std::endl;

}

void QueryEngine::work_refinement() {


	z3::expr_vector and_pos_vec(cm_ptr->C);
	z3::expr_vector or_neg_vec (cm_ptr->C);

	std::vector<std::vector<int>> pos_Qs;
	std::vector<std::vector<int>> neg_Qs;


	and_pos_vec.push_back( cm_ptr->C.bool_val(true) );
	or_neg_vec.push_back( cm_ptr->C.bool_val(false) );

	int round = 0;
	while (true) {
		++round;

		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		std::vector<int> Q = random_mode ? random_pick_old() : execute_one_round();

	    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	    std::cout << "Round " << round << " execution takes "
	              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
	              << " ms.\n";

		if(Q.size() == 0) {
			break;
		}

		std::chrono::steady_clock::time_point before_refine = std::chrono::steady_clock::now();

		bool positive = dp_ptr->ask(Q);


		std::cout << (positive ? "positive" : "negative")
				<< " answer for Question: " << dp_ptr->nice_display(Q)
				<< std::endl;

		//std::copy(Q.begin(), Q.end(), std::ostream_iterator<int>(std::cout, ", ") );
		//std::cout << std::endl;

		if (positive) {

			pos_Qs.push_back(Q);

			z3::expr q = convert_question(Q);
			and_pos_vec.push_back(q);
			z3::expr and_qs = z3::mk_and(and_pos_vec);
			z3::expr or_qs = z3::mk_or(or_neg_vec);

			//z3::expr qs = and_qs && (!or_qs);

			std::cout << "before elimination & refinement, Gs.size= "
					<< dp_ptr->Gs.size() << ", Ss.size= " << dp_ptr->Ss.size()
					<< std::endl;

			//eliminate_and_refine( dp_ptr->Gs, dp_ptr->Ss, true, and_qs, or_qs, Q);
			eliminate_and_refine2(dp_ptr->Gs, dp_ptr->Ss, true, Q, neg_Qs);

			std::cout << "after elimination & refinement, Gs.size= "
					<< dp_ptr->Gs.size() << ", Ss.size= " << dp_ptr->Ss.size()
					<< std::endl;

		} else {
			neg_Qs.push_back(Q);

			z3::expr q = convert_question(Q);
			or_neg_vec.push_back(q);
			z3::expr or_qs = z3::mk_or(or_neg_vec);
			z3::expr and_qs = z3::mk_and(and_pos_vec);

			//z3::expr qs = and_qs && (!or_qs);

			std::cout << "before elimination & refinement, Gs.size= "
					<< dp_ptr->Gs.size() << ", Ss.size= " << dp_ptr->Ss.size()
					<< std::endl;


			//eliminate_and_refine( dp_ptr->Ss, dp_ptr->Gs, false, and_qs, or_qs, Q);
			eliminate_and_refine2(dp_ptr->Ss, dp_ptr->Gs, false, Q, pos_Qs);

			std::cout << "after elimination & refinement, Gs.size= "
					<< dp_ptr->Gs.size() << ", Ss.size= " << dp_ptr->Ss.size()
					<< std::endl;

		}

		std::chrono::steady_clock::time_point after_refine = std::chrono::steady_clock::now();

	    std::cout << "Round " << round << " refinement takes "
	              << std::chrono::duration_cast<std::chrono::milliseconds>(after_refine - before_refine).count()
	              << " ms.\n";

	    std::cout << "Round " << round << " overall takes "
	              << std::chrono::duration_cast<std::chrono::milliseconds>(after_refine - start).count()
	              << " ms.\n";

		//break;
	}

	std::set<int> greens;

	std::cout << "converged at round: " << round << std::endl;

	std::cout << "remaining general programs\n";
	for(const DatalogProgram& x : dp_ptr->Gs) {
		greens.insert(x.prog_id);
		std::cout << "\n\n" << dp_ptr->str(x);
	}

	std::cout << "remaining specific programs\n";
	for(const DatalogProgram& x : dp_ptr->Ss) {
		greens.insert(x.prog_id);
		std::cout << "\n\n" << dp_ptr->str(x);
	}

#ifdef LOG_REFINEMENT_LAYER_GRAPH
	draw_layer_graph(greens);
#endif


}



void QueryEngine::work_baseline() {
	int round = 0;
	while (true) {
		++round;

		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		std::vector<int> Q = random_mode ? random_pick() : execute_one_round();

	    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	    std::cout << "Round " << round << " execution takes "
	              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
	              << " ms.\n";

		if(Q.size() == 0) {
			break;
		}

		std::chrono::steady_clock::time_point before_refine = std::chrono::steady_clock::now();

		bool positive = dp_ptr->ask(Q);

		std::cout << (positive ? "positive" : "negative")
				<< " answer for Question: " << dp_ptr->nice_display(Q)
				<< std::endl;

		std::vector<DatalogProgram> dps;
		int remove_ct = 0;

		for (DatalogProgram& x : dp_ptr->Gs) {
			bool res = test2(x, Q);

			if (res == positive) {
				dps.push_back(std::move(x));
			} else {
				++remove_ct;

				// cancel vote
				for (auto& it : vote_stats) {
					it.second.erase(x.prog_id);
				}
			}
		}

		dp_ptr->Gs = std::move(dps);

		std::cout << "removed_ct=" << remove_ct << ", refine_ct = " << 0
				<< ", size: " << dp_ptr->Gs.size() << std::endl;

		std::chrono::steady_clock::time_point after_refine = std::chrono::steady_clock::now();

	    std::cout << "Round " << round << " refinement takes "
	              << std::chrono::duration_cast<std::chrono::milliseconds>(after_refine - before_refine).count()
	              << " ms.\n";

	    std::cout << "Round " << round << " overall takes "
	              << std::chrono::duration_cast<std::chrono::milliseconds>(after_refine - start).count()
	              << " ms.\n";

	}

	std::cout << "Synthesized programs by baseline\n";
	for(const DatalogProgram& x : dp_ptr->Gs) {
		std::cout << "\n\n" << dp_ptr->str(x);
	}

}


void QueryEngine::work() {

	std::chrono::steady_clock::time_point s1 = std::chrono::steady_clock::now();

	if(dp_ptr->enableG || dp_ptr->enableS){
		//std::cout << "will examine IDBTRs ...\n";
		dp_ptr->examine_each_IDBTR(this);
	}

	//std::cout << "\n\nwill initGS ... \n";
	dp_ptr->initGSX();

	std::chrono::steady_clock::time_point s2 = std::chrono::steady_clock::now();

	// Now we use the "at most twice" mechanism
	// execute only once at the beginning
	execute_one_round_helper(dp_ptr->Gs);
	execute_one_round_helper(dp_ptr->Ss);


	if(dp_ptr->enableX) {
		work_baseline();
	}
	else{
		work_refinement();
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "end-s2: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - s2).count()
              << " ms.\n";

	std::cout << "end-s1: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - s1).count()
              << " ms.\n";

}

void QueryEngine::draw_layer_graph(std::set<int>& greens){

	std::string fpath = "layerG.dot";

	std::ofstream fout(fpath);
	fout << "digraph G {\n";


	const size_t L = layers.size();

	//for(std::vector<int>& v : layers){

	for(int i=0; i < L; ++i) {
		std::vector<int>& v = layers[i];

		fout << "  subgraph {\n";
		fout << "    rank = same;";
		for(int x : v) {
			fout <<"layer" << i << "_P" << x << "; ";
		}
		fout << "\n  }\n";
	}


	for(int i = 0; i < L; ++i) {
		std::vector<int>& v = layers[i];
		for(int x : v) {
			std::set< std::pair<int,int> > st = edges[x];
			std::set<int> st2;

			if (i + 1 < L) {
				for (int y : layers[i + 1]) {
					if (x == y) {
						st2.insert(y);
					}
				}
			}


			for(auto pr : st){
				//std::cout <<"layer=" <<i <<", x=" << x << ", pr.first = " << pr.first  << ", pr.second = " << pr.second << std::endl;
				if(i+1 == pr.first) {
					//std::cout <<"x=" << x << ", pr.second = " << pr.second << std::endl;
					st2.insert(pr.second);
				}
			}

			if(st2.size()) {
				fout <<" layer" << i << "_P" << x << " -> {";

				for(int y : st2) {
					fout << " layer" << (i+1) << "_P" << y <<" ";
				}

				fout << "  };\n";
			}
		}
	}

	/*
	for (auto pr : edges) {
		int x = pr.first;
		if (pr.second.size()) {
			fout << "P" << x << " -> {";
			for (int y : pr.second) {
				fout << "P" << y << " ";
			}
			fout << " };\n";
		}
	}*/

	/*
	for (int i = 0; i < sz; ++i) {
		std::string s = templates[i].toStr();
		int j = 0;
		while(j < s.length()){
			if(s[j] == '-') break;
			++j;
		}
		fout << "v" << i << " [label=\"" << s.substr(j+1) << "\"];\n";
	}
    */

	for(int i=0; i < L; ++i) {
		std::vector<int>& v = layers[i];
		for(int x : v){
			if(greens.find(x) != greens.end())
			fout <<"layer" << i << "_P" << x << " [style=filled color=\"yellow\"]; \n";
		}
	}


	fout << "}\n";

	fout.close();

}

} // end of namepsace SpeedyFOIL
