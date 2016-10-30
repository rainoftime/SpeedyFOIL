MODE = "-GS"

T_GRAPH = "tmpl_graph.txt"
T_MATH = "tmpl_math.txt"
T_ANALYSIS = "tmpl_analysis.txt"
T_DS = "tmpl_ds.txt"
T_ALL = "tmpl_overall.txt"

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

pa_tasks = [downcast, polysite, escape, p25_modref, apisan, andersen,]
math_tasks = [ackermann, ncm, gcd,]
graph_tasks = [ path, ancestor, abduce, animals,  sgen, scc ]
ds_tasks = [ member, sort, perm, uf, reverse ]

tasks = pa_tasks + math_tasks + graph_tasks + ds_tasks

#tasks = [polysite]


def get_tmpl(t):
    if t not in tasks:
        print Yellow(t), "is not a valid tasks"
    if t in pa_tasks:
        return T_ANALYSIS
    if t in math_tasks:
        return T_MATH
    if t in graph_tasks:
        return T_GRAPH
    if t in ds_tasks:
        return T_DS

    print "Unknown category for ", Yellow(t)

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

