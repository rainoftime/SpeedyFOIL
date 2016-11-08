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
	int prog_id;

	FixedPoint(ContextManager* pCM);

	FixedPoint(const FixedPoint&) = delete;

	FixedPoint(FixedPoint&& fp) noexcept :
			pContext(fp.pContext),
			z3_fp(fp.z3_fp),
			recent(fp.recent),
			defaultS(fp.defaultS),
			prog_id(fp.prog_id){
		fp.pContext = nullptr;
		fp.prog_id = -1;
	}

	FixedPoint& operator = (FixedPoint& fp) = delete;
	FixedPoint& operator = (const FixedPoint& fp) = delete;


	FixedPoint& operator = (FixedPoint&& fp) noexcept {
		defaultS = fp.defaultS;
		pContext = fp.pContext;
		z3_fp = fp.z3_fp;
		recent = fp.recent;
		prog_id = fp.prog_id;

		fp.pContext = nullptr;
		fp.prog_id = -1;

		return *this;
	}

	void set_prog_id(int id) {prog_id = id;}

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

	//std::map<std::vector<int>, int> vote_stats;
	std::map<std::vector<int>, std::set<int> > vote_stats;

	std::set<int> fail_to_derive;

	//  data structure  for debugging
	std::map<int, std::set< std::pair<int,int> >> edges;
	std::vector< std::vector<int> > layers;
	void draw_layer_graph(std::set<int>&);

	// random selection
	std::set< std::pair<int,int> > random_picked;
	bool random_mode = false;


	z3::expr build_func_constr(z3::context& context,
			std::map<int, z3::expr>& z3_vars,
			std::pair<Relation, std::vector<int>>& pair);

	void execute_one_round_helper(std::vector<DatalogProgram>&);
	std::vector<int> execute_one_round();
	std::vector<int> random_pick();
	bool test_converge();

	//FixedPoint prepare(const DatalogProgram & dp,
	//		std::set<std::pair<Relation, std::vector<int>>>&queries);

	FixedPoint prepare(const DatalogProgram & dp);

	bool execute(const DatalogProgram & dp);
	//z3::expr construct_query(const std::pair<Relation, std::vector<int>>& Q);
	//void queryIDBs(std::set<std::pair<Relation, std::vector<int>>>& queries, FixedPoint& fp, bool=false);
	z3::expr construct_query(Relation);
	void queryIDBs(FixedPoint& fp, bool=false);
	bool validate_with_full_IDBs();

	void parse_and_update(z3::expr&, int, int, bool=false);

	bool test(const DatalogProgram&, z3::expr);

	z3::expr convert_question(std::vector<int>&);

	void eliminate_and_refine(std::vector<DatalogProgram>& A, std::vector<DatalogProgram>& B, bool,
			z3::expr&, z3::expr&, std::vector<int>& );

	bool test2(const DatalogProgram&, const std::vector<int>&);
	std::pair<bool,bool> test3(DatalogProgram&, bool, std::vector<int>&, std::vector<std::vector<int> >&);


	void eliminate_and_refine2(std::vector<DatalogProgram>& A,
			std::vector<DatalogProgram>& B, bool, std::vector<int>&,
			std::vector<std::vector<int> >&);

	void work();

	void work_refinement();
	void work_baseline();

};


} // end of namepsace SpeedyFOIL




#endif
