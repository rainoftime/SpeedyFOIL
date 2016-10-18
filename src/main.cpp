#include  "defns.h"
#include  "extern.h"
#include  "template.h"


#include <iostream>
#include <string>

/*  Read parameters and initialise variables  */

void initialize_usercustom() {
	FILE *ifp;
	int cnt = 0;
	char target_rel[100];
	char body_rel[100];
	ifp = fopen("custom", "r");
	if (ifp == NULL) {
		fprintf(stderr, "Can't open input file for customized refinement \n");
		exit(1);
	}
	fscanf(ifp, "%d", &cnt);
	UserInfo.head = (char**) malloc(sizeof(char*) * cnt);
	UserInfo.body = (char**) malloc(sizeof(char*) * cnt);
	UserInfo.num = cnt;
	cnt = 0;
	while (fscanf(ifp, "%s %s", target_rel, body_rel) != EOF) {
		UserInfo.head[cnt] = strdup(target_rel);
		UserInfo.body[cnt] = strdup(body_rel);
		/*		  UserInfo.head[cnt] = (char*)malloc(strlen(target_rel)+1);
		 UserInfo.body[cnt] = (char*)malloc(strlen(body_rel)+1);
		 strncpy(UserInfo.head[cnt], target_rel, sizeof(target_rel));
		 strncpy(UserInfo.body[cnt], body_rel, sizeof(body_rel)); */
		printf("%s %s\n", UserInfo.head[cnt], UserInfo.body[cnt]);
		cnt++;
	}
}

void readSDNConfig(const char* fName) {
	FILE* fin;
	char buf[1000];
	int i = 0, d;

	fin = fopen(fName, "r");
	if (fin == NULL) {
		fprintf(stderr, "Cannot open SDN configure file: %s\n", fName);
		exit(1);
	}

	sdnConfig.RelationNames = (char**) malloc(sizeof(char*) * 1000);
	sdnConfig.BodyV = (int*) malloc(sizeof(int) * 1000);

	while (fscanf(fin, "%s%d", buf, &d) != EOF) {
		sdnConfig.RelationNames[i] = strdup(buf);
		sdnConfig.BodyV[i] = d;
		++i;
	}

	sdnConfig.Number = i;
	printf("sdn config: ");
	while (i--) {
		printf("%s %d", sdnConfig.RelationNames[i], sdnConfig.BodyV[i]);
		//auto pr = [](const char*p, int x){ printf("%s %d", p, x); };
		//pr( sdnConfig.RelationNames[i], sdnConfig.BodyV[i] );
	}
	printf("\n");

	fclose(fin);
}

Boolean InSDNConfig(const char* RelName) {
	int i = 0;
	for (i = 0; i < sdnConfig.Number; ++i) {
		if (strcmp(RelName, sdnConfig.RelationNames[i]) == 0) {
			return true;
		}
	}
	return false;

}

Var GetSDNConfigBodyV(const char* RelName) {
	int i = 0;
	for (i = 0; i < sdnConfig.Number; ++i) {
		if (strcmp(RelName, sdnConfig.RelationNames[i]) == 0) {
			return sdnConfig.BodyV[i];
		}
	}
	//return -1;  
	return 255;
}

