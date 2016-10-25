#ifndef QUERY_H
#define QUERY_H


#include "template.h"
#include "datalog.h"
#include "context.h"

#include <vector>
#include <set>
#include <map>



namespace SpeedyFOIL {


struct FixedPoint{
	Z3_fixedpoint z3_fp;
	z3::context * pContext;
	z3::symbol defaultS;
	Z3_lbool recent;

	FixedPoint(ContextManager* pCM);

	FixedPoint(const FixedPoint&) = delete;

	FixedPoint(FixedPoint&& fp) noexcept : pContext(fp.pContext), z3_fp(fp.z3_fp), recent(fp.recent), defaultS(fp.defaultS) {
		fp.pContext = nullptr;
	}

	FixedPoint& operator = (FixedPoint& fp) = delete;
	FixedPoint& operator = (const FixedPoint& fp) = delete;


	FixedPoint& operator = (FixedPoint&& fp) noexcept {
		defaultS = fp.defaultS;
		pContext = fp.pContext;
		z3_fp = fp.z3_fp;
		recent = fp.recent;

		fp.pContext = nullptr;
		return *this;
	}

	void add_rules(std::vector<z3::expr>& rules);

	void add_rule(z3::expr rule);

	bool query(Z3_ast);
	z3::expr get_answer();

	void register_relation(Z3_func_decl f) {
		Z3_fixedpoint_register_relation( *pContext, z3_fp, f);
	}

	~FixedPoint() {
		if(pContext == nullptr) {
			// values have been moved, do nothing
			std::cerr << "values have been moved, do nothing" << std::endl;
		}
		else {
			Z3_fixedpoint_dec_ref(*pContext,z3_fp);
		}
	}

};

struct QueryEngine {

	int warn_ct = 0;

	DPManager* dp_ptr;
	ContextManager* cm_ptr;

	std::hash<std::string> str_hash;
	std::map<long long, std::set<std::string> > removedPrograms;

	std::map<std::vector<int>, int> vote_stats;

	//  data structure  for debugging
	std::map<int, std::set< std::pair<int,int> >> edges;
	std::vector< std::vector<int> > layers;
	void draw_layer_graph(std::set<int>&);


	z3::expr build_func_constr(z3::context& context,
			std::map<int, z3::expr>& z3_vars,
			std::pair<Relation, std::vector<int>>& pair);

	std::vector<int> execute_one_round();

	FixedPoint prepare(const DatalogProgram & dp,
			std::set<std::pair<Relation, std::vector<int>>>&queries);

	void execute(const DatalogProgram & dp);
	void queryIDBs(std::set<std::pair<Relation, std::vector<int>>>& queries, FixedPoint& fp);
	void parse_and_update(z3::expr&, int);

	bool test(const DatalogProgram&, z3::expr);

	z3::expr convert_question(std::vector<int>&);

	void eliminate_and_refine(std::vector<DatalogProgram>& A, std::vector<DatalogProgram>& B, bool,
			z3::expr&, z3::expr&, std::vector<int>& );
	void work();

};


} // end of namepsace SpeedyFOIL




#endif
