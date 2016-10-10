#include <template.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace SpeedyFOIL {

namespace {
Predicate readPredicate(std::ifstream& fin) {
	Predicate pred;
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


std::string Predicate::toStr() const {
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


std::string Clause::toStr() const {
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


// Template Encoding for rule: path(x,y) :- path(x,z), edge(z, y).
// 2
// 0 2 0 1
// 0 2 0 2
// 1 2 2 1

void TemplateManager::loadTemplates(std::string fpath) {
	std::ifstream fin(fpath);

	int nbody = 0;
	while (fin >> nbody) {
		Clause cl;
		cl.hd = readPredicate(fin);
		while (nbody--) {
			cl.vbody.push_back(readPredicate(fin));
		}

		templates.push_back(std::move(cl));
	}

}

void TemplateManager::showTemplates() const {
	std::cout << "Number of templates: " << templates.size() << std::endl;
	for (const Clause& cl : templates) {
		std::cout << cl.toStr() << std::endl;
	}
}

} // namespace SpeedyFOIL

int main(int argc, char* args[]) {
	std::string s(args[1]);
	std::cout << "Template file: " << s << std::endl;

	SpeedyFOIL::TemplateManager tm;
	tm.loadTemplates(s);
	tm.showTemplates();

	return 0;
}
