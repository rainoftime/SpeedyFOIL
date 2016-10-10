
#include <template.h>
#include <vector>

namespace SpeedyFOIL {

struct Matching {

  // all templates
  TemplateManager tm;

  // Relation Definitions
  // RelationManager relm


  /*
    - for each template, use DFS to enumerate all possible matches with arity / type pruning 

    - each matching/instantiation is a candidate rule, eliminate inconsistent ones

    - enumerate K consistent programs
    -- in case of no consistent programs, mutate templates and try again
    
   */
};


}
