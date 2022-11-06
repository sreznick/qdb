%option noyywrap

%top {
    #include "query.h"
    #include "parser.h"
    #include <vector>
}

INTEGER [0-9]+
IDENTIFIER [a-zA-Z][a-zA-Z0-9]*

%%

"+"       { return PLUS; }
"-"       { return MINUS; }
"*"       { return TIMES; }
"("       { return LPAREN; }
")"       { return RPAREN; }

";"       { return SEMICOLON; }
","       { return COMMA; }

"CREATE"  { return KW_CREATE; }
"TABLE"   { return KW_TABLE; }
"INSERT"  { return KW_INSERT; }
"INTO"    { return KW_INTO; }
"SELECT"  { return KW_SELECT; }
"FROM"    { return KW_FROM; }
"DELETE"  { return KW_DELETE; }
"UPDATE"  { return KW_UPDATE; }
"CHAR"    { return KW_CHAR; }



{INTEGER} {
    yylval.number = atoi(yytext);
    return NUMBER;
}

{IDENTIFIER} {
    yylval.name = new std::string(yytext);
    return IDENTIFIER;
}

. ;
