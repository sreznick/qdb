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
                case query::QueryType::CREATE: {
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

                    auto fileId = next_file_id(pageCachePtr);

                    auto tablePtr = create_table(pageCachePtr, fileId, tableSchemePtr, tableName);
                    break;
                }

                case query::QueryType::INSERT: {
                    auto query = get<query::Insert*>(ret->query);
                    std::string tableName = query->table_name;


                    auto allTables = get_tables(pageCachePtr);
                    if (allTables.find(tableName) == allTables.end()) {
                        std::cout << "ERROR: table with name " << tableName << " does not exist";
                        exit(EXIT_FAILURE);
                    }
                    auto table = allTables.at(tableName);
                    auto tableSchemePtr = table.scheme();
                    auto tableScheme = tableSchemePtr.get();


                    auto tablePtr = get_table_ptr(tableSchemePtr);

                    DenseTuple denseTuple = DenseTuple(tableSchemePtr);

                    auto values = query->insertion_values;
                    for (int i = 0; i < tableScheme->columnsCount(); i++) {
                        if (tableScheme->typeName(i) == "INT") {
                            denseTuple.setInt(i, std::stoi(values.at(i)->value));
                        } else if (tableScheme->typeName(i) == "CHAR") {
                            denseTuple.setChar(i, values.at(i)->value);
                        } else if (tableScheme->typeName(i) == "BOOLEAN") {
                            denseTuple.setBool(i, values.at(i)->value == "TRUE");
                        } else if (tableScheme->typeName(i) == "DOUBLE") {
                            denseTuple.setDouble(i, std::stod(values.at(i)->value));
                        } else {
                            std::cout << "unknown type" << std::endl;
                        }
                    }
                    auto denseTuplePtr = std::make_shared<DenseTuple>(denseTuple);

                    insert_tuple(pageCachePtr, tablePtr, denseTuplePtr, table.lastPageId());
                    break;
                }
                case query::QueryType::SELECT: {
                    auto query = get<query::Select*>(ret->query);
                    auto tableName = query->table_name;

                    auto allTables = get_tables(pageCachePtr);
                    // query->printProps();

                    if (allTables.find(tableName) == allTables.end()) {
                        std::cout << "ERROR: table with name " << tableName << " does not exist";
                        exit(EXIT_FAILURE);
                    }
                    auto table = allTables.at(tableName);
                    auto tableSchemePtr = table.scheme();

                    auto tuplesRepr = select(pageCachePtr, tableSchemePtr, table.fileId(), table.lastPageId().id, query->getWhereExpression());
                    pretty_print_relation(tuplesRepr);
                    break;
                }
                default: {
                    std::cout << "ERROR: Unsupported SQL operator" << std::endl;
                }
            }
        }

        yy_delete_buffer(state);
    }

    return 0;
}