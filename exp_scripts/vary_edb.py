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


def worker(cmd):
    print '>>>>>> ', Green(cmd)
    #status = 0
    status = os.system(cmd)
    if status != 0:
        print '>>>>>> execution failed for %d-th iteration, status=%d, cmd= %s ' % (i, status, Yellow(cmd) )
    else:
        print '>>>> Finished %d-th iteration for %s' % (i, Green(name))


jobs = []

name = andersen
bench = benchmarks[name]

tmpl_file = get_tmpl(name)
tmpl = os.path.join(t_dir, tmpl_file)

for i in xrange(10):
    fin = os.path.join(data_dir, "%s.d.%d" % (name, i) )
    fout = os.path.join(log_dir, name + ".log.%d" % i )

    check_exist_err(fin)

    cmd = "%s -B andersen -G -K 4 -T %s < %s > %s" % (foil, tmpl, fin, fout )
    jb = multiprocessing.Process(target = worker, args=(cmd,) )
    jobs.append(jb)


print "number of jos:", len(jobs)


for jb in jobs:
    jb.start()

for jb in jobs:
    jb.join()
