//%define api.value.type {int}
%parse-param {query::Query **ret}

%code top {
    #include <iostream>
    #include <vector>
    #include <stdio.h>
    #include "query.h"

    extern int yylex(void);

    static void yyerror(query::Query **ret, const char* s) {
        fprintf(stderr, "%s\n", s);
    }
}

%token PLUS MINUS TIMES LPAREN RPAREN
%token SEMICOLON
%token COMMA
%token KW_CREATE
%token KW_TABLE
%token KW_INSERT
%token KW_INTO
%token KW_SELECT
%token KW_FROM
%token KW_DELETE
%token KW_UPDATE
%token KW_CHAR

%left PLUS MINUS
%left TIMES

%union {
  std::string *name;
  int number;
  query::Query* query;
  query::DataType* type;
  std::vector<query::FieldDefinition*>* fieldDefs;
}

%token <name> IDENTIFIER
%token <number> NUMBER
%nterm <number> expr
%nterm <query> start
%nterm <type> type
%nterm <fieldDefs> field_defs
%%


start: 
    KW_CREATE KW_TABLE IDENTIFIER LPAREN field_defs RPAREN SEMICOLON { 
        query::CreateTable* createTable = new query::CreateTable(*$3, $5);
        query::Query* query = new query::Query(createTable);
        $$ = query;
        *ret = query;
    } |
    KW_INSERT KW_INTO IDENTIFIER SEMICOLON {
        query::Insert* insert = new query::Insert(*$3);
        query::Query* query = new query::Query(insert);
        $$ = query;
        *ret = query;
    } |
    KW_SELECT KW_FROM IDENTIFIER SEMICOLON {
        query::Insert* insert = new query::Insert(*$3);
        query::Query* query = new query::Query(insert);
        $$ = query;
        *ret = query;
    } |
    KW_DELETE KW_FROM IDENTIFIER SEMICOLON {
        query::Delete* deleteQuery = new query::Delete(*$3);
        query::Query* query = new query::Query(deleteQuery);
        $$ = query;
        *ret = query;
    } |
    KW_UPDATE IDENTIFIER SEMICOLON {
        query::Update* update = new query::Update(*$2);
        query::Query* query = new query::Query(update);
        $$ = query;
        *ret = query;
    }
;


field_defs :
    IDENTIFIER type {
        $$ = new std::vector<query::FieldDefinition*>{};
        $$->push_back(new query::FieldDefinition($1, $2));
    } |
    IDENTIFIER type COMMA field_defs {
        $$ = $4;
        $$->push_back(new query::FieldDefinition($1, $2));
    }
;

type : KW_CHAR LPAREN NUMBER RPAREN {
         $$ = new query::DataType(query::CharType{$3});
       }
;


expr
    : expr PLUS expr {
        $$ = $1 + $3;
    }
    | expr MINUS expr {
        $$ = $1 - $3;
    }
    | expr TIMES expr {
        $$ = $1 * $3;
    }
    | LPAREN expr RPAREN {
        $$ = $2;
    }
    | NUMBER {
        $$ = $1;
    }
;
