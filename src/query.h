#ifndef QUERY_H
#define QUERY_H


#include "template.h"

#include <vector>
#include <set>
#include <map>



namespace SpeedyFOIL {

struct TupleValue {
	TRelation rel;
	std::vector< std::vector<int> > pos;

	// not required for EDB tuple
	std::vector< std::vector<int> > neg;

};


} // end of namepsace SpeedyFOIL




#endif
