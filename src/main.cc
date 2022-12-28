#include <stdio.h>
#include "storage/storage.h"
#include "pagecache/pagecache.h"
#include "table/table.h"
#include "vector"

#include "query.h"
#include "lexer.h"
#include "parser.h"
#include "representation.h"
#include <qengine/qengine.h>
#include <common/common.h>

int initdb(const char*);
int prompt(const char*);

int main(int argc, const char *argv[]) {
    if (argc == 1) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << "qdb <path to data storage>" << std::endl;
        return 1;
    }

    const char* command = argv[1];
    if (strcmp(command, "initdb") == 0) {
        if (argc < 3) {
            std::cout << "Provide path:" << std::endl;
            return 1;
        }
        std::cout << "command: initdb" << std::endl;
        return initdb(argv[2]);
    } else if (strcmp(command, "prompt") == 0) {
        return prompt(argv[2]);
    } else {
        std::cerr << "unknown command" << std::endl;
        exit(EXIT_FAILURE);
    }
}

int initdb(const char* location) {
    Storage storage(location);

    // std::cout << storage.is_present() << std::endl;

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
    //std::cout << storage.is_present() << std::endl;

    PageCache pageCache {storage, {5, 16384}};

    init_db(std::make_shared<PageCache>(pageCache));

    return 0;
}

int prompt(const char* location) {
    Storage storage(location);

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

    PageCache pageCache{storage, {5, 16384}};
    std::shared_ptr<PageCache> pageCachePtr = std::make_shared<PageCache>(pageCache);

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

        if (strcmp(input, "exit\n") == 0) {
            commit(pageCachePtr);
            exit(0);
        }

        YY_BUFFER_STATE state;

        if (!(state = yy_scan_bytes(input, strcspn(input, "\n")))) {
            continue;
        }

        query::Query* ret;

        if (yyparse(&ret) == 0) {
            printf("= %d\n", ret->type());

            switch (ret->type()) {
                case 0: {
                    auto query = get<query::CreateTable*>(ret->query);
                    std::string tableName = query->table_name;
                    auto columns = query->columns;

                    std::vector<ColumnScheme> columnsVector;
                    for (int i = 0; i < columns->size(); i++) {
                        auto column = columns->at(i);
                        std::string datatype = column->datatype->name;
                        if (datatype == "REAL") datatype = "DOUBLE";
                        TypeTag typeTag = TypesRegistry::typeTagByName(datatype);
                        int size = 0;
                        if (!TypesRegistry::hasFixedSize(typeTag)) {
                            size = column->datatype->size;
                        }

                        auto columnScheme = ColumnScheme(column->name, typeTag, size);
                        if (!columnScheme.ok()) {
                            std::cout << "ERROR: Invalid column spec";
                            exit(EXIT_FAILURE);
                        }
                        columnsVector.push_back(columnScheme);
                    }

                    std::shared_ptr<std::vector<ColumnScheme>> ptr = std::make_shared<std::vector<ColumnScheme>>(columnsVector);

                    TableScheme tableScheme = TableScheme(ptr);
                    std::shared_ptr<TableScheme> tableSchemePtr = std::make_shared<TableScheme>(tableScheme);


                    auto tablePtr = create_table(pageCachePtr, FileId{2}, tableSchemePtr, tableName);
                    break;
                }

                case 1: {
                    PageId FAKE_PAGE_ID = {{2}, 0}; // mock

                    auto query = get<query::Insert*>(ret->query);
                    std::string tableName = query->table_name;


                    auto allTables = get_tables(pageCachePtr);
                    for (const auto & [key, value] : allTables) {
                        pretty_print_table_scheme(value);
                    }
                    if (allTables.find(tableName) == allTables.end()) {
                        std::cout << "ERROR: table with name " << tableName << " does not exist";
                        exit(EXIT_FAILURE);
                    }
                    auto tableScheme = allTables.at(tableName);
                    std::shared_ptr<TableScheme> tableSchemePtr = std::make_shared<TableScheme>(tableScheme);


                    auto tablePtr = get_table_ptr(tableSchemePtr);

                    DenseTuple denseTuple = DenseTuple(tableSchemePtr);

                    auto values = query->insertion_values;
                    for (int i = 0; i < tableScheme.columnsCount(); i++) {
                        if (tableScheme.typeName(i) == "INT") {
                            denseTuple.setInt(i, std::stoi(values.at(i)->value));
                        } else if (tableScheme.typeName(i) == "CHAR") {
                            denseTuple.setChar(i, values.at(i)->value);
                        } else if (tableScheme.typeName(i) == "BOOLEAN") {
                            denseTuple.setBool(i, values.at(i)->value == "TRUE");
                        } else if (tableScheme.typeName(i) == "DOBULE") {
                            denseTuple.setDouble(i, std::stod(values.at(i)->value));
                        } else {
                            std::cout << "unknown type" << std::endl;
                        }
                    }
                    auto denseTuplePtr = std::make_shared<DenseTuple>(denseTuple);

                    insert_tuple(pageCachePtr, tablePtr, denseTuplePtr, FAKE_PAGE_ID);
                    break;
                }
                case 2: {
                    PageId FAKE_PAGE_ID = {{2}, 0}; // mock

                    auto query = get<query::Select*>(ret->query);
                    auto expressions = query->getExpressions();
                    auto tableName = query->table_name;

                    auto allTables = get_tables(pageCachePtr);

                    if (allTables.find(tableName) == allTables.end()) {
                        std::cout << "ERROR: table with name " << tableName << " does not exist";
                        exit(EXIT_FAILURE);
                    }
                    auto tableScheme = allTables.at(tableName);
                    std::shared_ptr<TableScheme> tableSchemePtr = std::make_shared<TableScheme>(tableScheme);

                    auto tuples = select_all(pageCachePtr, tableSchemePtr, FAKE_PAGE_ID);
                    for (int i = 0; i < tuples.size(); i++) {
                        auto tuple = tuples.at(i);
                        pretty_print_tuple(std::make_shared<DenseTuple>(tuple));
                    }
                    break;
                }
                default: {
                    std::cout << "ERROR: Unsupported SQL operator" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }

        yy_delete_buffer(state);
    }

    return 0;
}