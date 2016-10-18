#ifndef DATALOG_H
#define DATALOG_H

#include "template.h"
#include "matching.h"

#include <vector>

namespace SpeedyFOIL {

struct DPManager;
struct DatalogProgram;
struct IDBTR;

// templates and rules for an IDB
struct IDBTR {
	TemplateManager tm;
	std::vector<IClause> rules;

	std::map<int, std::set<int>> tmpl2rules;
	//std::set<std::set<int> > mostG;
	//std::set<std::set<int> > mostS;

	IDBTR(){}

	IDBTR(IDBTR&& idb) noexcept :
			tm(std::move(idb.tm)),
			rules(std::move(idb.rules)),
			tmpl2rules(std::move(idb.tmpl2rules)) {}
			//mostG(std::move(idb.mostG)),
			//mostS(std::move(idb.mostS)) {}

	IDBTR& operator = (IDBTR&& idb) noexcept {
		tm = std::move(idb.tm);
		rules = std::move(idb.rules);
		tmpl2rules = std::move(idb.tmpl2rules);
		//mostG = std::move(idb.mostG);
		//mostS = std::move(idb.mostS);

		return *this;
	}

	void init();
	std::set<int> extractRules(const std::set<int>&) const;
	std::set<int> chooseK(int k) const;
	std::set<int> generalize(int rule_index) const;
	std::set<int> specialize(int rule_index) const;

};

struct DatalogProgram {
	static int programCt;

	const DPManager* dpm;
	int prog_id;
	std::map<int, std::set<int>> state;

	DatalogProgram(const DPManager* p) : prog_id(++programCt), dpm(p) {}
	DatalogProgram(const DatalogProgram&) = delete;

	DatalogProgram(DatalogProgram&& dp) noexcept : prog_id(dp.prog_id), dpm(dp.dpm), state(std::move(dp.state)) {
		dp.prog_id = -1;
		dp.dpm = nullptr;
	}

	DatalogProgram& operator = (DatalogProgram&& dp)  = delete;

	bool operator < (const DatalogProgram& dp) const {
		return prog_id < dp.prog_id;
	}

};

struct DPManager {
	const Matching& M;
	std::vector<IDBTR> idbRules;


	std::set<DatalogProgram> Gs;
	std::set<DatalogProgram> Ss;

	std::set<DatalogProgram> generalizeProg(const DatalogProgram& prog) const;
	std::set<DatalogProgram> refineProg(const DatalogProgram& prog, bool specialize) const;


	DPManager(const Matching& match) : M(match) {}

	void exploreCandidateRules();

	// execute rules

};

} // namespace SpeedyFOIL

#endif
