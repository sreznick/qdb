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

%token PLUS MINUS TIMES LPAREN RPAREN DIVIDE
%token NOT
%token CHAR
%token VARCHAR
%token INT
%token IREAL
%token TEXT
%token BOOLEAN
%token CREATE
%token TABLE
%token INSERT
%token INTO
%token SELECT
%token FROM
%token VALUES
%token DELETE
%token UPDATE
%token SET
%token WHERE
%token EQ
%token NEQ
%token ASSIGN
%token OR
%token AND
%token SEMICOLON
%token COMMA
%token TRUE
%token FALSE

%left PLUS MINUS
%left TIMES

%union {
    query::Query* query;
    int type;
    std::vector<query::FieldDefinition*>* definitions_fields;
    
    query::VariantType* value;
    std::vector<query::VariantType*>* values_fields;

    std::pair<query::Exp*, std::string>* setup_single_field;
    std::vector<std::pair<query::Exp*, std::string>*>* setup_fields;
    
    query::Exp* Ex;
    query::Exp* lEx;
    std::vector<query::Exp*>* Exs;

    std::string *name;
    int number;

    float fnumber;
    std::string *svalue;
}

%token <name> IDENTIFIER
%token <number> NUMBER
%token <svalue> STRING
%token <fnumber> REAL
%nterm <query> start
%nterm <type> type
%nterm <value> value
%nterm <values_fields> values_fields
%nterm <lEx> lEx
%nterm <setup_single_field> setup_single_field
%nterm <setup_fields> setup_fields
%nterm <definitions_fields> definitions_fields
%nterm <Ex> Ex
%nterm <Exs> Exs
%%

start: 
    CREATE TABLE IDENTIFIER LPAREN definitions_fields RPAREN SEMICOLON { 
        query::CreateTable* createTable = new query::CreateTable(*$3, $5);
        query::Query* query = new query::Query(createTable);
        $$ = query;
        *ret = query;
    } |
    INSERT INTO IDENTIFIER VALUES LPAREN values_fields RPAREN SEMICOLON {
        query::Insert* insert = new query::Insert(*$3, $6);
        query::Query* query = new query::Query(insert);
        $$ = query;
        *ret = query;
    } |
    SELECT TIMES FROM IDENTIFIER SEMICOLON {
        query::Select* insert = new query::Select(*$4);
        query::Query* query = new query::Query(insert);
        $$ = query;
        *ret = query;
    } |
    SELECT TIMES FROM IDENTIFIER WHERE lEx SEMICOLON {
        query::Select* insert = new query::Select(*$4, $6);
        query::Query* query = new query::Query(insert);
        $$ = query;
        *ret = query;
    } |
    SELECT Exs FROM IDENTIFIER WHERE lEx SEMICOLON {
        query::Select* insert = new query::Select(*$4, $6, $2);
        query::Query* query = new query::Query(insert);
        $$ = query;
        *ret = query;
    } |
    DELETE FROM IDENTIFIER SEMICOLON {
        query::Delete* deleteQuery = new query::Delete(*$3);
        query::Query* query = new query::Query(deleteQuery);
        $$ = query;
        *ret = query;
    } |
    DELETE FROM IDENTIFIER WHERE lEx SEMICOLON {
        query::Delete* deleteQuery = new query::Delete(*$3, $5);
        query::Query* query = new query::Query(deleteQuery);
        $$ = query;
        *ret = query;
    } |
    UPDATE IDENTIFIER SET setup_fields WHERE lEx SEMICOLON {
        query::Update* update = new query::Update(*$2, $6, $4);
        query::Query* query = new query::Query(update);
        $$ = query;
        *ret = query;
    }
;

definitions_fields :
    IDENTIFIER type {
        $$ = new std::vector<query::FieldDefinition*>{};
        $$->push_back(new query::FieldDefinition(*$1, $2));
    } |
    IDENTIFIER type COMMA definitions_fields {
        $$ = $4;
        $$->push_back(new query::FieldDefinition(*$1, $2));
    }
;

setup_fields: 
    setup_single_field {
        $$ = new std::vector<std::pair<query::Exp*, std::string>*>{};
        $$->push_back($1);
    } | 
    setup_single_field COMMA setup_fields {
        $$ = $3;
        $$->push_back($1);
    }

value: NUMBER { $$ = new query::VariantType($1); }
        | REAL { $$ = new query::VariantType($1); }
        | STRING { $$ = new query::VariantType($1); }

type : VARCHAR LPAREN NUMBER RPAREN {
            $$ = 6;
        } | CHAR LPAREN NUMBER RPAREN {
            $$ = 5;
        } |
        INT {
            $$ = 1;
        } |
        IREAL {
            $$ = 2;
        } |
        TEXT {
            $$ = 3;
        } |
        BOOLEAN {
            $$ = 4;
        }
;

values_fields: value {
        $$ = new std::vector<query::VariantType*>{};
        $$->push_back($1);
    } |
    value COMMA values_fields {
        $$ = $3;
        $$->push_back($1);
    }

setup_single_field: IDENTIFIER ASSIGN Ex {
    $$ = new std::pair($3, *$1);
}

Exs: Ex {
        $$ = new std::vector<query::Exp*>{};
        $$->push_back($1);
    } |
    Ex COMMA Exs {
        $$ = $3;
        $$->push_back($1);
    }


lEx: NOT lEx {
            $$ = new query::Exp(-1, 8);
            $$->llogEx = $2;
        } | TRUE { 
            $$ = new query::Exp(-1, 0);
        } | FALSE { 
            $$ = new query::Exp(-1, 1); 
        } | LPAREN lEx RPAREN {
            $$ = new query::Exp(-1, 3);
            $$->llogEx = $2;
        } | lEx OR lEx {
            $$ = new query::Exp(-1, 7);
            $$->llogEx = $1;
            $$->rlogEx = $3;
        } | lEx AND lEx {
            $$ = new query::Exp(-1, 6);
            $$->llogEx = $1;
            $$->rlogEx = $3;
        } | Ex EQ Ex { 
            $$ = new query::Exp(-1, 4);
            $$->leftEx = $1;
            $$->rightEx = $3;
        } | Ex NEQ Ex {
            $$ = new query::Exp(-1, 5);
            $$->leftEx = $1;
            $$->rightEx = $3;
        }

Ex: 
        Ex TIMES Ex {
            $$ = new query::Exp(9, -1);
            $$->leftEx = $1;
            $$->rightEx = $3;
        } | Ex PLUS Ex {
            $$ = new query::Exp(7, -1);
            $$->leftEx = $1;
            $$->rightEx = $3;
        } | Ex DIVIDE Ex{
            $$ = new query::Exp(10, -1);
            $$->leftEx = $1;
            $$->rightEx = $3;
        } | NUMBER {
            $$ = new query::Exp(1, -1);
            $$->ivals = $1;
        } | REAL {
            $$ = new query::Exp(2, -1);
            $$->fvalks = $1;
        } | LPAREN Ex RPAREN {
            $$ = new query::Exp(6, -1);
            $$->leftEx = $2;
        } | Ex MINUS Ex {
            $$ = new query::Exp(8, -1);
            $$->leftEx = $1;
            $$->rightEx = $3;
        } | STRING {
            $$ = new query::Exp(3, -1);
            $$->svals = *$1;
        } | IDENTIFIER {
            $$ = new query::Exp(4, -1);
            $$->svals = *$1;
        } | lEx {
            $$ = new query::Exp(5, -1);
            $$->llogEx = $1;
        }
 