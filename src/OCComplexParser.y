

%{
    /*!
     @header OCComplexFromCString
     @discussion
     Parses complex double from calculator string input
     */
    typedef const struct impl_complexNode * ComplexNodeRef;
    typedef const struct impl_complexNumberValue * ComplexNumberRef;
    typedef const struct impl_complexNodeFunction * ComplexNumberFunctionRef;
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

    static ComplexNodeRef ocpc_root = NULL;
    static double complex result;
    int ocpclex(void);

    struct impl_complexNode {
        int nodeType;
        ComplexNodeRef left;
        ComplexNodeRef right;
    } impl_complexNode;

    struct impl_complexNodeFunction {
        int nodeType;
        ComplexNodeRef left;
        builtInMathFunctions funcType;
    } impl_complexNodeFunction;

    struct impl_complexNumberValue {
        int nodeType;
        double complex number;
    } impl_complexNumberValue;

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
%token ABS              // <--- Add this token for absolute value

%type <a> exp add_exp mult_exp unary_exp power_val primary_exp explist

/* Operator Precedence: lowest to highest */
%left '+' '-'
%left '*' '/'
%left IMPLICIT_MULT
%nonassoc UMINUS
%right '^'
%nonassoc ABS           // <--- Only once!

%start calclist
%expect 10
%%
calclist:   /* do nothing */
| calclist exp {
    ocpc_root = $2;
    result    = ComplexNodeEvaluate($2);
}
;

exp     : add_exp ;

add_exp : mult_exp
        | add_exp '+' mult_exp { $$ = ComplexNodeCreateInnerNode('+', $1,$3); }
        | add_exp '-' mult_exp { $$ = ComplexNodeCreateInnerNode('-', $1,$3); }
        ;

mult_exp : unary_exp
         | mult_exp '*' unary_exp { $$ = ComplexNodeCreateInnerNode('*', $1,$3); }
         | mult_exp '/' unary_exp { $$ = ComplexNodeCreateInnerNode('/', $1,$3); }
         | mult_exp power_val %prec IMPLICIT_MULT { $$ = ComplexNodeCreateInnerNode('*', $1,$2); } /* Implicit multiplication with power_val */
         ;

unary_exp : power_val
          | '-' unary_exp %prec UMINUS { $$ = ComplexNodeCreateInnerNode('M', $2, NULL); }
          | ABS exp ABS %prec ABS { $$ = ComplexNodeCreateInnerNode('|', $2, NULL); }
          ;

power_val : primary_exp
          | primary_exp '^' unary_exp { $$ = ComplexNodeCreateInnerNode('^', $1,$3); } /* Base is primary_exp, exponent can be unary_exp (e.g., a^-b) */
          ;

primary_exp : NUMBER               { $$ = ComplexNodeCreateNumberLeaf($1); }
            | FUNC '(' explist ')' { $$ = ComplexNodeCreateFunction($1, $3); }
            | '(' exp ')'          { $$ = $2; }
            ;

explist: exp
| exp ',' explist   {$$ = ComplexNodeCreateInnerNode('L',$1,$3);}
;

%%

extern int ocpc_scan_string(const char *);
extern void ocpclex_destroy(void);
bool syntax_error;

/* ... C code below ... */

double complex OCComplexFromCString(const char *string)
{

    syntax_error = false;
    ocpc_scan_string(string);
    ocpcparse();
    ocpclex_destroy();

    double complex out = nan("");
    if (!syntax_error && ocpc_root) {
        out = result;
    }

    /* whether parse succeeded or not, free the tree once here */
    if (ocpc_root) {
        ComplexNodeFree(ocpc_root);
        ocpc_root = NULL;
    }

    return out;
}

ComplexNodeRef ComplexNodeCreateInnerNode(int nodeType, ComplexNodeRef left, ComplexNodeRef right)
{
    struct impl_complexNode *node = malloc(sizeof(struct impl_complexNode));
    node->nodeType = nodeType;
    node->left = left;
    node->right = right;
    return node;
}


ComplexNodeRef ComplexNodeCreateFunction(int funcType, ComplexNodeRef left)
{
    struct impl_complexNodeFunction *node = malloc(sizeof(struct impl_complexNodeFunction));
    node->nodeType = 'F';
    node->left = left;
    node->funcType = funcType;
    return (ComplexNodeRef) node;
}

ComplexNodeRef ComplexNodeCreateNumberLeaf(double complex number)
{
    struct impl_complexNumberValue *leaf = malloc(sizeof(struct impl_complexNumberValue));
    leaf->nodeType = 'K';
    leaf->number = number;
    return (ComplexNodeRef) leaf;
}

static double complex builtInMathFunction(ComplexNumberFunctionRef func)
{
    builtInMathFunctions funcType = func->funcType;
    double complex value = ComplexNodeEvaluate(func->left);

    double complex result;
    switch(funcType) {
        case BM_sqrt:
            result = csqrt(value);
            break;
        case BM_cbrt:
            result = ccbrt(value);
            break;
        case BM_qtrt:
            result = cqtrt(value);
            break;
        case BM_exp:
            result = cexp(value);
            break;
        case BM_log:
            result = clog(value);
            break;
        case BM_acos:
            result = cacos(value);
            break;
        case BM_asin:
            result = casin(value);
            break;
        case BM_cos:
            result = ccos(value);
            break;
        case BM_sin:
            result = csin(value);
            break;
        case BM_conj:
            result = conj(value);
            break;
        case BM_creal:
            result = creal(value);
            break;
        case BM_cimag:
            result = cimag(value);
            break;
        case BM_carg:
            result = cargument(value);
            break;
        default:
            fprintf(stderr,"builtInMathFunction: Unknown funcType %d\n", funcType);
            return nan("");
    }
    return result;
}


double complex ComplexNodeEvaluate(ComplexNodeRef node)
{
    if (!node) {
        return nan("");
    }


    switch(node->nodeType) {
        case 'K': {
            ComplexNumberRef leaf = (ComplexNumberRef) node;
            return leaf->number;
        }
        case '+': {
            double complex l = ComplexNodeEvaluate(node->left);
            double complex r = ComplexNodeEvaluate(node->right);
            double complex res = l + r;
            return res;
        }
        case '-': {
            double complex l = ComplexNodeEvaluate(node->left);
            double complex r = ComplexNodeEvaluate(node->right);
            double complex res = l - r;
            return res;
        }
        case '*': {
            double complex l = ComplexNodeEvaluate(node->left);
            double complex r = ComplexNodeEvaluate(node->right);
            double complex res = l * r;
            return res;
        }
        case '/': {
            double complex l = ComplexNodeEvaluate(node->left);
            double complex r = ComplexNodeEvaluate(node->right);
            double complex res = l / r;
            return res;
        }
        case '^': {
            double complex l = ComplexNodeEvaluate(node->left);
            double complex r = ComplexNodeEvaluate(node->right);
            double complex res = cpow(l, r);
            return res;
        }
        case '|': {
            if (!node->left) {
                return nan("");
            }
            double complex val = ComplexNodeEvaluate(node->left);
            double abs_val = cabs(val);
            return abs_val + 0.0 * I;
        }
        case 'M': {
            double complex l = ComplexNodeEvaluate(node->left);
            double real = -creal(l);
            double imag = -cimag(l);
            // Fix: ensure +0.0 for real negative numbers
            if (fabs(imag) < 1e-15) imag = 0.0;
            double complex res = real + imag * I;
            return res;
        }
        case 'F': {
            double complex result = builtInMathFunction((ComplexNumberFunctionRef) node);
            return result;
        }
        default:
            return nan("");
    }
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
        case 'L': // Added 'L' to handle explist nodes
        ComplexNodeFree(node->right);
        // Fallthrough
        case '|':
        case 'M':
        case 'F':
        ComplexNodeFree(node->left);
        // Fallthrough
        case 'K':
        free((void *) node);
    }
}

void ocpcerror(char *s, ...)
{
    syntax_error = true;
}
