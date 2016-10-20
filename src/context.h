#ifndef CONTEXT_H
#define CONTEXT_H


#include "template.h"

#include <vector>
#include <set>
#include <map>
#include <z3++.h>


namespace SpeedyFOIL {

struct ContextManager{
	z3::context C;

	std::map<TypeInfo, z3::sort> sortMap;
	std::map<Relation, z3::func_decl> funcMap;

	void buildSorts(TypeInfo* TArr, int N);
	void buildFuncDecls(Relation* RArr, int N);

};


} // end of namepsace SpeedyFOIL




#endif
