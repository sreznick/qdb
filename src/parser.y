//%define api.value.type {int}
%parse-param {query::Query **ret}
%error-verbose
%code top {
    #include <iostream>
    #include <stdio.h>
    #include "query.h"

    #define YYDEBUG 1

    extern int yylex(void);

    static void yyerror(query::Query **ret, const char* s) {
        fprintf(stderr, "%s\n", s);
    }
}

%token PLUS MINUS TIMES DIVIDE LPAREN RPAREN
%token SEMICOLON
%token COMMA

%token KW_CREATE
%token KW_INSERT
%token KW_INTO
%token KW_SELECT
%token KW_SET


%token KW_WHERE
%token KW_FROM
%token KW_TABLE
%token KW_VALUES

%token KW_TRUE
%token KW_FALSE
%token OP_NOT
%token OP_EQ
%token OP_NOTEQ
%token OP_OR
%token OP_AND

%token OP_GT
%token OP_GTE
%token OP_LS
%token OP_LSE


%left OP_EQ OP_NOTEQ
%left OR
%left AND
%left PLUS MINUS
%left DIVIDE TIMES


%union {
  std::string* name;
  int number;
  query::Query* query;
  datatypes::Datatype* datatype_spec;
  std::vector<datatypes::Column*>* columns;
  datatypes::Literal* literal;
  std::vector<datatypes::Literal*>* literals;
  datatypes::Expression* expression;
  std::vector<datatypes::Expression*>* expressions_list;
}

%token <name> IDENTIFIER
%token <number> INTEGER
%token <name> REAL
%token <name> STRING
%token <number> NUMBER

%token <datatype_spec> DT_CHAR
%token <datatype_spec> DT_VARCHAR
%token <datatype_spec> DT_INT
%token <datatype_spec> DT_BOOLEAN
%token <datatype_spec> DT_TEXT
%token <datatype_spec> DT_REAL
%token <datatype_spec> DT_LONG

%type <query> create_table
%type <query> insert_into_table;
%type <query> select_from_table;

%type <expression> expr;
%type <expression> logical_expr;
%type <expressions_list> expr_list;

%type <datatype_spec> datatype;
%type <columns> columns;
%type <literal> literal;
%type <literals> insertion_values;

%%

start: create_table
     | insert_into_table
     | select_from_table;

create_table
    : KW_CREATE KW_TABLE IDENTIFIER LPAREN columns RPAREN SEMICOLON {
        query::CreateTable* createTable = new query::CreateTable(*$3, $5);
        query::Query* query = new query::Query(createTable);
        $$ = query;
        *ret = query;
    }

insert_into_table
    : KW_INSERT KW_INTO IDENTIFIER KW_VALUES LPAREN insertion_values RPAREN SEMICOLON {
        query::Insert* insertIntoTable = new query::Insert(*$3, *$6);
        query::Query* query = new query::Query(insertIntoTable);
        $$ = query;
        *ret = query;
    }
;

select_from_table
    : KW_SELECT expr_list KW_FROM IDENTIFIER KW_WHERE logical_expr SEMICOLON {
        query::Select* selectFromTable = new query::Select(*$4, *$2, $6);
        query::Query* query = new query::Query(selectFromTable);
        $$ = query;
        *ret = query;
    }
    | KW_SELECT expr_list KW_FROM IDENTIFIER SEMICOLON {
        query::Select* selectFromTable = new query::Select(*$4, *$2);
        query::Query* query = new query::Query(selectFromTable);
        $$ = query;
        *ret = query;
    }
;

datatype
    : DT_INT {
      $$ = $1;
    }
    | DT_CHAR {
      $$ = $1;
    }
    | DT_VARCHAR {
      $$ = $1;
    }
    | DT_BOOLEAN {
      $$ = $1;
    }
    | DT_TEXT {
      $$ = $1;
    }
    | DT_REAL {
      $$ = $1;
    }
;

columns
    : IDENTIFIER datatype {
        datatypes::Column* column = new datatypes::Column(*$1, $2);
        std::vector<datatypes::Column*>* columns = new std::vector<datatypes::Column*>;
        columns->push_back(column);
        $$ = columns;
    }
    | columns COMMA IDENTIFIER datatype {
        std::vector<datatypes::Column*>* values = $1;
        values->push_back(new datatypes::Column(*$3, $4));
        $$ = values;
    }
;

