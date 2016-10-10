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
} // namespace anonymous


std::string TPredicate::toStr() const {
	std::stringstream ss;
	ss << "P" << pid;
	if (arity > 0) {
		if (arity != vdom.size()) {
			std::cerr
					<< "Predicate: the arity and actual vdom size does not match"
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


void IClause::explain() {
	std::cout << "One possible instantiation: " << std::endl;
	std::cout << "Template: " << tc.toStr() << std::endl;

	std::vector<std::string> hd_strs = cl_hd.getStrs();
	if(tc.hd.arity != hd_strs.size() - 1) {
		std::cerr << "Arity does not match with template head" << std::endl;
	}


	// output head
	std::cout << hd_strs[0] << "(";
	for(int i=0; i<tc.hd.arity; ++i) {
		if(i) std::cout << ",";
		std::cout << "x" << tc.hd.vdom[i] << ":" << hd_strs[i+1];
	}


	// output body


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
			//res.push_back(tc);
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
		res.push_back( std::string(ty->Name) );
	}

	return res;
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
		std::cout << r.getRelName() << ", ";
	}
	std::cout << std::endl << "IDB relations: ";

	for(const TRelation& r : vIDBRel) {
		std::cout << r.getRelName() << ", ";
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
