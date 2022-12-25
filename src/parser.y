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
%token KW_NOT
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
%token KW_VARCHAR
%token KW_VALUES
%token KW_OR
%token KW_AND
%token KW_SET
%token KW_WHERE

%token DT_INT
%token DT_REAL
%token DT_TEXT
%token DT_BOOLEAN

%token TRUE
%token FALSE
%token LOG_EQUAL
%token LOG_NOEQUAL
%token ASSIGN

%left PLUS MINUS
%left TIMES

%union {
    std::string *name;
    int number;

    float fnumber;
    std::string *svalue;

    query::Query* query;
    query::DataType* type;
    query::FieldValue* value;
    query::Expression* expression;
    query::LogicalExpression* logical_expression;
    query::FieldSetup* field_setup;

    std::vector<query::FieldDefinition*>* field_defs;
    std::vector<query::FieldValue*>* field_values;
    std::vector<query::Expression*>* expressions;
    std::vector<query::FieldSetup*>* field_setups;

    char* string_field;
    int integer_field;
    float float_field;
}

%token <name> IDENTIFIER
%token <number> NUMBER
%token <svalue> STRING
%token <fnumber> REAL
%nterm <query> start
%nterm <type> type
%nterm <value> value
%nterm <field_defs> field_defs
%nterm <field_values> field_values
%nterm <logical_expression> logical_expression
%nterm <expression> expression
%nterm <expressions> expressions
%nterm <field_setup> field_setup
%nterm <field_setups> field_setups
%%


