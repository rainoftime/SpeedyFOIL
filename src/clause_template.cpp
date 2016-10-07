

#incldue <vector>


struct Predicate {
  int pid;
  int arity;
  vector<int> vdom;
  
};

struct Clause {
  Predicate hd;
  vector<Predicate> vbody;
};
