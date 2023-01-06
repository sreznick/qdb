#include <stdio.h>
#include "storage/storage.h"
#include "pagecache/pagecache.h"
#include "pagecache/index.h"

#include "query.h"
#include "lexer.h"
#include "parser.h"


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
    std::cout << storage.is_present() << std::endl;

    PageCache page {storage, {5, 10}};

    auto btree = BTree<long long>(page, 20);

    for (int i = 0; i < 10000; i++) {
        btree.insert_key(i);
    }
    for (int i = 0; i < 10000; i++) {
        assert(btree.has_key(i));
    }

    for (int i = 10000; i < 20000; i++) {
        assert(!btree.has_key(i));
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