start: 
    KW_CREATE KW_TABLE IDENTIFIER LPAREN field_defs RPAREN SEMICOLON { 
        query::CreateTable* createTable = new query::CreateTable(*$3, $5);
        query::Query* query = new query::Query(createTable);
        $$ = query;
        *ret = query;
    } |
    KW_INSERT KW_INTO IDENTIFIER KW_VALUES LPAREN field_values RPAREN SEMICOLON {
        query::Insert* insert = new query::Insert(*$3, $6);
        query::Query* query = new query::Query(insert);
        $$ = query;
        *ret = query;
    } |
    KW_SELECT TIMES KW_FROM IDENTIFIER SEMICOLON {
        query::Select* insert = new query::Select(*$4);
        query::Query* query = new query::Query(insert);
        $$ = query;
        *ret = query;
    } |
    KW_SELECT TIMES KW_FROM IDENTIFIER KW_WHERE logical_expression SEMICOLON {
        query::Select* insert = new query::Select(*$4, $6);
        query::Query* query = new query::Query(insert);
        $$ = query;
        *ret = query;
    } |
    KW_SELECT expressions KW_FROM IDENTIFIER KW_WHERE logical_expression SEMICOLON {
        query::Select* insert = new query::Select(*$4, $6, $2);
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
    KW_DELETE KW_FROM IDENTIFIER KW_WHERE logical_expression SEMICOLON {
        query::Delete* deleteQuery = new query::Delete(*$3, $5);
        query::Query* query = new query::Query(deleteQuery);
        $$ = query;
        *ret = query;
    } |
    KW_UPDATE IDENTIFIER KW_SET field_setups KW_WHERE logical_expression SEMICOLON {
        query::Update* update = new query::Update(*$2, $6, $4);
        query::Query* query = new query::Query(update);
        $$ = query;
        *ret = query;
    }
;


field_defs :
    IDENTIFIER type {
        $$ = new std::vector<query::FieldDefinition*>{};
        $$->push_back(new query::FieldDefinition(*$1, $2));
    } |
    IDENTIFIER type COMMA field_defs {
        $$ = $4;
        $$->push_back(new query::FieldDefinition(*$1, $2));
    }
;


field_values: value {
            $$ = new std::vector<query::FieldValue*>{};
            $$->push_back($1);
        } |
        value COMMA field_values {
            $$ = $3;
            $$->push_back($1);
        }

field_setups: 
    field_setup {
        $$ = new std::vector<query::FieldSetup*>{};
        $$->push_back($1);
    } | 
    field_setup COMMA field_setups {
        $$ = $3;
        $$->push_back($1);
    }

field_setup: IDENTIFIER ASSIGN expression {
    $$ = new query::FieldSetup(*$1, $3);
}

value: NUMBER { $$ = new query::FieldValue($1); }
        | REAL { $$ = new query::FieldValue($1); }
        | STRING { $$ = new query::FieldValue($1); }

type : KW_VARCHAR LPAREN NUMBER RPAREN {
            $$ = new query::DataType(new query::VarCharType{static_cast<unsigned int>($3)});
        } | KW_CHAR LPAREN NUMBER RPAREN {
            $$ = new query::DataType(new query::CharType{static_cast<unsigned int>($3)});
        } |
        DT_INT {
            $$ = new query::DataType(query::SimpleType::INT);
        } |
        DT_REAL {
            $$ = new query::DataType(query::SimpleType::REAL);
        } |
        DT_TEXT {
            $$ = new query::DataType(query::SimpleType::TEXT);
        } |
        DT_BOOLEAN {
            $$ = new query::DataType(query::SimpleType::BOOLEAN);
        }
;

expressions: expression {
        $$ = new std::vector<query::Expression*>{};
        $$->push_back($1);
    } |
    expression COMMA expressions {
        $$ = $3;
        $$->push_back($1);
    }


logical_expression: TRUE { 
            $$ = new query::LogicalExpression(query::LogicalExpressionType::TRUE);
        } | FALSE { 
            $$ = new query::LogicalExpression(query::LogicalExpressionType::FALSE); 
        } | LPAREN logical_expression RPAREN {
            $$ = new query::LogicalExpression(query::LogicalExpressionType::LOG_BRACKETS);
            $$->leftLogicalExpression = $2;
        } | expression LOG_EQUAL expression { 
            $$ = new query::LogicalExpression(query::LogicalExpressionType::EQUALS);
            $$->leftExpression = $1;
            $$->rightExpression = $3;
        } | expression LOG_NOEQUAL expression {
            $$ = new query::LogicalExpression(query::LogicalExpressionType::NEQUALS);
            $$->leftExpression = $1;
            $$->rightExpression = $3;
        } | logical_expression KW_OR logical_expression {
            $$ = new query::LogicalExpression(query::LogicalExpressionType::OR);
            $$->leftLogicalExpression = $1;
            $$->rightLogicalExpression = $3;
        } | logical_expression KW_AND logical_expression {
            $$ = new query::LogicalExpression(query::LogicalExpressionType::AND);
            $$->leftLogicalExpression = $1;
            $$->rightLogicalExpression = $3;
        } | KW_NOT logical_expression {
            $$ = new query::LogicalExpression(query::LogicalExpressionType::NOT);
            $$->leftLogicalExpression = $2;
        }

expression: 
        NUMBER {
            $$ = new query::Expression(query::ExpressionType::INTEGER);
            $$->integerField = $1;
        } | REAL {
            $$ = new query::Expression(query::ExpressionType::EXPR_REAL);
            $$->floatField = $1;
        } | STRING {
            $$ = new query::Expression(query::ExpressionType::STRING);
            $$->stringField = *$1;
        } | IDENTIFIER {
            $$ = new query::Expression(query::ExpressionType::FIELD);
            $$->stringField = *$1;
        } | logical_expression {
            $$ = new query::Expression(query::ExpressionType::LOGICAL_EXPR);
            $$->logicalExpression = $1;
        } | LPAREN expression RPAREN {
            $$ = new query::Expression(query::ExpressionType::BRACKETS);
            $$->leftExpression = $2;
        } | expression PLUS expression {
            $$ = new query::Expression(query::ExpressionType::PLUS);
            $$->leftExpression = $1;
            $$->rightExpression = $3;
        } | expression MINUS expression {
            $$ = new query::Expression(query::ExpressionType::MINUS);
            $$->leftExpression = $1;
            $$->rightExpression = $3;
        } | expression TIMES expression {
            $$ = new query::Expression(query::ExpressionType::MULTIPLY);
            $$->leftExpression = $1;
            $$->rightExpression = $3;
        } | expression DIVIDE expression{
            $$ = new query::Expression(query::ExpressionType::DIVIDE);
            $$->leftExpression = $1;
            $$->rightExpression = $3;
        }
 