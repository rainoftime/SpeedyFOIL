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


def worker(foil, common, fout):

    #status = 0

    cmd = "%s  %s  > %s" % (foil, common, fout)
    print '>>>>>> ', Green(cmd)

    status = os.system(cmd)
    if status != 0:
        print '>>>>>> execution failed  status=%d, cmd= %s ' % ( status, Yellow(cmd) )
    else:
        print '>>>> Finished for %s' % Green(cmd)

    return

    # for i in xrange(100):
    #     cmd = "%s -R -y %d  %s > %s " % (foil, i, common, fout + ".r%d" % i)
    #     print '>>>>>> ', Green(cmd)

    #     status = os.system(cmd)
    #     if status != 0:
    #         print '>>>>>> execution failed at %d-th iteration, status=%d, cmd= %s ' % (i, status, Yellow(cmd) )
    #     else:
    #         print '>>>> Finished for %d-th iteration %s' % (i, Green(cmd))



jobs = []

name = andersen
bench = benchmarks[name]

tmpl_file = get_tmpl(name)
tmpl = os.path.join(t_dir, tmpl_file)

#R = [100,1000]
#R = range(10,21)
R = range(21)
R.append(100)
R.append(1000)

#for i in xrange(10):
for i in R:
    fin = os.path.join(data_dir, "%s.d.%d" % (name, i) )
    fout = os.path.join(log_dir, name + ".log.%d" % i )

    check_exist_err(fin)


    comm =  " -B andersen -G -K 4 -T %s < %s " % (tmpl, fin)
    #cmd = "%s -B andersen -G -K 4 -T %s < %s > %s" % (foil, tmpl, fin, fout )
    jb = multiprocessing.Process(target = worker, args=(foil, comm, fout) )
    jobs.append(jb)


print "number of jos:", len(jobs)

i = 0
while i < len(jobs):
    for j in xrange(12):
        if i + j < len(jobs):
            jobs[i+j].start()

    for j in xrange(12):
        if i + j < len(jobs):
            jobs[i+j].join()

    i += 12

# for jb in jobs:
#     jb.start()

# for jb in jobs:
#     jb.join()