literal
    : STRING {
      std::string value = *$1;
      value.erase(std::remove(value.begin(), value.end(), '\''), value.end());
      $$ = new datatypes::Literal(value, "TEXT");
    }
    | INTEGER {
      $$ = new datatypes::Literal(std::to_string($1), "INTEGER");
    }
    | REAL {
      $$ = new datatypes::Literal(*$1, "REAL");
    }
    | KW_TRUE {
      $$ = new datatypes::Literal("TRUE", "BOOLEAN");
    }
    | KW_FALSE {
      $$ = new datatypes::Literal("FALSE", "BOOLEAN");
    }
;

insertion_values
    : literal {
        std::vector<datatypes::Literal*>* values = new std::vector<datatypes::Literal*>;
        values->push_back($1);

        $$ = values;
    }
    | insertion_values COMMA literal {
        std::vector<datatypes::Literal*>* values = $1;
        values->push_back($3);

        $$ = values;
    }
;

expr_list
    : TIMES {
        std::vector<datatypes::Expression*>* values = new std::vector<datatypes::Expression*>;
        datatypes::ExpressionValue* expressionValue = new datatypes::ExpressionValue("*", std::string("UNKNOWN"));
        datatypes::Expression* value = new datatypes::Expression(expressionValue, new std::string("variable"));
        values->push_back(value);

        $$ = values;
    }
    | expr {
        std::vector<datatypes::Expression*>* values = new std::vector<datatypes::Expression*>;
        values->push_back($1);

        $$ = values;
    }
    | expr_list COMMA expr {
        std::vector<datatypes::Expression*>* values = $1;
        values->push_back($3);

        $$ = values;
    }
;

logical_expr
    : KW_TRUE {
        datatypes::ExpressionValue* expressionValue = new datatypes::ExpressionValue(std::string("TRUE"), std::string("BOOLEAN"));
        $$ = new datatypes::Expression(expressionValue, new std::string("const"));
    }
    | KW_FALSE {
        datatypes::ExpressionValue* expressionValue = new datatypes::ExpressionValue(std::string("FALSE"), std::string("BOOLEAN"));
        $$ = new datatypes::Expression(expressionValue, new std::string("const"));
    }
    | LPAREN logical_expr RPAREN {
        $$ = $2;
    }
    | expr OP_EQ expr {
        $$ = new datatypes::Expression($1, $3, new std::string("="));
    }
    | expr OP_NOTEQ expr {
        $$ = new datatypes::Expression($1, $3, new std::string("!="));
    }
    | logical_expr OP_AND logical_expr {
        $$ = new datatypes::Expression($1, $3, new std::string("AND"));
    }
    | logical_expr OP_OR logical_expr {
        $$ = new datatypes::Expression($1, $3, new std::string("OR"));
    }
    | OP_NOT logical_expr {
        $$ = new datatypes::Expression($2, new std::string("OR"));
    }
    | expr OP_GT expr {
        $$ = new datatypes::Expression($1, $3, new std::string(">"));
    }
    | expr OP_LS expr {
        $$ = new datatypes::Expression($1, $3, new std::string("<"));
    }
    | expr OP_GTE expr {
        $$ = new datatypes::Expression($1, $3, new std::string(">="));
    }
    | expr OP_LSE expr {
        $$ = new datatypes::Expression($1, $3, new std::string("<="));
    }
;

expr
    : REAL {
        datatypes::ExpressionValue* expressionValue = new datatypes::ExpressionValue(*$1, std::string("REAL"));
        $$ = new datatypes::Expression(expressionValue, new std::string("const"));
    }
    | STRING {
        datatypes::ExpressionValue* expressionValue = new datatypes::ExpressionValue(*$1, std::string("TEXT"));
        $$ = new datatypes::Expression(expressionValue, new std::string("const"));
    }
    | INTEGER {
        datatypes::ExpressionValue* expressionValue = new datatypes::ExpressionValue(std::to_string($1), std::string("INT"));
        $$ = new datatypes::Expression(expressionValue, new std::string("const"));
    }
    | IDENTIFIER {
        datatypes::ExpressionValue* expressionValue = new datatypes::ExpressionValue(*$1, std::string("UNKNOWN"));
        $$ = new datatypes::Expression(expressionValue, new std::string("variable"));
    }
    | expr PLUS expr {
        $$ = new datatypes::Expression($1, $3, new std::string("+"));
    }
    | expr DIVIDE expr {
        $$ = new datatypes::Expression($1, $3, new std::string("/"));
    }
    | expr MINUS expr {
        $$ = new datatypes::Expression($1, $3, new std::string("-"));
    }
    | expr TIMES expr {
        $$ = new datatypes::Expression($1, $3, new std::string("*"));
    }
    | LPAREN expr RPAREN {
        $$ = $2;
    }
    | logical_expr {
        $$ = $1;
    }
