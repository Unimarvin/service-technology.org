/*!
\file Lexic.ll
\author Erik
\status not approved

Joint lexic for all inputs to LoLA
Mainly copied from LoLA1

\todo Herausfinden, ob es Probleme bei zu langen Kommentaren/Bezeichnern gibt. Idee: Maximallänge angeben.
\todo Präfix hinzufügen?
*/

/* we want line numbering */
%option yylineno  
/* we don't neet yyunput() */
%option nounput
%option noyywrap

%option outfile="lex.yy.c"
%option prefix="ptbuechi_"


%{
#include <cmdline.h>

#include <Parser/ast-system-k.h>
#include <Parser/ast-system-yystype.h>

#include <Parser/ArcList.h>
#include <Parser/FairnessAssumptions.h>
#include <Parser/ParserPTBuechi.hh>
#include <InputOutput/Reporter.h>
#include <InputOutput/InputOutput.h>

using namespace kc;

extern int currentFile;
extern gengetopt_args_info args_info;
extern Reporter* rep;
extern Input *netFile;

void ptbuechi_setcol();
extern void ptbuechi_error(char const* mess);
int ptbuechi_colno = 1;
%}

%s IN_COMMENT

%%

 /* from http://flex.sourceforge.net/manual/How-can-I-match-C_002dstyle-comments_003f.html */
"/*"                                     { ptbuechi_setcol(); BEGIN(IN_COMMENT); }
<IN_COMMENT>"*/"                         { ptbuechi_setcol(); BEGIN(INITIAL); }
<IN_COMMENT>[^*\n\r]+                    { ptbuechi_setcol(); /* comments */ }
<IN_COMMENT>"*"                          { ptbuechi_setcol(); /* comments */ }
<IN_COMMENT>[\n\r]                       { ptbuechi_setcol(); /* comments */ }

FIREABLE                                 { ptbuechi_setcol(); return _FIREABLE_; }
DEADLOCK                                 { ptbuechi_setcol(); return _DEADLOCK_; }
INITIAL                                  { ptbuechi_setcol(); return _INITIAL_; }

BUECHI                                   { ptbuechi_setcol(); return _BUECHI_; }
AND                                      { ptbuechi_setcol(); return _AND_; }
NOT                                      { ptbuechi_setcol(); return _NOT_; }
OR                                       { ptbuechi_setcol(); return _OR_; }
XOR                                      { ptbuechi_setcol(); return _XOR_; }
TRUE                                     { ptbuechi_setcol(); return _TRUE_; }
FALSE                                    { ptbuechi_setcol(); return _FALSE_; }

ALLPATH                                  { ptbuechi_setcol(); return _ALLPATH_; }
EXPATH                                   { ptbuechi_setcol(); return _EXPATH_; }

ALWAYS                                   { ptbuechi_setcol(); return _ALWAYS_; }
EVENTUALLY                               { ptbuechi_setcol(); return _EVENTUALLY_; }
UNTIL                                    { ptbuechi_setcol(); return _UNTIL_; }
NEXTSTATE                                { ptbuechi_setcol(); return _NEXTSTATE_; }
RELEASE                                  { ptbuechi_setcol(); return _RELEASE_; }

REACHABLE                                { ptbuechi_setcol(); return _REACHABLE_; }
INVARIANT                                { ptbuechi_setcol(); return _INVARIANT_; }
IMPOSSIBLE                               { ptbuechi_setcol(); return _IMPOSSIBLE_; }

\;                                       { ptbuechi_setcol(); return _semicolon_; }
\<\-\>                                   { ptbuechi_setcol(); return _iff_; }
!=                                       { ptbuechi_setcol(); return _notequal_; }
\<\>                                     { ptbuechi_setcol(); return _notequal_; }
\-\>                                     { ptbuechi_setcol(); return _implies_; }
=                                        { ptbuechi_setcol(); return _equals_; }
\+                                       { ptbuechi_setcol(); return _plus_; }
\-                                       { ptbuechi_setcol(); return _minus_; }
\*                                       { ptbuechi_setcol(); return _times_; }
\(                                       { ptbuechi_setcol(); return _leftparenthesis_; }
\)                                       { ptbuechi_setcol(); return _rightparenthesis_; }
[>]                                      { ptbuechi_setcol(); return _greaterthan_; }
[<]                                      { ptbuechi_setcol(); return _lessthan_; }
[#]                                      { ptbuechi_setcol(); return _notequal_; }
[>]=                                     { ptbuechi_setcol(); return _greaterorequal_; }
[<]=                                     { ptbuechi_setcol(); return _lessorequal_; }


[\n\r]                                   { ptbuechi_colno = 1; /* whitespace */ }
[\t ]                                    { ptbuechi_setcol();  /* whitespace */ }

[0-9]+                                   { ptbuechi_setcol(); ptbuechi_lval.yt_integer = kc::mkinteger(atoi(ptbuechi_text)); return NUMBER; }

"{"[^\n\r]*"}"                           { ptbuechi_setcol(); /* comments */ }


[^,;:()\t \n\r\{\}]+                     { ptbuechi_setcol(); ptbuechi_lval.yt_casestring = kc::mkcasestring(ptbuechi_text); return IDENTIFIER; }

.                                        { ptbuechi_setcol(); ptbuechi_error("lexical error"); }

%%

inline void ptbuechi_setcol()
{
    ptbuechi_colno += ptbuechi_leng;
}
