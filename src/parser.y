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

%token ASSIGN
%token KW_EQ KW_NEQ
%token PLUS MINUS TIMES DIVIDE LPAREN RPAREN
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
%token KW_INT
%token KW_REAL
%token KW_TEXT
%token KW_BOOLEAN
%token KW_TRUE
%token KW_FALSE
%token KW_OR
%token KW_AND
%token KW_NOT
%token KW_VALUES
%token KW_WHERE
%token KW_SET

%nonassoc ASSIGN
%left KW_OR
%left KW_AND
%nonassoc KW_NOT
%left KW_EQ KW_NEQ
%left PLUS MINUS
%left TIMES DIVIDE

%union {
  std::string *name;
  int number;
  float fnumber;
  std::string *svalue;

  query::Query* query;
  query::DataType* type;
  std::vector<query::FieldDefinition*>* field_defs;

  query::FieldValue* value;
  std::vector<query::FieldValue*>* field_values;

  query::LogicalExpr* logical_expr;
  query::Expr* expr;
  std::vector<query::Expr*>* exprs;

  query::FieldSet* field_set;
  std::vector<query::FieldSet*>* field_sets;
}

%token <name> IDENTIFIER
%token <number> NUMBER
%token <fnumber> REAL
%token <svalue> STRING
%nterm <query> start
%nterm <type> type
%nterm <field_defs> field_defs
%nterm <value> value
%nterm <field_values> field_values
%nterm <logical_expr> logical_expr
%nterm <expr> expr
%nterm <exprs> exprs
%nterm <field_set> field_set
%nterm <field_sets> field_sets
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
    KW_SELECT TIMES KW_FROM IDENTIFIER KW_WHERE logical_expr SEMICOLON {
        query::Select* insert = new query::Select(*$4, $6);
        query::Query* query = new query::Query(insert);
        $$ = query;
        *ret = query;
    } |
    KW_SELECT exprs KW_FROM IDENTIFIER KW_WHERE logical_expr SEMICOLON {
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
    KW_DELETE KW_FROM IDENTIFIER KW_WHERE logical_expr SEMICOLON {
            query::Delete* deleteQuery = new query::Delete(*$3, $5);
            query::Query* query = new query::Query(deleteQuery);
            $$ = query;
            *ret = query;
        } |
    KW_UPDATE IDENTIFIER KW_SET field_sets KW_WHERE logical_expr SEMICOLON {
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

field_values :
    value {
        $$ = new std::vector<query::FieldValue*>{};
        $$->push_back($1);
    } |
    value COMMA field_values {
        $$ = $3;
        $$->push_back($1);
    }

value :
    NUMBER {
        $$ = new query::FieldValue($1);
    } |
    REAL {
        $$ = new query::FieldValue($1);
    } |
    STRING {
        $$ = new query::FieldValue(*$1);
    }

field_sets :
    field_set {
        $$ = new std::vector<query::FieldSet*>{};
        $$->push_back($1);
    } |
    field_set COMMA field_sets {
        $$ = $3;
        $$->push_back($1);
    }

field_set : IDENTIFIER ASSIGN expr {
        $$ = new query::FieldSet(*$1, $3);
    }

type : KW_CHAR LPAREN NUMBER RPAREN {
         $$ = new query::DataType(new query::CharType{static_cast<unsigned int>($3)});
       } |
       KW_VARCHAR LPAREN NUMBER RPAREN {
         $$ = new query::DataType(new query::VarCharType{static_cast<unsigned int>($3)});
       } |
       KW_INT {
         $$ = new query::DataType(query::BasicType::INT);
       } |
       KW_REAL {
         $$ = new query::DataType(query::BasicType::REAL);
       } |
       KW_TEXT {
         $$ = new query::DataType(query::BasicType::TEXT);
       } |
       KW_BOOLEAN {
         $$ = new query::DataType(query::BasicType::BOOLEAN);
       }
;

exprs:
    expr {
        $$ = new std::vector<query::Expr*>{};
        $$->push_back($1);
    } |
    expr COMMA exprs {
        $$ = $3;
        $$->push_back($1);
    }

logical_expr :
    KW_TRUE {
        $$ = new query::LogicalExpr(query::LogicalExprType::TRUE);
    } |
    KW_FALSE {
        $$ = new query::LogicalExpr(query::LogicalExprType::FALSE);
    } |
    LPAREN logical_expr RPAREN {
        $$ = new query::LogicalExpr(query::LogicalExprType::LOG_BRACKETS);
        $$->_rightLogicalExpr = $2;
    } |
    expr KW_EQ expr {
        $$ = new query::LogicalExpr(query::LogicalExprType::EQ);
        $$->_leftExpr = $1;
        $$->_rightExpr = $3;
    } |
    expr KW_NEQ expr {
        $$ = new query::LogicalExpr(query::LogicalExprType::NEQ);
        $$->_leftExpr = $1;
        $$->_rightExpr = $3;
    } |
    logical_expr KW_OR logical_expr {
        $$ = new query::LogicalExpr(query::LogicalExprType::OR);
        $$->_leftLogicalExpr = $1;
        $$->_rightLogicalExpr = $3;
    } |
    logical_expr KW_AND logical_expr {
        $$ = new query::LogicalExpr(query::LogicalExprType::AND);
        $$->_leftLogicalExpr = $1;
        $$->_rightLogicalExpr = $3;
    } |
    KW_NOT logical_expr {
        $$ = new query::LogicalExpr(query::LogicalExprType::NOT);
        $$->_rightLogicalExpr = $2;
    }

expr:
    expr PLUS expr {
        $$ = new query::Expr(query::ExprType::PLUS);
        $$->_leftExpr = $1;
        $$->_rightExpr = $3;
    } |
    MINUS expr %prec TIMES {
        $$ = new query::Expr(query::ExprType::UN_MINUS);
        $$->_rightExpr = $2;
    } |
    expr MINUS expr {
        $$ = new query::Expr(query::ExprType::MINUS);
        $$->_leftExpr = $1;
        $$->_rightExpr = $3;
    } |
    expr TIMES expr {
        $$ = new query::Expr(query::ExprType::MULTIPLY);
        $$->_leftExpr = $1;
        $$->_rightExpr = $3;
    } |
    expr DIVIDE expr {
        $$ = new query::Expr(query::ExprType::DIVIDE);
        $$->_leftExpr = $1;
        $$->_rightExpr = $3;
    } |
    LPAREN expr RPAREN {
        $$ = new query::Expr(query::ExprType::BRACKETS);
        $$->_rightExpr = $2;
    } |
    NUMBER {
        $$ = new query::Expr(query::ExprType::INTEGER);
        $$->_integerField = $1;
    } |
    REAL {
        $$ = new query::Expr(query::ExprType::EXPR_REAL);
        $$->_floatField = $1;
    } |
    STRING {
        $$ = new query::Expr(query::ExprType::STRING);
        $$->_stringField = *$1;
    } |
    IDENTIFIER {
        $$ = new query::Expr(query::ExprType::FIELD);
        $$->_stringField = *$1;
    } |
    logical_expr {
        $$ = new query::Expr(query::ExprType::LOGICAL_EXPR);
        $$->_logicalExpr = $1;
    }
