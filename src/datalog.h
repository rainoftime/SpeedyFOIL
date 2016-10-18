#ifndef DATALOG_H
#define DATALOG_H

#include "template.h"
#include "matching.h"

#include <vector>

namespace SpeedyFOIL {

// templates and rules for an IDB
struct IDBTR {
	TemplateManager tm;
	std::vector<IClause> rules;

	std::map<int, std::set<int>> tmpl2rules;

	std::set<std::set<int> > mostG;
	std::set<std::set<int> > mostS;

	std::set<int> chooseK(int k);
	std::set<int> refineDown();
	std::set<int> refineUp();

};

struct DatalogProgram {
	const Matching& M;

	std::vector<IDBTR> idbRules;

	DatalogProgram(const Matching& match) : M(match) {}

	void work();
	// execute rules
};

} // namespace SpeedyFOIL

#endif
