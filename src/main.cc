#include <stdio.h>
#include "storage/storage.h"
#include "pagecache/pagecache.h"
#include "index/btree.h"
#include "index/btree.cc"

#include "query.h"
#include "lexer.h"
#include "parser.h"

bool comparator(int a, int b) {
    return a < b;
}

int main(int argc, const char *argv[]) {
    if (argc == 1) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "qdb <path to data storage>" << std::endl;
        return 1;
    }

    Storage storage(argv[1]);
    const char* mode = argv[2];

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

    if (mode[0] == 't') {
        BTree<int>* tree = new BTree<int>(4, comparator);

        tree->insert(2);
        tree->insert(5);
        tree->insert(3);

        std::cout << "search for 4: " << tree->search(4).first << " = nullptr\n";
        std::cout << "search for 5: " << tree->search(5).first << " found 5\n";

        tree->remove(5);
        // Fails for remove with bad key but we decided that we don't have 

        std::cout << "search for 5: " << tree->search(5).first << " = nullptr after delete\n";
        std::cout << "Test end";
        return 0;
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
