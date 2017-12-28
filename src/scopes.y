%{
#include <string>
#include <stdio.h>
    void yyerror(char const *);
    extern int yylex(void);

    // Track indent level
    int current_indent = 0;
    // Set to ' ' or '\t'
    char indent_type;
    // Set to some integer, on the first indent.
    int indent_multiplicity;

    // Handles arbitrary indents - multiple homogeneous whitespaces are OK
    void handle_indent(char type, int multiplicity);
%}

%error-verbose

%union {
    int int_val;
    std::string *str_val;
}

%token EOL
%token ' '
%token '\t'
%token <str_val> TOK

%type <int_val> spaceindent;
%type <int_val> tabindent;

/* 
 *Grammar rules and actions
 */
%%

input:
| input EOL
| input line EOL {
    if (current_indent > 0) {
        while (current_indent > 0) {
            current_indent--;
            printf("\n0:}");
        }
    }
    printf("\n");
}
| input line EOL spaceindent {
    handle_indent(' ', $4);
}
| input line EOL tabindent {
    handle_indent('\t', $4);
}
;

spaceindent:
  ' ' { $$ = 1; }
| ' ' spaceindent { $$ = 1 + $2; }
;
tabindent:
  '\t' { $$ = 1; }
| '\t' tabindent { $$ = 1 + $2; }
;

line: TOK {
  extern int yylineno;
  printf("%d:%s ", yylineno, $1->c_str());
}
| line TOK {
  printf("%s ", $2->c_str());
}
// Ignore spaces & tabs inside that aren't indentation
| line ' '
| line '\t'
;

%%

#include "../obj/scopes.yy.c"
int main() {
  yyin = stdin;
  yyparse();

  if (current_indent > 0) {
      // Zero out the indent count to clean up closing brackets
      handle_indent(indent_type, 0);
  }
  return 0;
}

void yyerror(char const *m) {
  fprintf(stderr, "Scope parse error on line %d: %s\n", yylineno, m);
  exit(1);
}

void handle_indent(char type, int multiplicity) {
    if (current_indent == 0) {
        // Root statement, but it's about to indent - plop a brace in
        current_indent = 1;
        indent_multiplicity = multiplicity;
        indent_type = type;
        printf("{");
    } else if (indent_type != type) {
        yyerror("Inconsistent indentation");
    } else if (multiplicity != current_indent * indent_multiplicity) {
        if (multiplicity % indent_multiplicity != 0) {
            yyerror("Inconsistent indentation");
        }

        int new_indent = multiplicity / indent_multiplicity;
        int delta = new_indent - current_indent;

        // Don't allow multiple indents at a time!
        if (delta > 1) {
            yyerror("Inconsistent indentation");
        }

        for (; delta < 0; delta++) {
            printf("\n0:}");
        }
        for (; delta > 0; delta--) {
            printf("{");
        }

        current_indent = new_indent;
    }
    printf("\n");
}

