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


	z3::expr helper(z3::context& context,
			std::map<int, z3::expr>& z3_vars,
			std::pair<Relation, std::vector<int>>& pair);

	void execute_one_query();


	~QueryEngine(){
		dp_ptr.release();
		cm_ptr.release();
	}

};


} // end of namepsace SpeedyFOIL




#endif
