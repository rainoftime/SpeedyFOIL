#include "template.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace SpeedyFOIL {

namespace {
TPredicate readPredicate(std::ifstream& fin) {
	TPredicate pred;
	fin >> pred.pid;
	fin >> pred.arity;
	int a = pred.arity;
	while (a--) {
		int t;
		fin >> t;
		pred.vdom.push_back(t);
	}

	return pred;
}

std::string formatTPredicate(const TPredicate& pred, const TRelation& rel){
	std::stringstream ss;

	std::vector<std::string> strs = rel.getStrs();

	if (pred.arity != strs.size() - 1) {
		std::cerr << "Arity does not match with template " << std::endl;
	}

	ss << strs[0] << "(";
	for (int i = 0; i < pred.arity; ++i) {
		if (i) {
			ss << ",";
		}
		ss << "x" << pred.vdom[i] << ":" << strs[i + 1];
	}
	ss << ")";

	return ss.str();
}

bool patternMatched(const TPredicate& pred1, const TPredicate& pred2, std::map<int, int>& relMap,
		std::map<int, int>& varMap) {
	if(pred1.arity != pred2.arity) {
		return false;
	}

	auto it = relMap.find(pred1.pid);
	if(it != relMap.end() && it->second != pred2.pid) {
		return false;
	}

	for(int i = 0; i < pred1.arity; ++i) {
		auto it2 = varMap.find( pred1.vdom[i] );
		if(it2 != varMap.end() && it2->second != pred2.vdom[i]){
			return false;
		}
	}

	// enforce match, update relMap/ varMap
	if(it == relMap.end()){
		relMap.insert( std::make_pair(pred1.pid, pred2.pid) );
	}

	for(int i = 0; i < pred1.arity; ++i) {
		auto it2 = varMap.find( pred1.vdom[i] );
		if(it2 == varMap.end()) {
			varMap.insert( std::make_pair(pred1.vdom[i], pred2.vdom[i]) );
		}
	}

	return true;
}

bool quickCheckGeneral(const std::vector<TPredicate>& rb1, const std::vector<TPredicate>& rb2) {
	std::set<int> st1, st2;
	for(auto & x : rb1) st1.insert( x.arity );
	for(auto & x : rb2) st2.insert( x.arity );

	for(auto x : st1) {
		if(st2.find(x) == st2.end()) {
			return false;
		}
	}

	return true;
}

bool moreGeneral(int i, int j, const std::vector<TPredicate>& rb1,
		const std::vector<TPredicate>& rb2, std::map<int, int>& relMap,
		std::map<int, int>& varMap) {

	// check termination
	if(i == rb1.size()){
		return true;
	}

	std::map<int,int> localRelMap = relMap;
	std::map<int,int> localVarMap = varMap;

	// i-th Predicate match the j-th Predicate
	if (j < rb2.size()) {
		if (patternMatched(rb1[i], rb2[j], relMap, varMap)) {
			if (moreGeneral(i + 1, j + 1, rb1, rb2, relMap, varMap)) {
				return true;
			}

			// restore relMap, varMap
			relMap = localRelMap;
			varMap = localVarMap;
		}
	}

	// i-th Predicate matches some k-th (k < j)
	for(int k = 0; k < j; ++k) {
		if(patternMatched(rb1[i], rb2[k], relMap, varMap)) {
			if( moreGeneral(i+1, j, rb1, rb2, relMap, varMap) ) {
				return true;
			}

			// restore relMap, varMap
			relMap = localRelMap;
			varMap = localVarMap;
		}
	}

	return false;
}

} // namespace anonymous


std::string TPredicate::toStr() const {
	std::stringstream ss;
	ss << "P" << pid;
	if (arity > 0) {
		if (arity != vdom.size()) {
			std::cerr
					<< "Predicate: the arity and actual vdom size does not match, "
					<< "arity=" << arity
					<< ", vdom.sz=" << vdom.size()
					<< std::endl;
		}
		//static_assert(arity == vdom.size(), "Predicate: the arity and actual vdom size does not match");
		ss << "(";
		for (int i = 0; i < arity; ++i) {
			if (i) {
				ss << ",";
			}
			ss << "v" << vdom[i];
		}
		ss << ")";

	}

	return ss.str();
}


std::string TClause::toStr() const {
	std::stringstream ss;
	ss << hd.toStr();
	if (vbody.size()) {
		ss << " :- ";
		for (int i = 0; i < vbody.size(); ++i) {
			if (i)
				ss << ",";
			ss << vbody[i].toStr();
		}

		ss << ".";
	}

	return ss.str();
}

