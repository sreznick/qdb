#pragma once

#include <string>
#include <variant>
#include <vector>
#include <attribute.h>
#include <datatype.h>
#include <parsing_utils.h>

namespace query {
    class BaseQuery {
    public:
        std::string query_type;
        std::string table_name;

        BaseQuery(std::string query_type, std::string table_name): query_type(query_type), table_name(table_name) {
        }

        void printProps() {};
    };

    class CreateTable : public BaseQuery {
    public:
        std::vector<datatypes::Column*>* columns;

        CreateTable(std::string table_name, std::vector<datatypes::Column*>* columns): BaseQuery("CREATE", table_name), columns(columns) {}

        void printProps() {
            printf("COLUMNS DATA\n");

            for (datatypes::Column* column : *columns) {
                printf("name: %s\n", column->name.c_str());
                printf("%s\n", column->datatype->to_s().c_str());
            }
            printf("\n");
        }
    };

    class Insert : public BaseQuery {
    public:
        std::vector<datatypes::Literal*> insertion_values;

        Insert(std::string tn, std::vector<datatypes::Literal*> insertion_values): BaseQuery("INSERT", tn), insertion_values(insertion_values) {

        }

        void printProps() {
            printf("Insertion data:\n");
            for (datatypes::Literal* literal : insertion_values) {
                printf("%s :: %s\n", literal->value.c_str(), literal->type.c_str());
            }
            printf("\n");
        };
    };

    class Select : public BaseQuery {
        std::vector<datatypes::Expression*> expressions;
        datatypes::Expression* whereExpr;

    public:
        Select(std::string tn, std::vector<datatypes::Expression*> expressions) : BaseQuery("SELECT", tn), expressions(expressions) {}
        Select(std::string tn, std::vector<datatypes::Expression*> expressions, datatypes::Expression* whereExpr) : BaseQuery("SELECT", tn), expressions(expressions), whereExpr(whereExpr) {}
        void printProps() {
            for (datatypes::Expression* expression : expressions) {
                std::cout << "\nSelect expression:\n";
                parsing_utils::printExpression(expression);
                std::cout << std::endl;
            }

            if (whereExpr != nullptr) {
                std::cout << "Where expression:\n";
                parsing_utils::printExpression(whereExpr);
                std::cout << std::endl;
            }
        }

        std::vector<datatypes::Expression*> getExpressions() {
            return expressions;
        }
    };

    class Query {
    public:
        std::variant<CreateTable*, Insert*, Select*> query;

        int type() {
            if (std::holds_alternative<CreateTable*>(query)) {
              return 0;
            } else if (std::holds_alternative<Insert*>(query)) {
                return 1;
            } else if (std::holds_alternative<Select*>(query)) {
                return 2;
            }

            return -1;
        }

        Query(CreateTable* createTable): query(createTable) {}

        Query(Insert* insertIntoTable): query(insertIntoTable) {}

        Query(Select* selectFromTable): query(selectFromTable) {}
    };
};