int main(int Argc, char *Argv[])
/*    ----  */
{
	int o, i, Cases, Errors;
	extern char *optarg;
	extern int optind;
	Boolean FirstTime = true;
	Var V;
	extern Var *Arg; /* in literal.c */
	Relation R;
	Tuple Case;
	char Line[200], PlusOrMinus;

	SpeedyFOIL::Matching M;
	std::string s;


	/* Check overlaying of Const and float */
	if (sizeof(Const) != sizeof(float)) {
		printf("Integers and floating point numbers are different sizes\n");
		printf("Alter declaration of type Const (defns.i) and recompile\n");
		exit(1);
	}

	printf("FOIL 6.4   [January 1996]\n--------\n");

	/*  Process options  */

	while ((o = getopt(Argc, Argv, "pnNus:a:f:g:V:d:A:w:l:t:m:v:y:T:")) != EOF) {
		if (FirstTime) {
			printf("\n    Options:\n");
			FirstTime = false;
		}

		switch (o) {
		case 'n':
			NEGLITERALS = NEGEQUALS = false;
			printf("\tno negated literals\n");
			break;

		case 'N':
			NEGLITERALS = false;
			printf("\tnegated literals only for =, > relations\n");
			break;

		case 'p':
			IGNORE_NEG = true;
			break;
		case 'u':
			USERCUSTOM = true;
			initialize_usercustom();
			/*UNIFORMCODING = true;
			 printf("\tuniform coding of literals not available\n"); */
			break;

		case 's':
			SAMPLE = atof(optarg);
			printf("\tsample %g%% of negative tuples\n", SAMPLE);
			SAMPLE /= 100;
			break;

		case 'a':
			MINACCURACY = atof(optarg);
			printf("\tminimum clause accuracy %f%%\n", MINACCURACY);
			MINACCURACY /= 100;
			break;

		case 'f':
			MINALTFRAC = atof(optarg);
			printf("\tbacked-up literals have %g%% of best gain\n", MINALTFRAC);
			MINALTFRAC /= 100;
			break;

		case 'g':
			i = atoi(optarg);
			printf("\tuse determinate literals when gain <= ");
			printf("%d%% possible\n", i);
			DETERMINATE = i / 100.0;
			break;

		case 'V':
			MAXVARS = atoi(optarg);
			if (MAXVARS > pow(2.0, 8 * sizeof(Var) - 1.0)) {
				MAXVARS = pow(2.0, 8 * sizeof(Var) - 1.0) - 0.9;
			}
			printf("\tallow up to %d variables\n", MAXVARS);
			break;

		case 'd':
			MAXVARDEPTH = atoi(optarg);
			printf("\tmaximum variable depth %d\n", MAXVARDEPTH);
			break;

		case 'w':
			MAXWEAKLITS = atoi(optarg);
			printf("\tallow up to %d consecutive weak literals\n", MAXWEAKLITS);
			break;

		case 'l':
			MAXPOSSLIT = atoi(optarg) + 1;
			printf("\tmaximum %d backups from one literal\n", MAXPOSSLIT - 1);
			break;

		case 't':
			MAXALTS = atoi(optarg);
			printf("\tmaximum %d total backups\n", MAXALTS);
			break;

		case 'm':
			MAXTUPLES = atoi(optarg);
			printf("\tmaximum %d tuples \n", MAXTUPLES);
			break;

		case 'v':
			VERBOSITY = atoi(optarg);
			printf("\tverbosity level %d\n", VERBOSITY);
			break;

		case 'y':
			SDNCONSTRAINT = true;
			readSDNConfig(optarg);
			break;

		case 'T':
			s = std::string(optarg);
			M.tm.loadTemplates(s);
			M.tm.showTemplates();
			M.tm.buildPartialOrder();
			break;

		case '?':
			printf("unrecognised option\n");
			exit(1);
		}
	}

	/*  Set up predefined relations.

	 These are treated specially in Join().  Rather than giving explicit
	 pos tuples, the Pos field identifies the relation  */

	/*  Note: EQCONST and GTCONST take one argument and one parameter
	 (a theory constant or floating-point threshold).  To simplify the
	 code for all other relations, this parameter is packed into a
	 "standard" arglist; the number of variable positions that it
	 occupies will depend on the implementation.  */

	Reln = Alloc(10, Relation);

	EQVAR= AllocZero(1, struct _rel_rec);

	EQVAR->Name = "=";
	EQVAR->Arity = 2;

	EQVAR->Type = AllocZero(3, int);
	EQVAR->TypeRef = AllocZero(3, TypeInfo);

	EQVAR->Pos = (Tuple *) 0;
	EQVAR->BinSym = true;

	EQCONST= AllocZero(1, struct _rel_rec);

	EQCONST->Name = "==";
	EQCONST->Arity = 1;

	EQCONST->Type = AllocZero(2, int);
	EQCONST->TypeRef = AllocZero(2, TypeInfo);

	EQCONST->Pos = (Tuple *) 1;
	EQCONST->BinSym = false;

	GTVAR= AllocZero(1, struct _rel_rec);

	GTVAR->Name = ">";
	GTVAR->Arity = 2;

	GTVAR->Type = AllocZero(3, int);
	GTVAR->TypeRef = AllocZero(3, TypeInfo);

	GTVAR->Pos = (Tuple *) 2;
	GTVAR->BinSym = true;

	GTCONST= AllocZero(1, struct _rel_rec);

	GTCONST->Name = ">";
	GTCONST->Arity = 1;

	GTCONST->Type = AllocZero(2, int);
	GTCONST->TypeRef = AllocZero(2, TypeInfo);

	GTCONST->Pos = (Tuple *) 3;
	GTCONST->BinSym = false;

	MaxRel = 3;

	ForEach(i, 0, MaxRel)
	{
		Reln[i]->PossibleTarget = false;
	}

	/*  Read input  */

	ReadTypes();

	CheckTypeCompatibility();

	ReadRelations();

	/*  Initialise all global variables that depend on parameters  */

	Variable = Alloc(MAXVARS + 1, VarInfo);
	DefaultVars = Alloc(MAXVARS + 1, Var);

	ForEach(V, 0, MAXVARS)
	{
		Variable[V] = Alloc(1, struct _var_rec);

		if (V == 0) {
			//Variable[0]->Name = "_";
			Variable[0]->Name = Alloc(2, char);
			Variable[0]->Name[0] = '-';
			Variable[0]->Name[1] = '\0';

		} else if (V <= 26) {
			Variable[V]->Name = Alloc(2, char);
			Variable[V]->Name[0] = 'A' + V - 1;
			Variable[V]->Name[1] = '\0';
		} else {
			Variable[V]->Name = Alloc(3, char);
			Variable[V]->Name[0] = 'A' + ((V - 27) / 26);
			Variable[V]->Name[1] = 'A' + ((V - 27) % 26);
			Variable[V]->Name[2] = '\0';
		}

		DefaultVars[V] = V;
	}

	Barred = AllocZero(MAXVARS + 1, Boolean);

	ToBeTried = Alloc(MAXALTS + 1, Alternative);
	ForEach(i, 0, MAXALTS)
	{
		ToBeTried[i] = Alloc(1, struct _backup_rec);
		ToBeTried[i]->UpToHere = Nil;
	}

	PartialOrder = Alloc(MAXARGS + 1, Boolean *);
	ForEach(V, 1, MAXARGS)
	{
		PartialOrder[V] = Alloc(MAXVARS + 1, Boolean);
	}

	Possible = Alloc(MAXPOSSLIT + 1, PossibleLiteral);
	ForEach(i, 1, MAXPOSSLIT)
	{
		Possible[i] = Alloc(1, struct _poss_lit_rec);
		Possible[i]->Args = Alloc(MAXARGS + 1, Var);
	}

	Arg = Alloc(MAXARGS + 1, Var);
	Arg[0] = 0; /* active */

	/*  Allocate space for trial recursive call */

	RecursiveLitOrders = Alloc(1, Ordering *);
	RecursiveLitOrders[0] = Alloc(MAXARGS + 1, Ordering);


	/*  Find plausible orderings  for constants of each type  */

	OrderConstants();


	// test SpeedyFOIL
	bool testSpeedy = true;

	printf("will test SpeedyFOIL soon...\n");


	M.relm.loadRelations(RelnOrder, MaxRel+1);
	M.relm.showRelations();
	//M.work();
	M.work2();


	if(testSpeedy) {
		return 0;
	}


	/* Find Definitions */

	ForEach(i, 0, MaxRel)
	{
		R = RelnOrder[i];

		if (R->PossibleTarget) {
			FindDefinition(R);
		}
	}

	/*  Test definitions  */

	while (fgets(Line, sizeof(Line), stdin)) {
		R = Nil;
		for (i = 0; i <= MaxRel && !R; i++) {
			if (!strcmp(RelnOrder[i]->Name, Line))
				R = RelnOrder[i];
		}

		if (!R) {
			printf("\nUnknown relation %s\n", Line);
			exit(1);
		} else {
			printf("\nTest relation %s\n", Line);
		}

		Cases = Errors = 0;
		Current.MaxVar = HighestVarInDefinition(R);

		while ((Case = ReadTuple(R))) {
			while ((PlusOrMinus = getchar()) == ' ' || PlusOrMinus == '\t')
				;
			ReadToEOLN
				;

			Cases++;

			if (Interpret(R, Case) != (PlusOrMinus == '+')) {
				Verbose(1) {
					printf("    (%c)", PlusOrMinus);
					PrintTuple(Case, R->Arity, R->TypeRef, false);
				}
				Errors++;
			}
		}

		printf("Summary: %d error%s in %d trial%s\n", Errors, Plural(Errors),
				Cases, Plural(Cases));
	}

	return 0;
}
