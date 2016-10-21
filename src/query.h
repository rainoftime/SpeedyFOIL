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

	FixedPoint(ContextManager* pCM);

	FixedPoint(const FixedPoint&) = delete;

	FixedPoint(FixedPoint&& fp) noexcept : defaultS(fp.defaultS) {
		pContext = fp.pContext;
		z3_fp = fp.z3_fp;

		fp.pContext = nullptr;
	}

	FixedPoint& operator = (FixedPoint&& fp) noexcept {
		defaultS = fp.defaultS;
		pContext = fp.pContext;
		z3_fp = fp.z3_fp;

		fp.pContext = nullptr;
		return *this;
	}

	void add_rules(std::vector<z3::expr>& rules);

	void add_rule(z3::expr rule);

	void register_relation(Z3_func_decl f) {
		Z3_fixedpoint_register_relation( *pContext, z3_fp, f);
	}

	~FixedPoint() {
		if(pContext == nullptr) {
			// values have been moved, do nothing
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


	std::map< std::vector<int>, int > vote_stats;

	z3::expr build_func_constr(z3::context& context,
			std::map<int, z3::expr>& z3_vars,
			std::pair<Relation, std::vector<int>>& pair);

	void execute_one_round();

	Z3_fixedpoint prepare(const DatalogProgram & dp, std::set<std::pair<Relation, std::vector<int>>>& queries);
	void execute(const DatalogProgram & dp);
	void queryIDBs(std::set<std::pair<Relation, std::vector<int>>>& queries, Z3_fixedpoint& fp);
	void parse_and_update(z3::expr&, int);

	void work();

};


} // end of namepsace SpeedyFOIL




#endif
