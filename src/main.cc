#include <stdio.h>
#include "storage/storage.h"
#include "pagecache/pagecache.h"

#include "query.h"
#include "lexer.h"
#include "parser.h"

int initdb(const char*);
int prompt();

int main(int argc, const char *argv[]) {
    if (argc == 1) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "qdb <path to data storage>" << std::endl;
        return 1;
    }

    const char* command = argv[1];
    if (strcmp(command, "init") == 0) {
        if (argc < 3) {
            std::cout << "Provide path:" << std::endl;
            return 1;
        }
        std::cout << "command: init" << std::endl;
        return initdb(argv[2]);
    } else if (strcmp(command, "prompt") == 0) {
        return prompt();
    } else {
        return 1;
    }

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

int initdb(const char* location) {
    Storage storage(location);

    std::cout << storage.is_present() << std::endl;

    if (!storage.is_present()) {
        if (!storage.can_initialize()) {
            std::cerr << "Please specify one of" << std::endl;
            std::cerr << "  - path to existing storage (to use it)" << std::endl;
            std::cerr << "  - path to absolutely empty directory (to create storage there)" << std::endl;
            std::cerr << "  - non-existing path (to create storage there)" << std::endl;

            return 2;
        }
        std::cout << "initializing storage.." << std::endl;
        storage.initialize();
    }
    std::cout << storage.is_present() << std::endl;

    PageCache page {storage, {5, 16384}};

    return 0;
}

int prompt() {
    return 0;
}