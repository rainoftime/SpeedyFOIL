MODE = "-GS"

T_GRAPH = "graph.t"
T_MATH = "math.t"
T_PA= "pa.t"
T_LIST = "list.t"
T_KD = "knowledge.t"
T_ALL = "all.t"

TMPL = "template"
DFILE =  "dfile"
K = "K"

downcast = "downcast"
polysite = "polysite"
path = "path"
p25_modref = "p25_modref"
p50_modref = "p50_modref"
ancestor = "ancestor"
abduce = "abduce"
animals = "animals"
apisan = "apisan"
andersen = "andersen"
sgen = "samegen"
escape = "escape"
uf = "uf"
reverse = "reverse"
sort = "sort"
member = "member"
scc = "scc"
ncm = "ncm"
gcd = "gcd"
ackermann = "ackermann"
perm = "perm"

#full_modref = "full_modref"

pa_tasks = [downcast, polysite, escape, p25_modref,  andersen,]
kd_tasks = [ancestor, abduce, animals, sgen, apisan, ]
math_tasks = [ackermann, ncm, gcd,]
graph_tasks = [ path,  scc, uf ]
list_tasks = [ member, sort, perm, reverse ]

tasks = pa_tasks + math_tasks + graph_tasks + list_tasks + kd_tasks

#tasks = [polysite]


def get_tmpl(t):
    if t not in tasks:
        print t, "is not a valid tasks"
    if t in pa_tasks:
        return T_PA
    elif t in math_tasks:
        return T_MATH
    elif t in graph_tasks:
        return T_GRAPH
    elif t in list_tasks:
        return T_LIST
    elif t in kd_tasks:
        return T_KD

    print "Unknown category for ", t

    return T_ALL



benchmarks = {
    downcast : { K : 1, DFILE : "downcast.d", },
    polysite : { K : 1, DFILE : "polysite.d", },
    p25_modref   : { K : 2, DFILE : "p25_modref.d", },
    p50_modref   : { K : 2, DFILE : "p50_modref.d", },
    apisan   : { K : 2, DFILE : "apisan.d", },
    andersen : { K : 4, DFILE : "aws_andersen.d", },
    escape : { K:2, DFILE : "escape.d"},

    path     : { K : 2, DFILE : "path.d", },
    ancestor : { K : 2, DFILE : "ancestor_new.d", },
    abduce   : { K : 2, DFILE : "abduce.d", },
    animals  : { K : 1, DFILE : "animals.d", },
    sgen     : { K : 2, DFILE : "sgen.d"},
    scc     : { K : 2, DFILE : "scc.d"},


    ackermann : { K : 3,  DFILE : "ackermann.d", },
    gcd : { K : 3,  DFILE : "gcd.d", },
    ncm : { K:2, DFILE : "ncm.d", },


    member : { K:2, DFILE : "member.d", },
    sort :  { K:4, DFILE : "sort.d", },
    perm : { K:2, DFILE : "perm.d", },
    uf : { K:5, DFILE :  "uf.d"},
    reverse : { K:2,  DFILE: "list.d"},

    #full_modref : { K : 2, DFILE : "full_modref.d"},
}

