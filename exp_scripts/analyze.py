#!/usr/bin/env python

from myUtils import *
from config  import *

import os
import sys



SS_SIZE = "Ss.size"
GS_SIZE = "Gs.size"
OVERALL_TM = "overall_time"
REFINE_TM = "refine_time"
EXEC_TM = "execution_time"
REFINE_CT = "refine_ct"
PROG_CT = "#programs"
LOG_FILE = "logfile"

#tasks = [polysite]

usage_exit("log_dir")

log_dir = sys.argv[1]
check_exist_err(log_dir)


log_stats = dict()
for name in tasks:
    log_file = os.path.join(log_dir, name + ".log")
    check_exist_err(log_file)

    log_stats[ name ] = dict()
    log_stats[ name] [ LOG_FILE ] = log_file


for name in log_stats:
    print "process %s ... " % name
    dt = log_stats[ name ]
    lines = R( dt[LOG_FILE] )

    overall_tms = []
    refine_tms = []
    exec_tms = []
    refine_cts = []
    for line in lines:
        if line.startswith("Gs.size"):
            _,_, sz = line.split()
            dt[ GS_SIZE ] = int(sz) 
        elif line.startswith("Ss.size"):
            _,_, sz = line.split()
            dt[ SS_SIZE ] = int(sz) 
        elif "overall takes" in line:
            t = line.split()[-2]
            overall_tms.append( int(t) )
        elif "refinement takes" in line:
            t = line.split()[-2]
            refine_tms.append( int(t) )
        elif "execution takes" in line:
            t = line.split()[-2]
            exec_tms.append( int(t) )
        elif "refine_ct =" in line:
            k = line.rfind(',')
            j = line.rfind('=') + 2
            ct = int(line[j:k])
            refine_cts.append(ct)
        elif "so converged" in line:
            k = line.rfind(',')
            j = line.rfind(':') + 2
            progs = int(line[j:k])
            dt[ PROG_CT ] = progs

    dt[ OVERALL_TM ] = overall_tms
    dt[ REFINE_TM ] = refine_tms
    dt[ EXEC_TM ] = exec_tms
    dt[ REFINE_CT ] = refine_cts


for name in log_stats:
    dt = log_stats[ name ]
    res = []

    vis_progs = dt[ GS_SIZE ] + sum( dt[REFINE_CT] )
    if SS_SIZE in dt:
        vis_progs += dt[ SS_SIZE ]

    questions = len( dt[OVERALL_TM] )
    avg_time = sum( dt[OVERALL_TM] ) / questions
    out_progs = dt[ PROG_CT ]

    res.append( name )
    res.append( questions )
    res.append( avg_time/1000.0 )
    res.append( vis_progs)
    res.append( out_progs )

    print "\t".join( [ str(x) for x in res ] )

