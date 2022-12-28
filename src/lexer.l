%option noyywrap

%top {
    #include <regex>
    #include "query.h"
    #include "parser.h"
    #include "parsing_utils.h"
}

REAL [0-9]+\.[0-9]+
STRING '[^']*'
INTEGER [0-9]+
IDENTIFIER [a-zA-Z][a-zA-Z0-9_]*

DT_BOOLEAN (?i:BOOLEAN)
DT_INT (?i:INT)
DT_CHAR (?i:CHAR\([0-9]+\))
DT_VARCHAR (?i:VARCHAR\([0-9]+\))
DT_TEXT (?i:TEXT)
DT_REAL (?i:REAL)


%%

"+"       { return PLUS; }
"-"       { return MINUS; }
"*"       { return TIMES; }
"/"       { return DIVIDE; }

"("       { return LPAREN; }
")"       { return RPAREN; }

";"       { return SEMICOLON; }
","       { return COMMA; }

(?i:CREATE)  { return KW_CREATE; }
(?i:INSERT)  { return KW_INSERT; }
(?i:INTO)    { return KW_INTO; }
(?i:SELECT)  { return KW_SELECT; }
(?i:SET)     { return KW_SET; }

(?i:WHERE)   { return KW_WHERE; }
(?i:FROM)    { return KW_FROM; }
(?i:TABLE)   { return KW_TABLE; }
(?i:VALUES)  { return KW_VALUES; }

(?i:TRUE)    { return KW_TRUE; }
(?i:FALSE)   { return KW_FALSE; }
(?i:NOT)     { return OP_NOT; }
"="          { return OP_EQ; }
">"          { return OP_GT; }
"<"          { return OP_LS; }
">="         { return OP_GTE; }
"<="         { return OP_LSE; }
"!="         { return OP_NOTEQ; }
(?i:OR)      { return OP_OR; }
(?i:AND)     { return OP_AND; }


{DT_CHAR} {
    yylval.datatype_spec = new datatypes::Datatype(
        "char",
        parsing_utils::extract_int_from_datatype(std::string(yytext))
    );

    return DT_CHAR;
}

{DT_VARCHAR} {
    yylval.datatype_spec = new datatypes::Datatype(
        "varchar",
        parsing_utils::extract_int_from_datatype(std::string(yytext))
    );

    return DT_VARCHAR;
}

{DT_TEXT} {
    yylval.datatype_spec = new datatypes::Datatype("text", -1);

    return DT_TEXT;
}

{DT_INT} {
    yylval.datatype_spec = new datatypes::Datatype("int", -1);

    return DT_CHAR;
}

{DT_BOOLEAN} {
    yylval.datatype_spec = new datatypes::Datatype("boolean", -1);

    return DT_BOOLEAN;
}

{DT_REAL} {
    yylval.datatype_spec = new datatypes::Datatype("real", -1);

    return DT_REAL;
}

{INTEGER} {
    if (std::string(yytext).length() > 32) {
        return YYUNDEF;
    }

    yylval.number = atoi(yytext);

    return INTEGER;
}

{REAL} {
    if (std::string(yytext).length() > 32) {
        return YYUNDEF;
    }

    yylval.name = new std::string(yytext);

    return REAL;
}

{STRING} {
    yylval.name = new std::string(yytext);

    return STRING;
}

{IDENTIFIER} {
    yylval.name = new std::string(yytext);

    return IDENTIFIER;
}
. ;
