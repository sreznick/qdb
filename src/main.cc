#include <stdio.h>
#include "storage/storage.h"
#include "pagecache/pagecache.h"

#include "query.h"
#include "lexer.h"
#include "parser.h"
#include <fstream>

std::vector<std::string> loadfile(std::string filename) {
    std::vector<std::string> ret;

    std::ifstream fs(filename.c_str());

    std::string line;

    while (std::getline(fs, line)) {
        ret.push_back(line);
    }

    fs.close();

   return ret;
}

std::string query::Expression::print() {
    std::string left = "", right = "", mid = "";

    if (leftExpression != nullptr) {
        left = leftExpression->print();
    }

    if (rightExpression != nullptr) {
        right = rightExpression->print();
    }

    if (type == ExpressionType::INTEGER) {
        mid = std::to_string(integerField);
    } else if (type == ExpressionType::EXPR_REAL) {
        mid = std::to_string(floatField);
    } else if (type == ExpressionType::STRING || type == ExpressionType::FIELD) {
        mid = stringField;
    } else if (type == ExpressionType::LOGICAL_EXPR) {
        mid = logicalExpression->print();
    } else if (type == ExpressionType::BRACKETS) {
        mid = "(" + leftExpression->print() + ")";
        left = "";
    } else if (type == ExpressionType::PLUS) {
        mid = "+";
    } else if (type == ExpressionType::MINUS) {
        mid = "-";
    } else if (type == ExpressionType::DIVIDE) {
        mid = "/";
    } else if (type == ExpressionType::MULTIPLY) {
        mid = "*";
    }

    return left + " " +  mid + " " + right;
}

void printer(query::Query* query) {
    std::cout << "query: ";
    switch (query->type())
    {
    case query::Type::CREATE: {
            query::CreateTable* datac = query->createTable();

            std::cout << "CREATE\nTable: " << datac->name() << "\n";
            std::vector<query::FieldDefinition*>* defs = datac->defs();

            std::cout << "Definitions:\n";
            for (query::FieldDefinition* def : *defs) {
                std::cout << "Name: " << def->name() << " Type: " << def->type() << std::endl;
            }
            break;
    }
    case query::Type::UPDATE: {
        query::Update* datau = query->update();

        std::cout << "UPDATE\nTable: " << datau->name() << "\n";

        std::vector<query::FieldSetup*>* sets = datau->setups();

        for (query::FieldSetup* set : *sets) {
            std::cout << "Name: " << set->name() << " Expr: " << set->expression()->print() << std::endl;
        }

        std::cout << "Where:\n";
        std::cout << datau->logical()->print() << std::endl;
        break;
    }
    case query::Type::DELETE: {
        query::Delete* datad = query->deleteQuery();

        std::cout << "DELETE\nTable: " << datad->name() << "\n";

        query::LogicalExpression* logical = datad->logical();

        if (logical == nullptr) {
            std::cout << "With no logical statement\n";
            break;
        }

        std::cout << "Where:\n" << logical->print() << std::endl;
        break;
    }
    case query::Type::INSERT: {
        query::Insert* datai = query->insert();

        std::cout << "INSERT\nTable: " << datai->name() << "\n";
        std::vector<query::FieldValue*>* vals = datai->values();

        std::cout << "Value types:\n";
        for (query::FieldValue* val : *vals) {
            std::cout << val->type() << " ";
        }

        std::cout << std::endl;
        break;
    }
    case query::Type::SELECT: {
        query::Select* datas = query->select();

        std::cout << "SELECT\nTable: " << datas->name() << "\n";
        std::vector<query::Expression*>* exps = datas->expressions();

        if (exps == nullptr) {
            std::cout << "\n All content from table\n";
        } else {
            for (query::Expression* exp : *exps) {
                std::cout << exp->print() << std::endl;
            }
        }

        query::LogicalExpression* logical = datas->logical();

        if (logical == nullptr) {
            std::cout << "With no logical statement\n";
            break;
        }

        std::cout << "Where:\n" << logical->print() << std::endl;
        break;
    }
    case query::Type::UNKNOWN: {
        std::cout << "Unknown command." << std::endl;
        break;
    }
    default:
        break;
    }
}

int main(int argc, const char *argv[]) {
    if (argc == 1) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "qdb <path to data storage>" << std::endl;
        return 1;
    }

    Storage storage(argv[1]);

    if (!storage.is_present()) {
        if (!storage.can_initialize()) {
            std::cerr << "Please specify one of" << std::endl;
            std::cerr << "  - path to existing storage (to use it)" << std::endl;
            std::cerr << "  - path to absolutely empty directory (to create storage there)" << std::endl;
            std::cerr << "  - non-existing path (to create storage there)" << std::endl;

            return 2;
        }
        storage.initialize();
    }
    std::cout << (storage.is_present() ? "Storage initialized" : "Storage error") << std::endl;
    if (argc > 2) {
        const char* mode = argv[2];
        if (mode[0] == 't') {
            PageCache page {storage, {3, 4}};
            page.self_test();
            return 0;
        } else if (mode[0] == '-' && mode[1] == 'f') {
            std::vector<std::string> cmds = loadfile(argv[3]);

            for (auto cmd : cmds) {
                YY_BUFFER_STATE state;

                if (!(state = yy_scan_bytes(cmd.c_str(), strcspn(cmd.c_str(), "\n")))) {
                    continue;
                }

                query::Query* ret;

                if (yyparse(&ret) == 0) {
                    printf("= %d\n", ret->type());
                    ret->debug_print();
                    printer(ret);
                }

                yy_delete_buffer(state);
            }
            return 0;
        }
    }

    PageCache page {storage, {5, 16384}};
    while (1) {
        printf("sql>");
        fflush(stdout);
        fflush(stdin);
        char input[1024];

        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        if (input[0] == '\n') {
            continue;
         }

        YY_BUFFER_STATE state;

        if (!(state = yy_scan_bytes(input, strcspn(input, "\n")))) {
            continue;
        }

        query::Query* ret;

        if (yyparse(&ret) == 0) {
            printf("= %d\n", ret->type());
            ret->debug_print();

            printer(ret);
/*
            switch (ret->type()) {
                case query::Type::CREATE: {
                    printf("= %s\n", ret->createTable()->name().c_str());
                }
                break;

                case query::Type::INSERT: {
                    printf("= %s\n", ret->insert()->name().c_str());
                }
                break;

            }
*/
        }

        yy_delete_buffer(state);
    }

    return 0;
}


//CREATE TABLE hello(id int, name varchar(255));
//INSERT INTO hello VALUES(1, "abc");

//CREATE TABLE hello(id int);
//INSERT INTO hello VALUES(1);
//