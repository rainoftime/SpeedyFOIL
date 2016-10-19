
#include "datalog.h"


#include <vector>
#include <iostream>
#include <algorithm>

namespace SpeedyFOIL {

int DatalogProgram::programCt = 0;

namespace {

std::vector<std::set<int>> chooseK_helper(int n, int i, const std::vector<int>& v) {
	std::vector<std::set<int>> res;

	if(n > v.size()){
		return res;
	}

	if (i == 0 || n == 0) {
		res.push_back( std::set<int>() );
		return res;
	}

	auto encode = [&v](int x) {return v[x-1];};

	// choose n
	std::vector<std::set<int>> tmp1 = chooseK_helper(n-1, i-1, v);
	for(std::set<int>& x : tmp1) {
		x.insert( encode(n) );
		res.push_back( std::move(x) );
	}

	// don't choose n
	std::vector<std::set<int>> tmp2 = chooseK_helper(n-1, i, v);
	for(std::set<int>& x : tmp2) {
		res.push_back( std::move(x) );
	}

	return res;
}

} // end of anonymous namespace

std::set<int> IDBTR::extractRules(const std::set<int>& tmpls) const {
	std::set<int> res;
	for (const int x : tmpls) {
		auto it = tmpl2rules.find(x);
		if(it == tmpl2rules.end()) {
			continue;
		}

		for (const int e : it->second) {
			res.insert(e);
		}
	}
	return res;
}

// assume tm, rules  are already given
// initialize tmpl2rules, mostG, mostS
void IDBTR::init(){
	const int num_rules = rules.size();
	const int num_tmpls = tm.templates.size();

	// initialize tmpl2rules
	tmpl2rules.clear();

	for(int i=0; i < num_rules; ++i) {
		const IClause& cl = rules[i];
		int j = std::find(tm.templates.begin(), tm.templates.end(), cl.tc) - tm.templates.begin();
		if(j < num_tmpls) {
			tmpl2rules[j].insert(i);
		}
		else {
			std::cerr << "ERROR: IDBTR::int() cannot find template " << cl.tc.toStr() << " in tm\n";
		}
	}

	// initialize mostG/S
	//mostG.clear();
	//mostG.insert( extractRules(tm.getMostGeneral()) );
	//mostS.clear();
	//mostS.insert( extractRules(tm.getMostSpecific()) );

}

std::set<int> IDBTR::generalize(int rule_index) const{
	std::set<int> res;
	return res;
}

std::set<int> IDBTR::specialize(int rule_index) const{
	std::set<int> res;
	return res;
}


std::vector<std::set<int>> IDBTR::chooseK(int k, bool general) const {
	if(k >= 3) {
		std::vector<std::set<int>> res;
		std::cerr << "K is too large: " << k << std::endl;
		return res;
	}

	std::set<int> st;

	if(general) {
		st = extractRules( tm.getMostGeneral() );
	}
	else{
		st = extractRules( tm.getMostSpecific() );
	}

	std::vector<int> v(st.begin(), st.end());

	return chooseK_helper(v.size(), k, v);
}


void DPManager::exploreCandidateRules() {
	for(const TRelation& rel : M.relm.vIDBRel) {

		std::cout << "Relation: " << rel.getRelNameWithTypes( ) << std::endl;
		std::vector<TClause> candidates = M.tm.findAllPossilbeMatchings(rel);
		std::vector<TClause> useful_templates;

		std::vector<IClause> matchings;

		for(const TClause& tc : candidates) {
			std::vector<IClause> Ms = M.findMatchingsWithTemplate(rel, tc);

			if(Ms.size()) {
				std::cout << ">> Template: " << tc.toStr() << ",  matches: " << Ms.size() << std::endl;

				useful_templates.push_back(tc);
			}

			for(IClause& m : Ms) {
				matchings.push_back( std::move(m) );
			}
		}

		TemplateManager tempM;
		tempM.templates = std::move(useful_templates);
		tempM.buildPartialOrder();

		std::cout << "overall templates: " << tempM.templates.size() << std::endl;
		std::cout << "most general: " << tempM.getMostGeneral().size() << std::endl;
		std::cout << "most specific: " << tempM.getMostSpecific().size() << std::endl;
		std::cout << "independent: " << tempM.getIndependent().size() << std::endl;

		//tempM.logPO2dot( rel.getRelName() + ".dot" );

		IDBTR idb(rel);
		idb.tm = std::move(tempM);
		idb.rules = std::move(matchings);


		idbRules.push_back( std::move(idb) );
	}

	initGS();
}

std::set<DatalogProgram> DPManager::refineProg(const DatalogProgram& prog, bool specialize) const {
	std::set<DatalogProgram> res;

	for(auto pr : prog.state) {
		int idb_index = pr.first;
		const IDBTR & idb = idbRules[idb_index];

		const std::set<int>& rule_st = pr.second;

		// specialize one rule
		for(int rl : rule_st) {
			// try all candidates for one rule
			std::set<int> candidates = specialize ? idb.specialize(rl) : idb.generalize(rl);
			for(int can : candidates) {
				std::set<int> cp_st = rule_st;
				// NOTE: erase one then insert one,
				// #rules for an IDB does not change
				cp_st.erase(rl);
				cp_st.insert(can);

				std::map<int, std::set<int>> cp_state = prog.state;
				cp_state[idb_index] = std::move(cp_st);

				DatalogProgram dp(this);
				dp.state = std::move(cp_state);

				// NOTE: this insertion happens in three layer nested loops, could explode!!
				res.insert( std::move(dp) );
			}
		}
	}

	return res;
}

void DPManager::initGS() {


	//const IDBTR& idb = idbRules[k];

	// for each IDB choose 1~2 rules

	long long space = 1;

	const int sz = idbRules.size();
	for(int i=0; i<sz; ++i) {
		const IDBTR& idb = idbRules[i];
		std::vector<std::set<int>>  vst = idb.chooseK(2, true);

		std::cout << "Relation: " << idb.rel.getRelNameWithTypes() << ", #rules: " << vst.size() << std::endl;
		space *= vst.size();
	}

	std::cout << "space: " << space << std::endl;

}

} // end of namespace SpeedyFOIL
