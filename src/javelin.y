%{
#include <string>
#include <stdexcept>
#include <stdio.h>
#include "../src/type.hpp"
#include "../src/node.hpp"
#include "../src/scope.hpp"
    std::vector<NStatement*> rootStmts; // Main program blocks
    std::vector<NFunctionDeclStatement*> rootFuncStmts;
    std::vector<NAssignment*> rootAssignStmts;
    void yyerror(char const *);
    extern int yylex(void);
    FunctionScope *beginFunctionScope;
    FunctionStack *lastFuncStack;
%}

%error-verbose

%union {
  NBlock *block_p;
  NStatement *stmt;
  NFunctionDeclStatement *funcDeclStmt;
  NForStatement *for_stmt;
  NElifStatement *elif_stmt;
  NElseStatement *else_stmt;
  NAssignment *assign;
  NExpression *expr;
  NInteger *int_val;
  NString *str_val;
  NIdentifier *id_val;
  NArgs *args;
  NExpressionArgs *exprags;
  Type *type;
  std::string *str;
}

%token EOL
%token '=' '+' '-' '/' '*' '%' ':' ',' '{' '}' '[' ']'
%token FOR IN WHILE IF ELSE ELIF RETURN PASS CONTINUE BREAK
%token <str_val> STRING
%token <int_val> INTEGER
%token <id_val> ID
%token LT NOT LTE GT GTE EQ NEQ AND OR SL SR BA BO BN BX
%token DEF RTYPE

%type <type> rtype
%type <stmt> stmt assign block while if
%type <for_stmt> for for_header
%type <elif_stmt> elif
%type <else_stmt> else
%type <block_p> block_p
%type <expr> expr function_call list_expr
%type <stmt> def ret
%type <funcDeclStmt> funcDef
%type <args> args arg_list
%type <type> type // ROFLCOPTERLMFGSDAO
%type <exprags> args2 arg_list2

// For future reference regarding precedence:
// https://docs.python.org/3.6/reference/expressions.html
// http://stackoverflow.com/questions/16679272/

%left AND
%left OR
%left NOT
%left LT LTE GT GTE EQ NEQ
%precedence '('

%left '+' '-'
%left '*' '/' '%'

/*
 *Grammar rules and actions
 */
%%
program: /* empty */
   | program stmt { $2->addToRootStmts(); }
;

stmt: assign EOL { $$ = $1; }
    | while { $$ = $1; }
    | for { $$ = $1; }
    | if { $$ = $1; }
    | def { $$ = $1; }
    | ret { $$ = $1; }
    | function_call EOL { $$ = new NExpressionStatement($1); }
    | PASS EOL { $$ = new NPassStatement(); }
    | BREAK EOL { $$ = new NBreakStatement(); }
    | CONTINUE EOL { $$ = new NContinueStatement(); }
;

block: '{' EOL block_p_start block_p '}' EOL {
  $$ = $4;

  // Note: Don't deallocate the scope yet, just in case a block latched on
  currentScope = currentScope->next;

  // Pop off the function stack, as appropriate
  int depth = currentScope->depth();
  if (funcStack && funcStack->level >= depth) {
    // Keep this fellow around for type implications
    lastFuncStack = funcStack;
    funcStack = lastFuncStack->parent;
  }
}
;

// This exists soley to push a new scope, it doesn't match anything
block_p_start: /* empty */ {
  if (beginFunctionScope != NULL) {
    currentScope = beginFunctionScope;
    beginFunctionScope = NULL;
  } else {
    currentScope = new Scope(currentScope);
  }
};

block_p: stmt block_p { $$ = new NBlock($1, $2); }
    | stmt { $$ = new NBlock($1, NULL); }
;

expr: function_call { $$ = $1; }
    | expr '[' expr ']' { $$ = new NListIndex($1, $3); }
    | '(' expr ')' { $$ = $2; }
    | INTEGER { $$ = $1; }
    | ID { $$ = $1; }
    | STRING { $$ = $1; }
    | list_expr { $$ = $1; }
    | expr LT  expr { $$ = new NBinaryOperator($1, N_LT, $3); }
    | expr LTE expr { $$ = new NBinaryOperator($1, N_LTE, $3); }
    | expr GT  expr { $$ = new NBinaryOperator($1, N_GT, $3); }
    | expr GTE expr { $$ = new NBinaryOperator($1, N_GTE, $3); }
    | expr NEQ expr { $$ = new NBinaryOperator($1, N_NEQ, $3); }
    | expr EQ  expr { $$ = new NBinaryOperator($1, N_EQ, $3); }
    | expr AND expr { $$ = new NBinaryOperator($1, N_AND, $3); }
    | expr OR  expr { $$ = new NBinaryOperator($1, N_OR, $3); }
    | expr SL  expr { $$ = new NBinaryOperator($1, N_SL, $3); }
    | expr SR  expr { $$ = new NBinaryOperator($1, N_SR, $3); }
    | expr BA  expr { $$ = new NBinaryOperator($1, N_BA, $3); }
    | expr BO  expr { $$ = new NBinaryOperator($1, N_BO, $3); }
    | expr BX  expr { $$ = new NBinaryOperator($1, N_BX, $3); }
    | NOT expr      { $$ = new NUnaryOperator(N_NOT, $2); }
    | BN expr       { $$ = new NUnaryOperator(N_BN, $2); }
    | '-' expr      { $$ = new NUnaryOperator(N_SUB, $2); }
    | expr '+' expr { $$ = new NBinaryOperator($1, N_ADD, $3); }
    | expr '-' expr { $$ = new NBinaryOperator($1, N_SUB, $3); }
    | expr '*' expr { $$ = new NBinaryOperator($1, N_MUL, $3); }
    | expr '/' expr { $$ = new NBinaryOperator($1, N_DIV, $3); }
    | expr '%' expr {
      // Since modulus is different in C++ than in Python
      $$ = new NFunctionCallExpression(new NIdentifier("javelin::modulus"),
          new NExpressionArgs($1, new NExpressionArgs($3, NULL)));
    }
