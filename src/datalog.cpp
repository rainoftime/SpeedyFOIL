
#include "datalog.h"
#include "template.h"

#include <map>
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <z3++.h>


namespace SpeedyFOIL {

int DatalogProgram::programCt = 0;

typedef std::map<int,int> PII;

namespace {

bool check_proof(const IClause& g, const IClause& s, const PII& relMap, const PII& varMap) {

	if(g.cl_hd.pRel != s.cl_hd.pRel) {
		std::cout << "head relation does not match: g=" << g.cl_hd.pRel
				<< ", s=" << g.cl_hd.pRel << std::endl;
		return false;
	}

	/*
	for(auto pr : relMap) {
		// incorrect
		if( g.cl_body[pr.first].pRel != s.cl_body[pr.second].pRel){
			std::cout <<pr.first << "-th predicate name does not match " << pr.second << "-th \n";
			return false;
		}
	}*/

	// as variable order directly check

	// varMap seems unnecessary, as IClause does not change var orders from TClause
	// but generalization really matter on variable names, e.g. v2 -> v1


	// first map variable names, then check if the body is a strict subset of the other


	bool matched = false;
	int g_sz = g.cl_body.size();
	int s_sz = s.cl_body.size();
	for(int i=0; i < g_sz; ++i) {
		const TRelation& g_t = g.cl_body[i];

		matched = false;
		for(int j=0; j < s_sz; ++j) {
			const TRelation& s_t = s.cl_body[j];
			if(g_t.pRel != s_t.pRel) {
				continue;
			}


			int arity = s_t.getArity();
			int k = 0;
			while(k < arity){
				int g_var = g.tc.vbody[i].vdom[k];
				int s_var = s.tc.vbody[j].vdom[k];

				auto it = varMap.find(g_var);
				if(it == varMap.end() || it->second != s_var){
					break;
				}
				++k;
			}

			if(k == arity){
				matched = true;
				break;
			}
		}

		if(!matched) {
			//std::cout<<"i=" <<i << ", cannot find a match for " << g_t.getRelNameWithTypes() << std::endl;
			break;
		}
	}


	return matched;
}

std::vector<std::set<int>> chooseK_helper(int n, int i, const std::vector<int>& v) {
	std::vector<std::set<int>> res;

	if(n > v.size()){
		return res;
	}

	if (i == 0 || n == 0) {
		res.push_back( std::set<int>() );
		return res;
	}

	auto encode = [&v](int x) {return v[x-1];};

	// choose n
	std::vector<std::set<int>> tmp1 = chooseK_helper(n-1, i-1, v);
	for(std::set<int>& x : tmp1) {
		x.insert( encode(n) );
		res.push_back( std::move(x) );
	}

	// don't choose n
	std::vector<std::set<int>> tmp2 = chooseK_helper(n-1, i, v);
	for(std::set<int>& x : tmp2) {
		res.push_back( std::move(x) );
	}

	return res;
}

} // end of anonymous namespace

std::set<int> IDBTR::extractRules(const std::set<int>& tmpls) const {
	std::set<int> res;
	for (const int x : tmpls) {
		auto it = tmpl2rules.find(x);
		if(it == tmpl2rules.end()) {
			continue;
		}

		for (const int e : it->second) {
			res.insert(e);
		}
	}

	std::cout << "extractRules, tmpls.size = " << tmpls.size() << ", res.size=" << res.size() << std::endl;

	return res;
}

// assume tm, rules  are already given
// initialize tmpl2rules, mostG, mostS
void IDBTR::init(){
	const int num_rules = rules.size();
	const int num_tmpls = tm.templates.size();

	// initialize tmpl2rules
	tmpl2rules.clear();

	for(int i=0; i < num_rules; ++i) {
		const IClause& cl = rules[i];
		int j = std::find(tm.templates.begin(), tm.templates.end(), cl.tc) - tm.templates.begin();
		if(j < num_tmpls) {
			tmpl2rules[j].insert(i);
		}
		else {
			std::cerr << "ERROR: IDBTR::int() cannot find template " << cl.tc.toStr() << " in tm\n";
		}
	}

	// initialize mostG/S
	//mostG.clear();
	//mostG.insert( extractRules(tm.getMostGeneral()) );
	//mostS.clear();
	//mostS.insert( extractRules(tm.getMostSpecific()) );

}

std::set<int> IDBTR::generalize(int rule_index) const{
	std::set<int> res;
	const IClause& cl = rules[rule_index];
	const TClause& tc = cl.tc;
	const int ti = tm.indexOf(tc);

	if(ti < 0) {
		std::cerr << "ERRPR: IDBTR::generalize cannot find template " << tc.toStr() << std::endl;
		return res;
	}

	auto it_tmpls = tm.specific_po.find(ti);
	if (it_tmpls == tm.specific_po.end() || it_tmpls->second.empty()) {
		return res;
	}

	const std::set<int>& tmpls = it_tmpls->second;
	for(int t : tmpls) {
		auto it_proof = tm.general_proof.find(t);
		if(it_proof == tm.general_proof.end()) {
			std::cerr << "ERROR: IDBTR::generalize specific_po exists, but no proof" << std::endl;
			return res;
		}

		const std::map<int, std::pair<PII, PII>> & proof = it_proof->second;
		auto it = proof.find(ti);
		if (it == proof.end()) {
			std::cerr << "ERROR: IDBTR::generalize cannot find a proof between "
					<< tm.templates[t].toStr()  << "  and  " << tc.toStr()
					<< std::endl;
		}
		const std::pair<PII, PII> & pf = it->second;
		const std::map<int, int>& relMap = pf.first;
		const std::map<int, int>& varMap = pf.second;

		auto it2 = tmpl2rules.find(t);
		if (it2 == tmpl2rules.end()) {
			continue;
		}
		for (int r : it2->second) {
			// Later if the refinement becomes unsound
			// try to replace check_proof with explicit theta-subsumption check
			if (check_proof(rules[r], cl, relMap, varMap)) {
				res.insert(r);
			}
		}
	}

	return res;
}

std::set<int> IDBTR::specialize(int rule_index) const{
	std::set<int> res;

	const IClause& cl = rules[rule_index];
	const TClause& tc = cl.tc;
	const int ti = tm.indexOf(tc);

	if(ti < 0) {
		std::cerr << "ERRPR: IDBTR::specialize cannot find template " << tc.toStr() << std::endl;
		return res;
	}

	auto it_tmpls = tm.general_po.find(ti);
	if(it_tmpls == tm.general_po.end() || it_tmpls->second.empty()){
		return res;
	}


	//if(tc.vbody.size() > 1) {
	//	return res;
	//}
	//std::cout << "specialize:\n" << tc.toStr() << std::endl;



	const std::set<int>& tmpls = it_tmpls->second;

	// Note that there could be multiple proofs between two templates
	// Instead of directly using one proof to check general relation between
	// two IClauses, we would better redo theta-sumption bewteen them.
	auto it_proof = tm.general_proof.find(ti);
	if(it_proof == tm.general_proof.end()) {
		std::cerr << "ERROR: IDBTR::specialize templates are not empty, but no proof" << std::endl;
		return res;
	}

	const std::map<int, std::pair<PII,PII>> & proof = it_proof->second;

	for(int t : tmpls) {
		auto it = proof.find(t);
		if(it == proof.end()) {
			std::cerr << "ERROR: IDBTR::specialize cannot find a proof between "
					<< tc.toStr() << "  and  " << tm.templates[t].toStr()
					<< std::endl;
		}
		const std::pair<PII,PII> & pf = it->second;
		const std::map<int,int>& relMap = pf.first;
		const std::map<int,int>& varMap = pf.second;


		auto it2 = tmpl2rules.find(t);
		if(it2 == tmpl2rules.end()){
			continue;
		}

		for(int r : it2->second) {
			// Later if the refinement becomes unsound
			// try to replace check_proof with explicit theta-subsumption check

			//std::cout << "test...\n";
			//std::cout << "    " << cl.toStr() << std::endl;
			//std::cout << "    " << rules[r].toStr() << std::endl;


			if(check_proof(cl, rules[r], relMap, varMap)) {

				//std::cout << "IDBTR::specialize, Partial order between two IClauses\n";
				//std::cout << cl.toStr() << std::endl;
				//std::cout << rules[r].toStr() << std::endl;
				res.insert(r);
			}
		}
	}

	return res;
}


std::vector<std::set<int>> IDBTR::chooseK(int k, bool general) const {
	if(k >= 3) {
		std::vector<std::set<int>> res;
		std::cerr << "K is too large: " << k << std::endl;
		return res;
	}

	std::set<int> st;

	if(general) {
		st = extractRules( tm.getMostGeneral() );
	}
	else{
		st = extractRules( tm.getMostSpecific() );
	}

	std::cout << "chooseK in " << rel.getRelNameWithTypes() << std::endl;
	std::cout << "st.size = " << st.size() <<std::endl;

	std::vector<int> v(st.begin(), st.end());

	return chooseK_helper(v.size(), k, v);
}


void DPManager::fillIDBValues() {

	for(const IDBTR& x : idbRules){
		const TRelation& tr = x.rel;

		std::set<std::vector<int>> st;

		Relation rel = tr.pRel;
		Tuple* tp = rel->Pos;
		while (*tp) {

			std::vector<int> v;
			for (int j = 1; j <= rel->Arity; ++j) {
				v.push_back( (int) (*tp)[j]);
			}

			st.insert(std::move(v));

			++tp;
		}


		IDBValue iv(tr);
		iv.pos = std::move(st);

		idbValues.push_back(std::move(iv));

	}
}

void DPManager::exploreCandidateRules() {
	for(const TRelation& rel : M.relm.vIDBRel) {

		std::cout << "Relation: " << rel.getRelNameWithTypes( ) << std::endl;
		std::vector<TClause> candidates = M.tm.findAllPossilbeMatchings(rel);
		std::vector<TClause> useful_templates;

		std::vector<IClause> matchings;

		for(const TClause& tc : candidates) {
			std::vector<IClause> Ms = M.findMatchingsWithTemplate(rel, tc);

			if(Ms.size()) {
				std::cout << ">> Template: " << tc.toStr() << ",  matches: " << Ms.size() << std::endl;

				useful_templates.push_back(tc);
			}

			for(IClause& m : Ms) {
				matchings.push_back( std::move(m) );
			}
		}

		TemplateManager tempM;
		tempM.templates = std::move(useful_templates);
		tempM.buildPartialOrder();

		std::cout << "overall templates: " << tempM.templates.size() << std::endl;
		std::cout << "most general: " << tempM.getMostGeneral().size() << std::endl;
		std::cout << "most specific: " << tempM.getMostSpecific().size() << std::endl;
		std::cout << "independent: " << tempM.getIndependent().size() << std::endl;

		tempM.logPO2dot( rel.getRelName() + ".dot" );

		IDBTR idb(rel);
		idb.tm = std::move(tempM);
		idb.rules = std::move(matchings);
		idb.init();

		idbRules.push_back( std::move(idb) );
	}

	initGS();
}

std::vector<DatalogProgram> DPManager::refineProg(const DatalogProgram& prog, bool specialize) {
	std::vector<DatalogProgram> res;

	for(auto pr : prog.state) {
		int idb_index = pr.first;
		const IDBTR & idb = idbRules[idb_index];

		const std::set<int>& rule_st = pr.second;

		// specialize one rule
		for(int rl : rule_st) {
			// try all candidates for one rule
			std::set<int> candidates = specialize ? idb.specialize(rl) : idb.generalize(rl);
			for(int can : candidates) {
				std::set<int> cp_st = rule_st;
				// NOTE: erase one then insert one,
				// #rules for an IDB does not change
				cp_st.erase(rl);
				cp_st.insert(can);

				std::map<int, std::set<int>> cp_state = prog.state;
				cp_state[idb_index] = std::move(cp_st);

				DatalogProgram dp(this);
				dp.state = std::move(cp_state);

				if(! hash_and_record(dp) ){
					// already existed, skip
					continue;
				}

				// NOTE: this insertion happens in three layer nested loops, could explode!!
				//res.insert( std::move(dp) );
				res.push_back( std::move(dp) );
			}
		}
	}

	return res;
}


std::vector<DatalogProgram> DPManager::refineProgWithTarget(const DatalogProgram& prog, bool specialize, int idb_index) {
	std::vector<DatalogProgram> res;
	std::set<int> refine_st = backwardAnalysis(prog, idb_index);

	const int DEPTH = -1;

	if(DEPTH > 0 && refine_st.size() >= DEPTH) {return res;}
	/*
	std::cout <<"\n\n"<< str(prog);
	std::cout << "after backward analysis from " << idb_index << std::endl;
	std::copy(refine_st.begin(), refine_st.end(), std::ostream_iterator<int>(std::cout, ","));
	std::cout << std::endl;
	*/

	for(auto pr : prog.state) {
		int idb_index = pr.first;
		if(refine_st.find(idb_index) == refine_st.end()) {
			continue;
		}

		const IDBTR & idb = idbRules[idb_index];

		const std::set<int>& rule_st = pr.second;

		// specialize one rule
		for(int rl : rule_st) {
			// try all candidates for one rule
			std::set<int> candidates = specialize ? idb.specialize(rl) : idb.generalize(rl);
			for(int can : candidates) {
				std::set<int> cp_st = rule_st;
				// NOTE: erase one then insert one,
				// #rules for an IDB does not change
				cp_st.erase(rl);
				cp_st.insert(can);

				std::map<int, std::set<int>> cp_state = prog.state;
				cp_state[idb_index] = std::move(cp_st);

				DatalogProgram dp(this);
				dp.state = std::move(cp_state);

				if (DEPTH > 0) {
					bool pass = true;
					for (auto pr : dp.state) {
						int index = pr.first;
						std::set<int> s = backwardAnalysis(prog, idb_index);
						if (s.size() >= DEPTH) {
							pass = false;
							break;
						}
					}
					if (!pass) {
						continue;
					}
				}


				if(! hash_and_record(dp) ){
					// already existed, skip
					continue;
				}

				// NOTE: this insertion happens in three layer nested loops, could explode!!
				//res.insert( std::move(dp) );
				res.push_back( std::move(dp) );
			}
		}

	}

	return res;
}

std::set<int> DPManager::backwardAnalysis(const DatalogProgram& prog, int idb_index) const {
	std::set<int> st;

	int prev_size = 0;
	st.insert(idb_index);
	while(prev_size < st.size()){
		prev_size = st.size();
		std::set<int> tmp_st;

		for(int x : st) {
			auto it = prog.state.find(x);
			if(it == prog.state.end() || it->second.empty()){
				continue;
			}

			const std::vector<IClause>& rules = idbRules[x].rules;
			for(int r_index : it->second) {
				const IClause& cl = rules[r_index];
				for(const TRelation& trel : cl.cl_body){
					int k = findIDBIndex(trel.pRel);
					if(k>=0){
						tmp_st.insert(k);
					}
				}
			}
		}

		for(int x : tmp_st){
			st.insert(x);
		}
	}

	return st;
}



void DPManager::init_helper(bool general) {
	std::vector< std::vector< std::set<int> > > VVS;

	long long space = 1;
	const int sz = idbRules.size();
	for(int i=0; i<sz; ++i) {
		const IDBTR& idb = idbRules[i];
		std::vector<std::set<int>>  vst = idb.chooseK(2, general);

		auto it = vst.begin();
		while(it != vst.end()){
			if(it->empty()) {
				it = vst.erase(it);
			}
			else{
				++it;
			}
		}

		space *= vst.size();
		VVS.push_back( std::move(vst) );
	}

	std::cout << "space: " << space << std::endl;

	for(int p = 0; p < space; ++p) {
		DatalogProgram dp (this);
		std::map<int, std::set<int>> state;

		int k = p;
		for(int i = 0; i < sz; ++i) {
			int j = k % VVS[i].size();
			state [i] = VVS[i][j];

			k = k / VVS[i].size();
		}

		dp.state = std::move(state);

		if(! hash_and_record(dp) ){
			// already existed, skip
			continue;
		}

		if(general) {
			//Gs.insert(std::move(dp));
			Gs.push_back( std::move(dp) );
		}
		else{
			//Ss.insert(std::move(dp));
			Ss.push_back( std::move(dp) );
		}
	}

}

void DPManager::initGS() {
	init_helper(true);
	std::cout << "Gs.size = " << Gs.size() << std::endl;

	//init_helper(false);
	//std::cout << "Ss.size = " << Ss.size() << std::endl;

	//execute( *Gs.begin() );
}

std::vector<ConcreteRule> DPManager::getConcreteRules(const DatalogProgram& dp) {
	std::vector<ConcreteRule> res;

	for(auto pr : dp.state) {
		int idb_index = pr.first;
		const IDBTR & idb = idbRules[idb_index];

		for(auto r : pr.second) {
			res.push_back( idb.rules[r].zip() );
		}
	}

	return res;
}


std::string DPManager::str(const DatalogProgram& dp) {
	std::stringstream ss;
	for(auto pr : dp.state) {
		int idb_index = pr.first;
		const IDBTR & idb = idbRules[idb_index];

		for(auto r : pr.second) {
			ss << idb.rules[r].toStr() << std::endl;
		}
	}

	return ss.str();
}

int DPManager::findIDBIndex(Relation r) const {
	const int n = idbRules.size();
	for(int i=0; i < n; ++i) {
		const IDBTR& idb = idbRules[i];
		if(r == idb.rel.pRel) {
			return i;
		}
	}

	return -1;
}

bool DPManager::hash_and_record(const DatalogProgram& x){
	std::string s = str(x);
	long long h = str_hash( s );
	auto it = visited.find(h);
	if(it == visited.end()) {
		//visited.insert( std::make_pair(h, s) );
		visited[h].insert(s);
	}
	else{
		// find the key, now check if there is a collision
		auto it2 = it->second.find(s);
		if(it2 == it->second.end()) {
			it->second.insert(s);
		}
		else{
			// already existsed
			return false;
		}
	}

	return true;
}

bool IDBValue::ask_and_update(std::vector<int>& v) {
	auto it = pos.find(v);

	if(it == pos.end()) {
		asked_neg.insert(v);
		return false;
	}
	else{
		asked_pos.insert(v);
		return true;
	}
}

bool DPManager::ask(std::vector<int>& Q) {
	const int idb_index = Q[0];
	std::vector<int> tp(Q.begin()+1, Q.end());

	return idbValues[idb_index].ask_and_update(tp);
}

void DPManager::test_specialize() {

	for(const IDBTR& idb : idbRules ) {
		if(idb.rel.getRelName() == "reptile"){
			int sz = idb.rules.size();
			for(int i=0; i < sz; ++i) {
				idb.specialize(i);
			}
		}
	}

}


} // end of namespace SpeedyFOIL
