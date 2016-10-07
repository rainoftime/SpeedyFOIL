
#include <vector>


struct Predicate {
  int pid;
  int arity;
  std::vector<int> vdom;
  
};

struct Clause {
  Predicate hd;
  std::vector<Predicate> vbody;
};
