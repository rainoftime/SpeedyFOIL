/******************************************************************************/
/*									      */
/*	Variables defined in global.c					      */
/*									      */
/******************************************************************************/

#ifndef EXTERN_H
#define EXTERN_H

extern SDNConfig sdnConfig;
extern Var SDNCommonV;

extern Boolean
    IGNORE_NEG,
    USERCUSTOM,
  SDNCONSTRAINT,
	NEGLITERALS,
	NEGEQUALS,
	UNIFORMCODING,

	MissingVals,
	AnyPartialOrder,
	*Barred,
	OutOfWorld;


extern float
	SAMPLE,
	MINACCURACY,
	MINALTFRAC,
	DETERMINATE;

extern int
	MAXVARS,
	MAXARGS,
	MAXWEAKLITS,

	MAXPOSSLIT,
	MAXALTS,
	MAXVARDEPTH,
	MAXRECOVERS,
	MAXTUPLES,
	VERBOSITY,

	MaxConst,
	MaxType,
	MaxRel,

	AllTuples,
	NCl,
	NLit,
	NDetLits,
	NWeakLits,
	SavedClauseCover;


extern PossibleLiteral
	AlterSavedClause;


extern char
	**ConstName,
	*Flags,
    *UserFile;

extern UserCustom UserInfo;

extern Relation
	fakeRelation,
	*Reln,
	*RelnOrder,
	Target;


extern State
	StartDef,
	StartClause,
	Current,
	New;


extern float
	*LogFact,
	MaxPossibleGain,
	ClauseBits,
	AvailableBits,
	AdjMinAccuracy,
	SavedClauseAccuracy;


extern Clause
	NewClause,
	SavedClause;

extern Boolean
	**PartialOrder,
	**Compatible;

extern Ordering
	**RecursiveLitOrders;

extern int
	NRecLitClause,
	NRecLitDef;

extern VarInfo
	*Variable;

extern Var
	*DefaultVars;

extern TypeInfo
	*Type;

extern Tuple		*Found;
extern int		NFound;

extern Alternative	*ToBeTried;
extern int		NToBeTried;

extern PossibleLiteral	*Possible;
extern int		NPossible,
			NDeterminate;

#endif