;

list_expr: '[' args2 ']' {
  $$ = new NList($2);
};

ret: RETURN expr EOL { $$ = new NReturn($2); }
   | RETURN EOL { $$ = new NReturn(NULL); };

assign: ID '=' expr { $$ = new NAssignment($1, $3); }
;

while: WHILE expr ':' block { $$ = new NWhileStatement($2, $4); }
;

for: for_header block { $$ = $1; $$->stmt = $2; } ;
for_header: FOR ID IN expr ':' { $$ = new NForStatement($2, $4, NULL); };

if: IF expr ':' block        { $$ = new NIfStatement($2, $4, NULL, NULL); }
    | IF expr ':' block elif { $$ = new NIfStatement($2, $4, $5, NULL); }
    | IF expr ':' block else { $$ = new NIfStatement($2, $4, NULL, $5); }
;

elif: ELIF expr ':' block      { $$ = new NElifStatement($2, $4, NULL, NULL); }
    | ELIF expr ':' block elif { $$ = new NElifStatement($2, $4, $5, NULL); }
    | ELIF expr ':' block else { $$ = new NElifStatement($2, $4, NULL, $5); }
;

else: ELSE ':' block { $$ = new NElseStatement($3); }
;

def: DEF funcDef block {
  $2->stmt = $3;
  // In case the type has been implied
  Type *type = lastFuncStack->get_type();
  $2->type = type->isUnset() ? new VoidType() : type;
  $$ = $2;
};
// So we can define a function before evaluating the content (allow recursion)
funcDef: ID '(' args ')' rtype ':' {
    $$ = new NFunctionDeclStatement($1, $3, $5, NULL);
    funcStack->stmt = $$;
    beginFunctionScope = new FunctionScope(currentScope, $$);
};
args: /* empty */ { $$ = NULL; }
    | arg_list { $$ = $1; }
;
arg_list:
      ID ':' type { $$ = new NArgs($1, $3, NULL); }
    | ID ':' type ',' arg_list { $$ = new NArgs($1, $3, $5); }
    | ID { $$ = new NArgs($1, new UnsetType(), NULL); }
    | ID ',' arg_list { $$ = new NArgs($1, new UnsetType(), $3); }
;

// Pushes the function type to the stack here, so we know what returns should be
rtype: /* empty */ {
  $$ = new UnsetType();
  funcStack = new FunctionStack(currentScope->depth(), funcStack, $$);
} | RTYPE type {
  $$ = $2;
  funcStack = new FunctionStack(currentScope->depth(), funcStack, $$);
};

args2: /* empty */ { $$ = NULL; }
    | arg_list2 { $$ = $1; }
;
arg_list2: expr { $$ = new NExpressionArgs($1, NULL); }
    | expr ',' arg_list2 { $$ = new NExpressionArgs($1, $3); }
;

function_call: ID '(' args2 ')' { $$ = new NFunctionCallExpression($1, $3); }
;

// In the future, this should check the symbol table
type: ID {if ($1->name == "str") $$ =  new StringType();
          else $$ = new BasicType($1->name);}
%%

#include "../obj/javelin.yy.c"
int main() {
  try {
    currentScope = new RootScope();
    funcStack = NULL;

    yyin = stdin;
    yyparse();

    // One header to rule them all
    cout << "#include \"inc/javelin.h\"\n";

    // Generate the headers first, so we don't run into annoying mutuality conflicts
    for (NFunctionDeclStatement *stmt : rootFuncStmts) {
      stmt->generateHeader();
    }

    cout << "int main() {\n";
    for (NStatement *stmt : rootStmts) {
      stmt->generate(1);
    }
    cout << "}\n";
    for (NFunctionDeclStatement *stmt : rootFuncStmts) {
      stmt->generate(0);
    }

    // All root function & class declarations should be before main().
    return 0;
  } catch (const std::runtime_error& error) {
    yyerror(error.what());
  }
}

void yyerror(char const *m) {
    fprintf(stderr, "Parse error: %s on line %d\n", m, yylineno);
    exit(1);
}
