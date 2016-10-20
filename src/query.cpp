#include "query.h"

#include <vector>
#include <sstream>

namespace SpeedyFOIL {

namespace {

const char* str(int x) {
	std::stringstream ss;
	ss << "v" << x;
	return ss.str().c_str();
}

} // end of anonymous namespace

z3::expr QueryEngine::helper(z3::context& context,
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

void QueryEngine::execute_one_query() {
	auto itx = dp_ptr->Gs.begin();

	z3::context& context = cm_ptr->C;
	z3::sort bv_sort = context.bv_sort(cm_ptr->MaxBits);

	std::vector<ConcreteRule> rs = dp_ptr->getConcreteRules( *itx );

	std::vector<z3::expr> z3_rs;
	std::set<std::pair<Relation, std::vector<int>>> queries;

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

		z3::expr head = helper(context, z3_vars, r[0]);

		queries.insert(r[0]);

		z3::expr_vector ev(context);
		const int n = r.size();
		for (int i = 1; i < n; ++i) {
			z3::expr e = helper(context, z3_vars, r[i]);
			ev.push_back(e);
		}
		// z3::expr r2 = z3::forall(va, vb, vc, z3::implies( z3::mk_and(ev), path(va,vc) ));

		z3::expr rule = z3::forall(all_vs, z3::implies(z3::mk_and(ev), head));

		z3_rs.push_back(rule);
	}

	Z3_fixedpoint fp = Z3_mk_fixedpoint(context);
	Z3_fixedpoint_inc_ref(context,fp);

	for (auto pr : cm_ptr->funcMap) {
		Z3_fixedpoint_register_relation(context, fp, pr.second);
	}

	z3::symbol s_r1 = cm_ptr->C.str_symbol("r1");
	for (z3::expr& r : z3_rs) {
		Z3_fixedpoint_add_rule(context, fp, r, s_r1);
	}

	cm_ptr->appendEDBConstr(fp);


	// Query
	for (auto Q : queries) {
		Relation rel = Q.first;
		z3::func_decl f = cm_ptr->funcMap.find(rel)->second;

		std::set<int> vars(Q.second.begin(), Q.second.end());

		z3::expr_vector ex_vs(context);
		std::map<int, z3::expr> z3_vars;
		for (int x : vars) {
			z3::expr vx = context.constant(str(x), bv_sort);
			z3_vars.insert( std::make_pair(x, vx) );

			ex_vs.push_back(vx);
		}

		std::cout << "z3_vars:\n";
		for(auto pr : z3_vars) {
			std::cout << "key=" << pr.first << ", value=" << pr.second << std::endl;
		}

		z3::expr_vector params(context);
		for (int x : Q.second) {
			auto it = z3_vars.find(x);
			params.push_back( it->second );
		}

		std::cout << "parms: " << params << std::endl;

		z3::expr query_expr = z3::exists(ex_vs, f(params));
		std::cout << "query_expr: " << query_expr << std::endl;

		Z3_lbool res = Z3_fixedpoint_query(context, fp,
				query_expr);

		std::cout << "res = " << res << std::endl;
		if (res == Z3_L_FALSE) {
			std::cout << "res is L_false" << std::endl;
		} else if (res == Z3_L_UNDEF) {
			std::cout << "res is L_undef" << std::endl;
		} else if (res == Z3_L_TRUE) {
			std::cout << "res is L_true" << std::endl;
		} else {
			std::cout << "res is unknown value" << std::endl;
		}

		Z3_ast ast_res = Z3_fixedpoint_get_answer(context, fp);
		z3::expr detailed_res(context, ast_res);
		std::cout << detailed_res << std::endl;

	}

	Z3_fixedpoint_dec_ref(context,fp);

}

} // end of namepsace SpeedyFOIL
