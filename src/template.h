#ifndef CLAUSE_TEMPLATE_H
#define CLAUSE_TEMPLATE_H

#include "defns.h"

#include <vector>
#include <string>
#include <map>

namespace SpeedyFOIL {

struct TPredicate;
struct TClause;
struct TemplateManager;
struct RelationManager;
struct TRelation;

struct IClause;


struct TPredicate {
	int pid;
	int arity;
	std::vector<int> vdom;

	TPredicate() :
			pid(0), arity(0) {
	}

	TPredicate(TPredicate&& pr) noexcept : pid(pr.pid), arity(pr.arity), vdom(std::move(pr.vdom)) {}

	TPredicate& operator= (TPredicate&& pr) noexcept {
		std::swap(pid, pr.pid);
		std::swap(arity, pr.arity);
		//std::swap(vdom, pr.vdom);
		vdom = std::move(pr.vdom);
		return *this;
	}

	std::string toStr() const;
};

struct TClause {
	TPredicate hd;
	std::vector<TPredicate> vbody;

	TClause() {}

	TClause(TClause&& cl) noexcept : hd(std::move(cl.hd)), vbody(std::move(cl.vbody)) {}
	TClause& operator= (TClause&& cl) noexcept {
		hd = std::move(cl.hd);
		vbody = std::move(cl.vbody);
		return *this;
	}

	std::string toStr() const;
};

// instantiated clause
struct IClause {
	const TClause& tc;
	const TRelation& cl_hd;

	IClause(const TClause& c,
			const TRelation& r,
			std::vector<TRelation>& vrel): tc(c), cl_hd(r), cl_body(std::move(vrel)) {}

	IClause(IClause&& cl) noexcept: tc(cl.tc), cl_hd(cl.cl_hd), cl_body(std::move(cl.cl_body))  {}

	/*
	 * assignment should be forbidden
	IClause& operator= (IClause&& cl) noexcept{
		tc = cl.tc;
		cl_hd = cl.cl_hd;
		cl.cl_body = std::move(cl.cl_body);
		return *this;
	}
	 */
	std::vector<TRelation> cl_body;

	void explain();
};

struct TemplateManager {
	std::vector<TClause> templates;

	void loadTemplates(std::string fpath);
	void showTemplates() const;

	std::vector<TClause> findAllPossilbeMatchings(const TRelation&) const;

};

// a wrapper of the old Relation
struct TRelation {
	Relation pRel;
	TRelation(Relation p): pRel(p){}

	Relation getRel() const {return pRel;}
	int getArity() const {return pRel->Arity;}
	TypeInfo getType(int k) const {return pRel->TypeRef[k]; }
	std::string getRelName() const {return std::string(pRel->Name);}

	std::vector<std::string> getStrs() const;

	bool possibleMatch(const TPredicate &) const;
};


struct RelationManager {
	std::vector<TRelation> vEDBRel;
	std::vector<TRelation> vIDBRel;

	void loadRelations(Relation*, int);
	void showRelations() const;
	std::vector<TRelation> finPossibleInst(const TPredicate&, bool onlyIDB=true) const;
};



struct Matching {

  // all templates
  TemplateManager tm;

  // Relation Definitions
  RelationManager relm;

  /*
    - for each template, use DFS to enumerate all possible matches with arity / type pruning

    - each matching/instantiation is a candidate rule, eliminate inconsistent ones

    - enumerate K consistent programs
    -- in case of no consistent programs, mutate templates and try again

   */

  std::vector<IClause> findAllMatchings(const TRelation & rel) const;

  std::vector<IClause> findMatchingsWithTemplate(const TRelation& rel, const TClause& tc) const;

  bool exploreBody(const TClause& tc,
		  std::vector<TRelation>& rels,
		  std::map<int, Relation>& relMap,
		  std::map<int, TypeInfo>& tyMap) const;

  void work();
};



} // namespace SpeedyFOIL

#endif
