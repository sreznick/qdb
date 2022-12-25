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
(?i:INT)     { return DT_INT; }
(?i:REAL)    { return DT_REAL; }
(?i:TEXT)    { return DT_TEXT; }
(?i:VALUES)  { return KW_VALUES; }
(?i:OR)      { return KW_OR; }
(?i:AND)     { return KW_AND; }
(?i:NOT)     { return KW_NOT; }
(?i:WHERE)   { return KW_WHERE; }
(?i:SET)     { return KW_SET; }
(?i:TRUE)    { return TRUE; }
(?i:FALSE)   { return FALSE; }
"=="         { return LOG_EQUAL; }
"!="         { return LOG_NOEQUAL; }

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
