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
	std::set<int> chooseK(int k);
	std::set<int> refineDown();
	std::set<int> refineUp();

};

struct DatalogProgram {
	const DPManager* dpm;
	int prog_id;
	std::map<int, std::set<int>> state;

	DatalogProgram(int _id, const DPManager* p) : prog_id(_id), dpm(p) {}
	DatalogProgram(const DatalogProgram&) = delete;

	DatalogProgram(DatalogProgram&& dp) noexcept : prog_id(dp.prog_id), dpm(dp.dpm), state(std::move(dp.state)) {
		dp.prog_id = -1;
		dp.dpm = nullptr;
	}

	DatalogProgram& operator = (DatalogProgram&& dp)  = delete;
	/*
	DatalogProgram& operator = (DatalogProgram&& dp) noexcept {
		prog_id = dp.prog_id;
		dpm = dp.dpm;
		state = std::move(dp.state);

		dp.prog_id = -1;
		dp.dpm = nullptr;

		return *this;
	}*/

	bool operator < (const DatalogProgram& dp) const {
		return prog_id < dp.prog_id;
	}

};

struct DPManager {
	const Matching& M;
	std::vector<IDBTR> idbRules;

	int programCt;

	std::set<DatalogProgram> Gs;
	std::set<DatalogProgram> Ss;

	std::set<DatalogProgram> generalizeProg(const DatalogProgram& prog) const;
	std::set<DatalogProgram> specializeProg(const DatalogProgram& prog) const;


	DPManager(const Matching& match) : M(match), programCt(0) {}

	void exploreCandidateRules();

	// execute rules

};

} // namespace SpeedyFOIL

#endif
