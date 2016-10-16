#ifndef DATALOG_H
#define DATALOG_H


#include "template.h"
#include <vector>


namespace SpeedyFOIL {

  struct DatalogProgram {
    std::vector<IClause> rules;

    // execute rules
  };




} // namespace SpeedyFOIL


#endif
