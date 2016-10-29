MODE = "-GS"

TMPL_FILE = "tmp_alps.txt"
DFILE =  "dfile"
K = "K"

downcast = "downcast"
polysite = "polysite"
path = "path"
modref = "modref"
ancestor = "ancestor"
abduce = "abduce"
animals = "animals"
apisan = "apisan"
andersen = "andersen"
sgen = "samegen"
full_modref = "full_modref"

tasks = [downcast, polysite, path, modref, ancestor, abduce, animals, apisan, andersen, sgen, full_modref ]
#tasks = [polysite]

benchmarks = {
    downcast : { K : 1, DFILE : "downcast.d", },

    polysite : { K : 1, DFILE : "polysite.d", },

    path     : { K : 2, DFILE : "path.d", },

    modref   : { K : 2, DFILE : "modref.d", },

    ancestor : { K : 2, DFILE : "ancestor_new.d", },

    abduce   : { K : 2, DFILE : "abduce.d", },

    animals  : { K : 1, DFILE : "animals.d", },

    apisan   : { K : 2, DFILE : "apisan.d", },

    andersen : { K : 4, DFILE : "aws_andersen.d", },

    sgen     : { K : 2, DFILE : "sgen.d"},

    full_modref : { K : 2, DFILE : "full_modref.d"},
}

