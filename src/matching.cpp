
#include "template.h"
#include <vector>
#include <map>
#include <iostream>

namespace SpeedyFOIL {

namespace {
bool testConsistency(const IClause& cl) {
	return false;
}

bool checkAndUpdate(const TPredicate& pred,
		const TRelation& rel,
		std::map<int, Relation>& relMap,
		std::map<int, TypeInfo>& tyMap) {

	if(rel.getArity() != pred.arity) {
		//std::cout << "checkAndUpdate: arity does not match\n";
		return false;
	}

	// std::map<int, Relation>::iterator
	auto it = relMap.find(pred.pid);
	if(it != relMap.end() && rel.getRel() != it->second) {
		//std::cout << "checkAndUpdate: relation is not expected\n";
		//std::cout << "pred=" << pred.toStr() << ", current rel:" << rel.getRelName()
		//		<< ", expected rel:" << it->second->Name << std::endl;
 		return false;
	}

	for(int i=0; i < pred.arity; ++i) {
		int v = pred.vdom[i];
		auto it2 = tyMap.find(v);
		if(it2 != tyMap.end() && it2->second != rel.getType(i)) {
			std::cout << "checkAndUpdate: type is not expected\n";

			return false;
		}
	}

	// passed arity / type checks, now updates

	if(it == relMap.end()){
		relMap.insert( std::make_pair(pred.pid, rel.getRel()) );
	}

	for(int i=0; i < pred.arity; ++i) {
		int v = pred.vdom[i];
		auto it2 = tyMap.find(v);
		if(it2 == tyMap.end()) {
			tyMap.insert( std::make_pair(v, rel.getType(i)) );
		}
	}

	return true;
}

} // end of anonymous namespace

std::vector<IClause> Matching::findMatchingsWithTemplate(const TRelation& rel,
		const TClause& tc) const {

	std::vector<IClause> matchings;

	std::map<int, Relation> relMap;
	std::map<int, TypeInfo> tyMap;

	std::vector<TRelation> vr;
	if (checkAndUpdate(tc.hd, rel, relMap, tyMap)) {
		if (exploreBody(tc, vr, relMap, tyMap)) {
			matchings.push_back(IClause(tc, rel, vr));
		}
	}

	return matchings;
}

bool Matching::exploreBody(const TClause& tc, std::vector<TRelation>& rels,
		std::map<int, Relation>& relMap, std::map<int, TypeInfo>& tyMap) const {

	std::cout << "exploreBody: tc=" << tc.toStr() << ", rels: ";
	for(auto &x : rels){
		std::cout << x.getRelName() <<", ";
	}
	std::cout << std::endl;

	if( rels.size() == tc.vbody.size() ) {
		return true;
	}
	int k = rels.size();
	const TPredicate & pred = tc.vbody[k];

	std::vector<TRelation> relCandidates = relm.finPossibleInst(pred, false);


	// make a local copy for each map for later backtracking if necessary
	std::map<int, Relation> localRelMap = relMap;
	std::map<int, TypeInfo> localTyMap = tyMap;

	for(const TRelation& r : relCandidates) {
		if (checkAndUpdate(pred, r, relMap, tyMap)) {
			rels.push_back(r);
			if(exploreBody(tc, rels, relMap, tyMap)){
				return true;
			}
			rels.pop_back();
		}

		// restore the map status
		relMap = localRelMap;
		tyMap = localTyMap;
	}

	return false;
}


std::vector<IClause> Matching::findAllMatchings(const TRelation & rel) const {
	std::vector<IClause> matchings;

	std::vector<TClause> candidates = tm.findAllPossilbeMatchings(rel);

	for(const TClause& tc : candidates) {
		//std::cout << "\nwill test template: " << tc.toStr()
		//		<< "  for relation: " << rel.getRelName() << "\n\n";
		for(IClause& x : findMatchingsWithTemplate(rel, tc)){
			//std::cout << "find a matching for " << rel.getRelName() << std::endl;
			//std::cout << "template is : " << tc.toStr() << std::endl;
			//x.explain();

			matchings.push_back( std::move(x) );
			//matchings.push_back( x );
		}
	}

	//std::cout << "before return result of findAllMatchings:\n";
	//for(const IClause& icl : matchings) icl.explain();
	return matchings;
}


void Matching::work() {
	for(TRelation& rel : relm.vIDBRel) {
		std::cout << "Explore IDB: " << rel.getRelName() << std::endl;

		const std::vector<IClause>& res = findAllMatchings(rel);

		std::cout << "after findAllMatchings:" << std::endl;
		for(const IClause& icl : res) {
			icl.explain();
		}
	}
}

}  // end of namespace SpeedyFOIL

