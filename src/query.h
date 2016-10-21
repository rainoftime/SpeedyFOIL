#ifndef QUERY_H
#define QUERY_H


#include "template.h"
#include "datalog.h"
#include "context.h"

#include <vector>
#include <set>
#include <map>



namespace SpeedyFOIL {


struct QueryEngine {

	//std::shared_ptr<DPManager> dp_ptr;
	//std::shared_ptr<ContextManager> cm_ptr;

	std::unique_ptr<DPManager> dp_ptr;
	std::unique_ptr<ContextManager> cm_ptr;


	std::map< std::vector<int>, int > vote_stats;

	z3::expr build_func_constr(z3::context& context,
			std::map<int, z3::expr>& z3_vars,
			std::pair<Relation, std::vector<int>>& pair);

	void execute_one_query();

	void execute(const DatalogProgram & dp);
	void queryIDBs(std::set<std::pair<Relation, std::vector<int>>>& queries, Z3_fixedpoint& fp);
	void parse_and_update(z3::expr&, int);

	void work();

	~QueryEngine(){
		dp_ptr.release();
		cm_ptr.release();
	}

};


} // end of namepsace SpeedyFOIL




#endif
