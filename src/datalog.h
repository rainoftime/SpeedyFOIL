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

	IDBTR(){}

	IDBTR(IDBTR&& idb) noexcept :
			tm(std::move(idb.tm)),
			rules(std::move(idb.rules)),
			tmpl2rules(std::move(idb.tmpl2rules)),
			mostG(std::move(idb.mostG)),
			mostS(std::move(idb.mostS)) {}

	IDBTR& operator = (IDBTR&& idb) noexcept {
		tm = std::move(idb.tm);
		rules = std::move(idb.rules);
		tmpl2rules = std::move(idb.tmpl2rules);
		mostG = std::move(idb.mostG);
		mostS = std::move(idb.mostS);

		return *this;
	}

	std::set<int> chooseK(int k);
	std::set<int> refineDown();
	std::set<int> refineUp();

};

struct DatalogProgram {
	const Matching& M;

	std::vector<IDBTR> idbRules;

	DatalogProgram(const Matching& match) : M(match) {}

	void exploreCandidateRules();

	// execute rules

};

} // namespace SpeedyFOIL

#endif
