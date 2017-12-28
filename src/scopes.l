%{
    /* Javelin Preparser */
%}

%option noyywrap

%{
    /* REGEX Variable Initialization */
%}
Newline     [\n]|[\r\n]|[\r]
Comment     [" "|"\t"]*#.*$
BlankLines  ^([" "|"\t"]*(#.*)?{Newline})+
Integer     [0-9]+
String      \"(\\.|[^\"])*\"|\'(\\.|[^\'])*\'
Id          (([a-zA-Z_])([a-zA-Z0-9_])*)
Operator    ("+"|"-"|"*"|"/"|"%"|"<"|"<="|">="|">"|"=="|"!="|"&"|"|"|"^"|">>"|"<<")
Delimiter   (":"|";"|"."|","|"="|"'"|"\""|"("|")"|"{"|"}"|"["|"]"|"+="|"-="|"*="|"%="|"&="|"|="|"^="|">>="|"<<="|"->")

%%

{BlankLines} {
    for (; *yytext != '\0'; yytext++) {
        if (*yytext == '\n') {
            yylineno++;
        }
    }
}

{Newline} {
    yylineno++;
    return EOL;
}
" "       return ' ';
"\t"      return '\t';

{Integer}|{String}|{Operator}|{Delimiter}|{Id} {
    yylval.str_val = new std::string(yytext);
    return TOK;
}

%{
    /* Comments have lower precedece than other tokens */
%}
{Comment}

. {
    fprintf(stderr, "Invalid token: '%s' on line %d\n", yytext, yylineno);
    yyterminate();
}