bool TClause::existDisconnectedPred() const {
	std::set<int> connectedVars;
	std::set<int> visited;
	for(int x : hd.vdom){
		connectedVars.insert(x);
	}

	const int sz = vbody.size();
	bool update = true;
	while(update) {
		update = false;
		for(int i=0; i< sz; ++i) {
			if(visited.find(i) != visited.end()) {
				continue;
			}

			const TPredicate& p = vbody[i];
			for(int x : p.vdom) {
				if( connectedVars.find(x) !=  connectedVars.end()){
					visited.insert(i);
					for(int y: p.vdom) connectedVars.insert(y);
					update = true;
					break;
				}
			}
		}
	}

	//std::cout << "visited(sz=" << visited.size() << "): ";
	//for(int x : visited) std::cout << x << " ";
	//std::cout <<"\nvbody.sz=" << sz << std::endl;

	return visited.size() != sz;
}

bool TClause::existUnboundVarInHead() const {
	std::set<int> visited;

	for(const TPredicate& pred : vbody){
		for(int x : pred.vdom) {
			visited.insert(x);
		}
	}

	for(int x : hd.vdom){
		if( visited.find(x) == visited.end() ){
			return true;
		}
	}

	return false;
}


bool TClause::moreGeneralThan(const TClause& tc) const {

	if(! quickCheckGeneral(vbody, tc.vbody)){
		return false;
	}

	std::map<int,int> relMap;
	std::map<int,int> varMap;

	if(! patternMatched(hd, tc.hd, relMap, varMap)  ) {
		return false;
	}

	std::vector<int> vi;
	const std::vector<TPredicate>& body = tc.vbody;
	int ct = 1;
	for(int i = 0; i < body.size(); ++i) {
		vi.push_back(i);
		ct *= i+1;
	}

	// test all permutation of tc.vbody, the cost is not too huge as |vbody| <= 8
	while(ct --) {

		// setup
		std::map<int,int> cpRelMap = relMap;
		std::map<int,int> cpVarMap = varMap;

		std::vector<TPredicate> tv(body.size());

		for(int i=0; i< body.size(); ++i) {
			tv[i] = body[ vi[i] ];
		}

		// test
		if(moreGeneral(0,0, vbody, tv, cpRelMap, cpVarMap)) {
			//std::cout << "great! find one more general case" << std::endl;
			return true;
		}

		// try next
		std::next_permutation(vi.begin(), vi.end());
	}

	return false;
}

void IClause::explain() const{
	std::cout << "One possible instantiation: " << std::endl;
	std::cout << "Template: " << tc.toStr() << std::endl;

	// output head
	std::cout << formatTPredicate(tc.hd, cl_hd) << " :- ";

	// output body
	int n = tc.vbody.size();
	for(int i=0; i < n; ++i) {
		if(i) {
			std::cout << ",";
		}
		std::cout << formatTPredicate(tc.vbody[i], cl_body[i]);
	}

	std::cout << "." << std::endl;

}

// Template Encoding for rule: path(x,y) :- path(x,z), edge(z, y).
// 2
// 0 2 0 1
// 0 2 0 2
// 1 2 2 1

void TemplateManager::loadTemplates(std::string fpath) {
	std::ifstream fin(fpath);

	int nbody = 0;
	while (fin >> nbody) {
		TClause cl;
		cl.hd = readPredicate(fin);
		while (nbody--) {
			cl.vbody.push_back(readPredicate(fin));
		}

		if(cl.existDisconnectedPred() || cl.existUnboundVarInHead() ){
			//std::cout << "ignore:  " << cl.toStr() << std::endl;
		}
		else {
			templates.push_back(std::move(cl));
		}
	}

}

void TemplateManager::showTemplates() const {
	std::cout << "Number of templates: " << templates.size() << std::endl;
	for (const TClause& cl : templates) {
		std::cout << cl.toStr() << std::endl;
	}
}

void TemplateManager::normalizePO() {
	const int sz = templates.size();
	std::cout << "normalizing PO..\n";
	for(int i=0;i< sz; ++i) {
		std::set<int> mark;
		for(int x : general_po[i]){
			std::set<int> st = general_po[x];
			if( st.find(i) != st.end()){
				int sz1 = templates[i].vbody.size();
				int sz2 = templates[x].vbody.size();
				if(sz1 > sz2) {
					mark.insert(x);
					std::cout << templates[i].toStr() << "  vs  " << templates[x].toStr() << std::endl;
				}
				else if(sz1 == sz2){
					std::cout << "equivalent templates: " << templates[i].toStr() << "  vs  " << templates[x].toStr() << std::endl;
				}
			}
		}

		for(int x : mark){
			general_po[i].erase(x);
			specific_po[x].erase(i);
		}
	}

}


void TemplateManager::buildPartialOrder() {
	const int sz = templates.size();
	for(int i=0;i< sz; ++i) {

		//std::cout<< "buildPartialOrder, i=" << i << std::endl;

		const TClause& ta = templates[i];
		for(int j=0; j < sz; ++j) {
			if(i==j) {
				continue;
			}
			const TClause& tb = templates[j];

			if(ta.moreGeneralThan(tb)) {
				//std::cout << ta.toStr() << " <= " << tb.toStr() << std::endl;
				general_po[i].insert(j);
				specific_po[j].insert(i);
			}
		}
	}

	normalizePO();
}

