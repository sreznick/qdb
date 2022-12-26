#include <stdio.h>

#include "query.h"
#include "lexer.h"
#include "parser.h"
#include <fstream>
#include <iostream>

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

void handleCreate(query::CreateTable* query){
    printf("Создана таблица: %s\n", query->name().c_str());
    std::vector<query::FieldDefinition*>* definitions = query->fdef();

    printf("Структура полей:\n");
    for (query::FieldDefinition* definition : *definitions) {
        std::string type = "неизвестно";

        if (definition->ftp() == 1) {
            type = "INT";
        } else if (definition->ftp() == 1) {
            type = "REAL";
        } else if (definition->ftp() == 1) {
            type = "TEXT";
        } else if (definition->ftp() == 1) {
            type = "BOOLEAN";
        } else if (definition->ftp() == 1) {
            type = "CHAR(NUMBER)";
        } else if (definition->ftp() == 1) {
            type = "VARCHAR(NUMBER)";
        }

        printf("Поле: %s типа: %s\n", definition->name().c_str(), type.c_str());
    }
}

void handleUpdate(query::Update* query){
    printf("Обновлена таблица %s\n", query->name().c_str());

    std::vector<std::pair<query::Exp*, std::string>*>* field_setups = query->setups();

    for (auto setup : *field_setups) {
        printf("Поле: %s типа: %s\n", setup->second.c_str(), ((setup->first))->print().c_str());
    }

    printf("Where:\n%s\n", query->log()->print().c_str());
}

void handleDelete(query::Delete* query){
    printf("Удаление в таблице: %s\n", query->name().c_str());

    query::Exp* log = query->log();

    if (log == nullptr) {
        printf("Условие отсутствует.\n");
    } else {
        printf("С условием: %s\n", log->print().c_str());
    }
}

void handleInsert(query::Insert* query){
    printf("Добавление данных в таблицу: %s\n", query->name().c_str());
    std::vector<query::VariantType*>* vals = query->vtypes();

    printf("Типы данных:\n");
    for (query::VariantType* val : *vals) {
        printf("%s ", val->type().c_str());
    }

    printf("\n");
}

void handleSelect(query::Select* query){
    printf("Выбрать из таблицы: %s\n", query->name().c_str());
    std::vector<query::Exp*>* exps = query->expressions();

    if (exps == nullptr) {
        printf("Всё содержимое.\n");
    } else {
        for (query::Exp* exp : *exps) {
            printf("Поля: %s\n", exp->print().c_str());
        }
    }

    query::Exp* log = query->log();

    if (log == nullptr) {
        printf("Условие отсутствует.\n");
    } else {
        printf("С условием: %s\n", log->print().c_str());
    }
}


void promt(bool interact, std::vector<std::string> commands) {
    int idx = -1;
    
    if (!interact) {
        idx = 0;
    } else {
        printf("sql>");
        fflush(stdout);
        fflush(stdin);
    }

    while (idx < commands.size()) {
        
        char input[1024];
        YY_BUFFER_STATE state;

        if (interact) {
            if (!fgets(input, sizeof(input), stdin)) {
                break;
            }

            if (input[0] == '\n') {
                continue;
            }

            if (!(state = yy_scan_bytes(input, strcspn(input, "\n")))) {
                continue;
            }
        } else {
            idx++;
            if (!(state = yy_scan_bytes(commands[idx - 1].c_str(), strcspn(commands[idx - 1].c_str(), "\n")))) {
                continue;
            }
        }

        query::Query* query;

        if (yyparse(&query) == 0) {
            printf("= %d\n", query->type());
            query->debug_print();

            std::cout << "query: ";
            if (query->type() == 0) {
                handleCreate(query->createTable());
            } else if (query->type() == 4) {
                handleUpdate(query->update());
            } else if (query->type() == 3) {
                handleDelete(query->deleteQuery());
            } else if (query->type() == 1) {
                handleInsert(query->insert());
            } else if (query->type() == 2) {
                handleSelect(query->select());
            } else {
                std::cout << "Unknown command." << std::endl;
            }
        }

        yy_delete_buffer(state);
    }
}

int main(int argc, const char *argv[]) {
    if (argc == 1) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "qdb <path to data storage>" << std::endl;
        return 1;
    }

    if (argc > 2) {
        const char* mode = argv[2];
        if (mode[0] == '-' && mode[1] == 'f') {
            std::vector<std::string> cmds;

            std::ifstream F;

            F.open(argv[3]);

            std::string line;

            while (std::getline(F, line))
            {
                cmds.push_back(line);
            }

            F.close();

            promt(false, cmds);
            return 0;
        }
    }

    promt(true, {});
    return 0;
}
