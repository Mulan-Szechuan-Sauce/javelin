%{

    /* Javelin Lexer */
#include "../src/node.hpp"
%}

%option noyywrap

%{
    /* REGEX Variable Initialization */
%}
Newline     [\n]|[\r\n]|[\r]
Integer     [0-9]+
String      \"(\\.|[^\"])*\"|\'(\\.|[^\'])*\'
Id          (([a-zA-Z_])([a-zA-Z0-9_])*)
Operator    ("+"|"-"|"*"|"/"|"%"|"<"|"<="|">="|">"|"=="|"!="|"&"|"|"|"^"|">>"|"<<")
Delimiter   (":"|";"|"."|","|"="|"'"|"\""|"("|")"|"{"|"}"|"["|"]"|"+="|"-="|"*="|"%="|"&="|"|="|"^="|">>="|"<<="|"->")
Keyword     ("and"|"not"|"while"|"elif"|"or"|"else"|"if"|"pass"|"break"|"print"|"class"|"in"|"continue"|"is"|"return"|"def"|"for"|"int"|"float"|"str"|"list"|"dict")
%%

{Newline}     {yylineno++; return EOL;}
{Integer}     {yylval.int_val = new NInteger(atoi(yytext)); return INTEGER;}
{String}      {yylval.str_val = new NString(yytext); return STRING;}
"("           return '(';
")"           return ')';
"["           return '[';
"]"           return ']';
"="           return '=';
"+"           return '+';
"-"           return '-';
"*"           return '*';
"/"           return '/';
"%"           return '%';
":"           return ':';
","           return ',';
"return"      return RETURN;
"and"         return AND;
"or"          return OR;
"not"         return NOT;
"pass"        return PASS;
"break"       return BREAK;
"continue"    return CONTINUE;
"<"           return LT;
"<="          return LTE;
">"           return GT;
">="          return GTE;
"=="          return EQ;
"!="          return NEQ;
"<<"          return SL;
">>"          return SR;
"|"           return BO;
"&"           return BA;
"~"           return BN;
"^"           return BX;
"while"       return WHILE;
"for"         return FOR;
"in"          return IN;
"if"          return IF;
"else"        return ELSE;
"elif"        return ELIF;
"def"         return DEF;
"->"          return RTYPE;
"{"           return '{';
"}"           return '}';
{Id}          {yylval.id_val = new NIdentifier(std::string(yytext)); return ID;}

%{
//{Keyword} Turn these into IDs for now
//{Operator} Let the above code handle delims & operators
//{Delimiter}
    /* Extra whitespace is ignored */
%}
"\t"|" "

. {
    fprintf(stderr, "Invalid token: '%s' on line %d\n", yytext, yylineno + 1);
    yyterminate();
}