std::set<int> TemplateManager::getMostGeneral() {
	std::set<int> res;
	const int sz = templates.size();
	for(int i=0;i<sz;++i) {
		if( specific_po[i].size() == 0 ) {
			res.insert(i);
		}
	}
	return res;
}

std::set<int> TemplateManager::getMostSpecific() {
	std::set<int> res;
	const int sz = templates.size();
	for(int i=0;i<sz;++i) {
		if( general_po[i].size() == 0 ) {
			res.insert(i);
		}
	}
	return res;
}

std::set<int> TemplateManager::getIndependent() {
	std::set<int> res;
	const int sz = templates.size();
	for(int i=0;i<sz;++i) {
		if( general_po[i].size() == 0 && specific_po[i].size() == 0 ) {
			res.insert(i);
		}
	}
	return res;
}


std::vector<TClause> TemplateManager::findAllPossilbeMatchings(const TRelation& rel) const {
	std::vector<TClause> res;
	for(const TClause& tc : templates) {
		if(rel.possibleMatch(tc.hd)){
			res.push_back(tc);
		}
 	}
	return res;
}

void TemplateManager::logPO2dot(std::string fpath) {
	std::ofstream fout(fpath);
	fout << "digraph G {\n";

	fout << "  subgraph {\n";
	fout << "    rank = same;";
	for(int x : getMostGeneral()) {
		fout << "v" << x << "; ";
	}
	fout << "\n  }\n";

	const int sz = templates.size();
	for (int i = 0; i < sz; ++i) {
		if (general_po[i].size()) {
			fout << "v" << i << " -> {";
			for (int x : general_po[i]) {
				fout <<"v" << x << " ";
			}
			fout << " };\n";
		}
	}

	for (int i = 0; i < sz; ++i) {
		std::string s = templates[i].toStr();
		int j = 0;
		while(j < s.length()){
			if(s[j] == '-') break;
			++j;
		}

		fout << "v" << i << " [label=\"" << s.substr(j+1) << "\"];\n";
	}

	fout << "}\n";

	fout.close();
}

std::vector<std::string> TRelation::getStrs() const {
	std::vector<std::string> res;
	res.push_back( getRelName() );

	int n = getArity();
	for(int i = 0; i < n; ++i){
		TypeInfo ty = getType(i);
		if(ty){
			res.push_back( std::string(ty->Name) );
		}
		else {
			res.push_back( "nil" );
		}
	}

	return res;
}

std::string TRelation::getRelNameWithTypes() const {
	std::stringstream ss;

	std::vector<std::string> vs = getStrs();
	ss << vs[0] << "(";
	for(int i=1; i<vs.size(); ++i) {
		if(i>1) ss << ",";
		ss << vs[i];
	}
	ss << ")";

	return ss.str();
}


bool TRelation::possibleMatch(const TPredicate & pr) const {
	return getArity() == pr.arity;
}


void RelationManager::loadRelations(Relation* reln, int n) {
	for(int i=0; i<n; ++i) {
		if(reln[i]->Name[0] == '=' || reln[i]->Name[0] == '>') {
			continue;
		}

		if(reln[i]->PossibleTarget) {
			vIDBRel.push_back( TRelation(reln[i]) );
		}
		else {
			vEDBRel.push_back( TRelation(reln[i]) );
		}
	}
}

void RelationManager::showRelations() const {
	std::cout <<"EDB relations:";
	for(const TRelation& r : vEDBRel) {
		std::cout << r.getRelNameWithTypes() << ", ";
	}
	std::cout << std::endl << "IDB relations: ";

	for(const TRelation& r : vIDBRel) {
		std::cout << r.getRelNameWithTypes() << ", ";
	}
	std::cout << std::endl;
}

std::vector<TRelation> RelationManager::finPossibleInst(const TPredicate& pr, bool onlyIDB) const {
	std::vector<TRelation> res;

	for(const TRelation& rel : vIDBRel) {
		if(rel.possibleMatch(pr)){
			res.push_back(rel);
		}
	}

	if (!onlyIDB) {
		for (const TRelation& rel : vEDBRel) {
			if (rel.possibleMatch(pr)) {
				res.push_back(rel);
			}
		}
	}

	return res;
}



} // namespace SpeedyFOIL

/*
int main(int argc, char* args[]) {
	std::string s(args[1]);
	std::cout << "Template file: " << s << std::endl;

	SpeedyFOIL::TemplateManager tm;
	tm.loadTemplates(s);
	tm.showTemplates();

	return 0;
}
*/
