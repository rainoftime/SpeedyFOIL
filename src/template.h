#ifndef CLAUSE_TEMPLATE_H
#define CLAUSE_TEMPLATE_H

#include <vector>
#include <string>

namespace SpeedyFOIL {

struct Predicate {
	int pid;
	int arity;
	std::vector<int> vdom;

	Predicate() :
			pid(0), arity(0) {
	}

	Predicate(Predicate&& pr) noexcept : pid(pr.pid), arity(pr.arity), vdom(std::move(pr.vdom)) {}

	Predicate& operator= (Predicate&& pr) noexcept {
		std::swap(pid, pr.pid);
		std::swap(arity, pr.arity);
		//std::swap(vdom, pr.vdom);
		vdom = std::move(pr.vdom);
		return *this;
	}

	std::string toStr() const;
};

struct Clause {
	Predicate hd;
	std::vector<Predicate> vbody;

	Clause() {
	}
	Clause(Clause&& cl) noexcept : hd(std::move(cl.hd)), vbody(std::move(cl.vbody)) {}
	Clause& operator= (Clause&& cl) noexcept {
		hd = std::move(cl.hd);
		vbody = std::move(cl.vbody);
		return *this;
	}

	std::string toStr() const;
};

struct TemplateManager {
	std::vector<Clause> templates;

	void loadTemplates(std::string fpath);
	void showTemplates() const;

};

struct RelationManager {
};

} // namespace SpeedyFOIL

#endif
