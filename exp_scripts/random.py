#!/usr/bin/env python

from myUtils import *
from config  import *

import socket
import sys
import os

import multiprocessing

usage_exit("path_to_foil", "data_dir", "template_dir",  "log_dir")

for f in sys.argv[1:]:
    check_exist_err(f)

foil, data_dir, t_dir, log_dir = sys.argv[1:]


def worker(foil,name, MODE, bench, tmpl, fin, fout):

    print '>> random experiment for ',  Green(name)

    k = bench[K]
    m = 2**20
    if M in bench:
        m = bench[M]

    for i in xrange(100,200):
        print '>>>> START %d-th iteration for %s' % (i, Green(name) )
        cmd =  "%s -R -B %s %s -y %d -K %d -M %d  -T %s < %s > %s" % (foil, name, MODE, i, k, m, tmpl, fin, fout + ('.%d' % i) )
        print '>>>>>> ', Green(cmd)

        #status = 0
        status = os.system(cmd)
        if status != 0:
            print '>>>>>> execution failed for %d-th iteration, status=%d, cmd= %s ' % (i, status, Yellow(cmd) )
        else:
            print '>>>> Finished %d-th iteration for %s' % (i, Green(name))


jobs = []

for name in tasks:
    bench = benchmarks[name]

    tmpl_file = get_tmpl(name)
    tmpl = os.path.join(t_dir, tmpl_file)
    fin = os.path.join(data_dir, bench[DFILE])
    fout = os.path.join(log_dir, name + ".log")

    check_exist_err(fin)

    #cmd =  "%s -B %s %s -K %d  -T %s < %s > %s" % (foil, name, MODE, bench[K], tmpl, fin, fout )
    jb = multiprocessing.Process(name = "SpeedyFOIL-r:" + name, target = worker, args=(foil,name, MODE, bench, tmpl, fin, fout,) )
    jobs.append(jb)


print "number of jos:", len(jobs)


for jb in jobs:
    jb.start()

for jb in jobs:
    jb.join()
