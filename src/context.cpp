
#include "context.h"

#include <algorithm>
#include <map>

namespace SpeedyFOIL {


void ContextManager::buildSorts(TypeInfo* TArr, int N) {
	const int START = 1; // to confirm
	const int END = N; // to confirm

	for(int i=START; i < END; ++i) {
		TypeInfo t = TArr[i];

		int sz  = t->NValues;

		int K = 1;
		while(sz) {
			++K;
			sz >>= 1;
		}

		z3::sort s = C.bv_sort( K );

		sortMap.insert( std::make_pair(t, s) );
	}
}

void ContextManager::buildFuncDecls(Relation* RArr, int N) {
	const int START = 1; // to confirm
	const int END = N; // to confirm

	for(int i=START; i < END; ++i) {
		Relation rel = RArr[i];

		if(rel->Name[0] == '=' || rel->Name[0] == '>') {
			continue;
		}

		//std::cout << "process " << rel->Name << std::endl;

		z3::sort_vector sv(C);

		for(int j=1; j <= rel->Arity; ++j){
			TypeInfo t = rel->TypeRef[j];
			auto it = sortMap.find(t);
			if(it == sortMap.end()) {
				std::cerr << "ERROR, buildFuncDecls get unknown type: " << t->Name << std::endl;
			}

			sv.push_back( it->second );
		}

		z3::func_decl pred = C.function(rel->Name, sv, C.bool_sort());

		funcMap.insert( std::make_pair(rel, pred) );
	}
}


} // end of namepsace SpeedyFOIL
