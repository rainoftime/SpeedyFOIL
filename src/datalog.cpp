
#include "datalog.h"

namespace SpeedyFOIL {

void DatalogProgram::exploreCandidateRules() {
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

		tempM.logPO2dot( rel.getRelName() + ".dot" );

		IDBTR idb;
		idb.tm = std::move(tempM);
		idb.rules = std::move(matchings);


		idbRules.push_back( std::move(idb) );
	}

}


} // end of namespace SpeedyFOIL
