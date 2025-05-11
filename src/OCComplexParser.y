%{
    /*!
     @header OCComplexFromCString
     @discussion
     Parses complex double from calculator string input
     
     */
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
    
    
    #include <math.h>
    #include <stdio.h>
    #include "OCLibrary.h"

    static double complex result;
    int ocpclex(void);
    
    struct __complexNode {
        int nodeType;
        ComplexNodeRef left;
        ComplexNodeRef right;
    } __complexNode;
    
    struct __complexNodeFunction {
        int nodeType;
        ComplexNodeRef left;
        builtInMathFunctions funcType;
    } __complexNodeFunction;
    
    struct __complexNumberValue {
        int nodeType;
        double complex number;
    } __complexNumberValue;
    
    ComplexNodeRef ComplexNodeCreateInnerNode(int nodeType, ComplexNodeRef left, ComplexNodeRef right);
    ComplexNodeRef ComplexNodeCreateNumberLeaf(double complex number);
    ComplexNodeRef ComplexNodeCreateFunction(int funcType, ComplexNodeRef left);
    ComplexNodeRef ComplexNodeCreateImaginaryUnitLeaf(double complex number);
    double complex ComplexNodeEvaluate(ComplexNodeRef tree);
    void ComplexNodeFree(ComplexNodeRef node);
    void ocpcerror(char *s, ...);

%}

%name-prefix="ocpc"

%union {
    ComplexNodeRef a;
    double complex d;
    int fn;
}

/* declare tokens */
%token <d> NUMBER
%token <fn> FUNC

/* highest precedence first, lowest1+ precedence last */
%left '+' '-'
%left '*' '/' '^'
%nonassoc '|' UMINUS

%type <a> exp explist

%start calclist

%%
calclist:   /* do nothing */
| calclist exp {
    result = ComplexNodeEvaluate($2);
    ComplexNodeFree($2);
}
;

exp: exp '+' exp          { $$ = ComplexNodeCreateInnerNode('+', $1,$3);}
| exp '-' exp          { $$ = ComplexNodeCreateInnerNode('-', $1,$3);}
| exp '*' exp          { $$ = ComplexNodeCreateInnerNode('*', $1,$3); }
| exp '/' exp          { $$ = ComplexNodeCreateInnerNode('/', $1,$3); }
| exp '^' exp          { $$ = ComplexNodeCreateInnerNode('^', $1,$3); }
| '|' exp              { $$ = ComplexNodeCreateInnerNode('|', $2, NULL); }
| '(' exp ')'          { $$ = $2; }
| '-' exp %prec UMINUS { $$ = ComplexNodeCreateInnerNode('M', $2, NULL); }
| NUMBER               { $$ = ComplexNodeCreateNumberLeaf($1); }
| FUNC '(' explist ')' { $$ = ComplexNodeCreateFunction($1, $3); }
;

explist: exp
| exp ',' explist   {$$ = ComplexNodeCreateInnerNode('L',$1,$3);}
;

%%

extern int ocpc_scan_string(const char *);
extern void ocpclex_destroy(void);
bool syntax_error;


double complex OCComplexFromCString(const char *string)
{
    syntax_error = false;
    ocpc_scan_string(string);
    ocpcparse();
    ocpclex_destroy();
    if(syntax_error) return nan("");
    return result;
}

ComplexNodeRef ComplexNodeCreateInnerNode(int nodeType, ComplexNodeRef left, ComplexNodeRef right)
{
    struct __complexNode *node = malloc(sizeof(struct __complexNode));
    node->nodeType = nodeType;
    node->left = left;
    node->right = right;
    return node;
}

ComplexNodeRef ComplexNodeCreateFunction(int funcType, ComplexNodeRef left)
{
    struct __complexNodeFunction *node = malloc(sizeof(struct __complexNodeFunction));
    node->nodeType = 'F';
    node->left = left;
    node->funcType = funcType;
    return (ComplexNodeRef) node;
}

ComplexNodeRef ComplexNodeCreateNumberLeaf(double complex number)
{
    struct __complexNumberValue *leaf = malloc(sizeof(struct __complexNumberValue));
    leaf->nodeType = 'K';
    leaf->number = number;
    return (ComplexNodeRef) leaf;
}

static double complex builtInMathFunction(ComplexNumberFunctionRef func)
{
    builtInMathFunctions funcType = func->funcType;
    double complex value = ComplexNodeEvaluate(func->left);
    
    switch(funcType) {
        case BM_sqrt:
        return csqrt(value);
        case BM_cbrt:
        return ccbrt(value);
        case BM_qtrt:
        return cqtrt(value);
        case BM_exp:
        return cexp(value);
        case BM_log:
        return clog(value);
        case BM_acos:
        return cacos(value);
        case BM_asin:
        return casin(value);
        case BM_cos:
        return ccos(value);
        case BM_sin:
        return csin(value);
        case BM_conj:
        return conj(value);
        case BM_creal:
        return creal(value);
        case BM_cimag:
        return cimag(value);
        case BM_carg:
        return cargument(value);
        default:
        return nan("");
    }
}

double complex ComplexNodeEvaluate(ComplexNodeRef node)
{
    switch(node->nodeType) {
        case 'K': {
            ComplexNumberRef leaf = (ComplexNumberRef) node;
            return leaf->number;
        }
        case '+':
        return ComplexNodeEvaluate(node->left) + ComplexNodeEvaluate(node->right);
        case '-':
        return ComplexNodeEvaluate(node->left) - ComplexNodeEvaluate(node->right);
        case '*':
        return ComplexNodeEvaluate(node->left) * ComplexNodeEvaluate(node->right);
        case '/':
        return ComplexNodeEvaluate(node->left) / ComplexNodeEvaluate(node->right);
        case '^':
        return cpow(ComplexNodeEvaluate(node->left),ComplexNodeEvaluate(node->right));
        case '|':
        return cabs(ComplexNodeEvaluate(node->left));
        case 'M':
        return -ComplexNodeEvaluate(node->left);
        case 'F':
        return builtInMathFunction((ComplexNumberFunctionRef) node);
        default:
        fprintf(stderr,"bad node\n");
    }
    return 0;
}

void ComplexNodeFree(ComplexNodeRef node)
{
    if(node==NULL) return;
    
    switch(node->nodeType) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        ComplexNodeFree(node->right);
        case '|':
        case 'M':
        case 'F':
        ComplexNodeFree(node->left);
        case 'K':
        free((void *) node);
    }
}

void ocpcerror(char *s, ...)
{
    syntax_error = true;
}
