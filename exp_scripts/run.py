#!/usr/bin/env python

from myUtils import *
from config  import *

import socket
import sys
import os

import multiprocessing

tasks = [polysite, downcast]

usage_exit("path_to_foil", "test_data_dir", "log_dir")

for f in sys.argv[1:]:
    check_exist_err(f)

foil, data_dir, log_dir = sys.argv[1:]


def worker(cmd):
    print '>>>> cmd: ', Green(cmd)
    status = 0
    #status = os.system(cmd)
    if status != 0:
        print '>>>>>> execution failed, status=', status, ', cmd=', Yellow(cmd)
    else:
        print '>>>>>> ', Green('Done')



jobs = []

for name in tasks:
    bench = benchmarks[name]

    tmpl = os.path.join(data_dir, TMPL_FILE)
    fin = os.path.join(data_dir, bench[DFILE])
    fout = os.path.join(log_dir, name + ".log")

    check_exist_err(fin)

    cmd =  "%s %s -K %d  -T %s < %s > %s" % (foil, MODE, bench[K], tmpl, fin, fout )
    jb = multiprocessing.Process(target = worker, args=(cmd,) )
    jobs.append(jb)


print "number of jos:", len(jobs)


for jb in jobs:
    jb.start()

for jb in jobs:
    jb.join()
