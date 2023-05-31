%option noyywrap

%top {
    #include "query.h"
    #include "parser.h"
    #include <vector>
}

INTEGER [0-9]+
IDENTIFIER [a-zA-Z_][a-zA-Z0-9_]*
REAL [0-9]+\.[0-9]+
STRING '[-a-zA-Z0-9"_ /.!\*\+\<\>\{\}$#\[\]]*'

%%

"="       { return ASSIGN; }
"+"       { return PLUS; }
"-"       { return MINUS; }
"*"       { return TIMES; }
"/"       { return DIVIDE; }
"("       { return LPAREN; }
")"       { return RPAREN; }

";"       { return SEMICOLON; }
","       { return COMMA; }

(?i:CREATE)  { return KW_CREATE; }
(?i:TABLE)   { return KW_TABLE; }
(?i:INSERT)  { return KW_INSERT; }
(?i:INTO)    { return KW_INTO; }
(?i:SELECT)  { return KW_SELECT; }
(?i:FROM)    { return KW_FROM; }
(?i:DELETE)  { return KW_DELETE; }
(?i:UPDATE)  { return KW_UPDATE; }
(?i:CHAR)    { return KW_CHAR; }
(?i:VARCHAR) { return KW_VARCHAR; }
(?i:INT)     { return KW_INT; }
(?i:REAL)    { return KW_REAL; }
(?i:TEXT)    { return KW_TEXT; }
(?i:BOOLEAN) { return KW_BOOLEAN; }
(?i:TRUE)    { return KW_TRUE; }
(?i:FALSE)   { return KW_FALSE; }
(?i:OR)      { return KW_OR; }
(?i:AND)     { return KW_AND; }
(?i:NOT)     { return KW_NOT; }
(?i:VALUES)  { return KW_VALUES; }
(?i:WHERE)   { return KW_WHERE; }
(?i:SET)     { return KW_SET; }
"=="         { return KW_EQ; }
"!="         { return KW_NEQ; }



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

. ;
