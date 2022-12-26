%option noyywrap

%top {
    #include "query.h"
    #include "parser.h"
    #include <vector>
}

INTEGER [0-9]+
IDENTIFIER [a-zA-Z][a-zA-Z0-9]*
REAL -?[0-9]+\.[0-9]+
STRING \"[a-zA-Z0-9]*\"

%%

"+"       { return PLUS; }
"-"       { return MINUS; }
"*"       { return TIMES; }
"/"       { return DIVIDE; }
"("       { return LPAREN; }
")"       { return RPAREN; }
"="       { return ASSIGN; }

";"       { return SEMICOLON; }
","       { return COMMA; }

(?i:CREATE)  { return CREATE; }
(?i:TABLE)   { return TABLE; }
(?i:INSERT)  { return INSERT; }
(?i:INTO)    { return INTO; }
(?i:SELECT)  { return SELECT; }
(?i:DELETE)  { return DELETE; }
(?i:UPDATE)  { return UPDATE; }
(?i:FROM)    { return FROM; }
(?i:WHERE)   { return WHERE; }
(?i:SET)     { return SET; }
(?i:TRUE)    { return TRUE; }
(?i:FALSE)   { return FALSE; }
(?i:OR)      { return OR; }
(?i:AND)     { return AND; }
(?i:NOT)     { return NOT; }
(?i:CHAR)    { return CHAR; }
(?i:VARCHAR) { return VARCHAR; }
(?i:INT)     { return INT; }
(?i:REAL)    { return IREAL; }
(?i:TEXT)    { return TEXT; }
(?i:VALUES)  { return VALUES; }
"=="         { return EQ; }
"!="         { return NEQ; }

{INTEGER} {
    yylval.number = atoi(yytext);
    return NUMBER;
}

{IDENTIFIER} {
    yylval.name = new std::string(yytext);
    return IDENTIFIER;
}

{REAL} {
    yylval.fnumber = atof(yytext);
    return REAL;
}

{STRING} {
    yylval.svalue = new std::string(yytext);
    return STRING;
}
