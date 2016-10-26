#!/usr/bin/env python

from myUtils import *

import socket
import sys
import os

import multiprocessing


MODE = "-GS"

tmpl_file = "tmp_alps.txt"
dfile =  "dfile"
K = "K"


benchmarks = {
    "downcast" : {
        K : 1,
        dfile : "downcast.d",
    },

    "polysite" : {
        K : 1,
        dfile : "polysite.d",
    },

    "path" : {
        K : 2,
        dfile : "path.d",
    },

    "modref" : {
        K : 2,
        dfile : "modref.d",
    },

    "ancestor" : {
        K : 2,
        dfile : "ancestor_new.d",
    },

    "abduce" : {
        K : 2,
        dfile : "abduce.d",
    },


    "animals" : {
        K : 1,
        dfile : "animals.d",
    },

    "apisan" : {
        K : 2,
        dfile : "apisan.d",
    },


    "andersen" : {
        K : 4,
        dfile : "aws_andersen.d",
    },    
}


usage_exit("path_to_foil", "test_data_dir", "log_dir")

for f in sys.argv[1:]:
    check_exist_err(f)

foil, data_dir, log_dir = sys.argv[1:]


def worker(cmd):
    print '>>>> cmd: ', Green(cmd)
    #status = 0
    status = os.system(cmd)
    if status != 0:
        print '>>>>>> execution failed, status=', status, ', cmd=', Yellow(cmd)
    else:
        print '>>>>>> ', Green('Done')



jobs = []

for name in benchmarks:
    bench = benchmarks[name]

    tmpl = os.path.join(data_dir, tmpl_file)
    fin = os.path.join(data_dir, bench[dfile])
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
