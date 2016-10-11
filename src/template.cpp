#include "template.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

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

		templates.push_back(std::move(cl));
	}

}

void TemplateManager::showTemplates() const {
	std::cout << "Number of templates: " << templates.size() << std::endl;
	for (const TClause& cl : templates) {
		std::cout << cl.toStr() << std::endl;
	}
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
