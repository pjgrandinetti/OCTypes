%option noyywrap nodefault
%option prefix = "ocpc"
%{
    typedef const struct __complexNode * ComplexNodeRef;
    typedef const struct __complexNumberValue * ComplexNumberRef;
    typedef const struct __complexNodeFunction * ComplexNumberFunctionRef;
    typedef enum builtInMathFunctions {
        BM_sqrt = 1,
        BM_cbrt,
        BM_qtrt,
        BM_exp,
        BM_log,
        BM_acos,
        BM_asin,
        BM_cos,
        BM_sin,
        BM_conj,
        BM_creal,
        BM_cimag,
        BM_carg
    } builtInMathFunctions;
    
    #include <stdio.h>
    #include <stdarg.h>
    #include <stdlib.h>
    #include <complex.h>
    #include <stdbool.h>
    #ifdef _WIN32
    #include <io.h>
    #else
    #include <unistd.h>
    #endif
    
    #include "math.h"
    #include "OCComplexParser.tab.h"
%}
/* exponent */
EXP ([Ee][-+]?[0-9]+)
REALNUMBER      [0-9]+"."[0-9]*{EXP}?|"."?[0-9]+{EXP}?
%%
{REALNUMBER}    {ocpclval.d = atof(yytext); return NUMBER;}
"I"             {ocpclval.d = I; return NUMBER;}
"pi"            {ocpclval.d = (4*atan(1)); return NUMBER;}
"sqrt"          {ocpclval.fn = BM_sqrt; return FUNC;}
"cbrt"          {ocpclval.fn = BM_cbrt; return FUNC;}
"qtrt"          {ocpclval.fn = BM_qtrt; return FUNC;}
"exp"           {ocpclval.fn = BM_exp; return FUNC;}
"log"           {ocpclval.fn = BM_log; return FUNC;}
"acos"          {ocpclval.fn = BM_acos; return FUNC;}
"asin"          {ocpclval.fn = BM_asin; return FUNC;}
"cos"           {ocpclval.fn = BM_cos; return FUNC;}
"sin"           {ocpclval.fn = BM_sin; return FUNC;}
"conj"          {ocpclval.fn = BM_conj; return FUNC;}
"creal"         {ocpclval.fn = BM_creal; return FUNC;}
"cimag"         {ocpclval.fn = BM_cimag; return FUNC;}
"carg"          {ocpclval.fn = BM_carg; return FUNC;}
[ \t\n]         {/* do nothing */}
.               {return(yytext[0]);}
%%

