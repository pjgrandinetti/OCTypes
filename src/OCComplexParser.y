

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
    result = ComplexNodeEvaluate($2);
    ComplexNodeFree($2);
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
          | ABS exp ABS %prec ABS { fprintf(stderr, "YACC: ABS node (unary_exp)!\n"); $$ = ComplexNodeCreateInnerNode('|', $2, NULL); }
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
    fprintf(stderr,"ocpc_scan_string of %s complete\n",string);
    ocpcparse();
    fprintf(stderr,"ocpcparse of %s complete\n",string);
    ocpclex_destroy();
    if(syntax_error) return nan("");
    return result;
}

ComplexNodeRef ComplexNodeCreateInnerNode(int nodeType, ComplexNodeRef left, ComplexNodeRef right)
{
    fprintf(stderr, "ComplexNodeCreateInnerNode, nodeType = %d\n",nodeType);
    struct __complexNode *node = malloc(sizeof(struct __complexNode));
    node->nodeType = nodeType;
    node->left = left;
    node->right = right;
    return node;
}

ComplexNodeRef ComplexNodeCreateFunction(int funcType, ComplexNodeRef left)
{
    fprintf(stderr, "ComplexNodeCreateFunction, funcType = %d\n",funcType);
    struct __complexNodeFunction *node = malloc(sizeof(struct __complexNodeFunction));
    node->nodeType = 'F';
    node->left = left;
    node->funcType = funcType;
    return (ComplexNodeRef) node;
}

ComplexNodeRef ComplexNodeCreateNumberLeaf(double complex number)
{
    fprintf(stderr, "ComplexNodeCreateNumberLeaf, number = %g+%g*I\n",creal(number),cimag(number));
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
    if (!node) {
        fprintf(stderr, "ComplexNodeEvaluate: NULL node pointer!\n");
        return nan("");
    }

    fprintf(stderr, "Evaluating node type '%c'\n", node->nodeType);

    switch(node->nodeType) {
        case 'K': {
            ComplexNumberRef leaf = (ComplexNumberRef) node;
            fprintf(stderr, "  Leaf value: creal=%.6f, cimag=%.6f\n",
                    creal(leaf->number), cimag(leaf->number));
            return leaf->number;
        }
        case '+': {
            fprintf(stderr, "  Calling ComplexNodeEvaluate(node->left) for '|'\n");
            double complex l = ComplexNodeEvaluate(node->left);
            double complex r = ComplexNodeEvaluate(node->right);
            double complex res = l + r;
            fprintf(stderr, "  '+' result: creal=%.6f, cimag=%.6f\n", creal(res), cimag(res));
            return res;
        }
        case '-': {
            fprintf(stderr, "  Calling ComplexNodeEvaluate(node->left) for '|'\n");
            double complex l = ComplexNodeEvaluate(node->left);
            double complex r = ComplexNodeEvaluate(node->right);
            double complex res = l - r;
            fprintf(stderr, "  '-' result: creal=%.6f, cimag=%.6f\n", creal(res), cimag(res));
            return res;
        }
        case '*': {
            fprintf(stderr, "  Calling ComplexNodeEvaluate(node->left) for '|'\n");
            double complex l = ComplexNodeEvaluate(node->left);
            double complex r = ComplexNodeEvaluate(node->right);
            double complex res = l * r;
            fprintf(stderr, "  '*' result: creal=%.6f, cimag=%.6f\n", creal(res), cimag(res));
            return res;
        }
        case '/': {
            fprintf(stderr, "  Calling ComplexNodeEvaluate(node->left) for '|'\n");
            double complex l = ComplexNodeEvaluate(node->left);
            double complex r = ComplexNodeEvaluate(node->right);
            double complex res = l / r;
            fprintf(stderr, "  '/' result: creal=%.6f, cimag=%.6f\n", creal(res), cimag(res));
            return res;
        }
        case '^': {
            fprintf(stderr, "  Calling ComplexNodeEvaluate(node->left) for '|'\n");
            double complex l = ComplexNodeEvaluate(node->left);
            double complex r = ComplexNodeEvaluate(node->right);
            double complex res = cpow(l, r);
            fprintf(stderr, "  '^' result: creal=%.6f, cimag=%.6f\n", creal(res), cimag(res));
            return res;
        }
        case '|': {
            if (!node->left) {
                fprintf(stderr, "  '|' node has NULL left child!\n");
                return nan("");
            }
            fprintf(stderr, "  Calling ComplexNodeEvaluate(node->left) for '|'\n");
            double complex val = ComplexNodeEvaluate(node->left);
            fprintf(stderr, "  Evaluated inner expression for '|': creal=%.6f, cimag=%.6f\n",
                    creal(val), cimag(val));
            double abs_val = cabs(val);
            fprintf(stderr, "  '|' result: |z| = %.6f\n", abs_val);
            return abs_val + 0.0 * I; // <--- THIS IS ESSENTIAL
        }
        case 'M': {
            fprintf(stderr, "  Calling ComplexNodeEvaluate(node->left) for '|'\n");
            double complex l = ComplexNodeEvaluate(node->left);
            double complex res = -l;
            fprintf(stderr, "  Unary minus result: creal=%.6f, cimag=%.6f\n", creal(res), cimag(res));
            return res;
        }
        case 'F': {
            fprintf(stderr, "  Function call node\n");
            return builtInMathFunction((ComplexNumberFunctionRef) node);
        }
        default:
            fprintf(stderr, "  Unknown node type '%c' — returning NaN\n", node->nodeType);
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
