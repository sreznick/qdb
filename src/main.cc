#include <stdio.h>
#include <fstream>

#include "storage/storage.h"
#include "table/table.h"
#include "pagecache/pagecache.h"
#include "qengine/qengine.h"

#include "query.h"
#include "lexer.h"
#include "parser.h"

#define PAGE_CACHE_COUNT_LIMIT 5
#define PAGE_CACHE_PAGE_COUNT 16384

int init(Storage storage);
void example();
void prompt(std::shared_ptr<PageCache> pageCache);
void file_prompt(std::string filename);

std::vector<std::string> load_queries(std::string filename) {
    std::ifstream file(filename.c_str());
    std::string line;

    std::vector<std::string> quieres; // ?
    while (std::getline(file, line)) {
        quieres.push_back(line);
    }

    file.close();
    return quieres;
}

int main(int argc, const char *argv[]) {
    if (argc == 1) {
        std::cerr << "Usage:" << std::endl;
        std::cerr << std::endl;
        std::cerr << "qdb <command> <path to data storage>" << std::endl;
        std::cerr << std::endl;
        std::cerr << "  command is one of:" << std::endl;
        std::cerr << std::endl;
        std::cerr << "  init - initialize db" << std::endl;
        std::cerr << "  prompt - start client prompt" << std::endl;
        std::cerr << "  file - start client prompt from file" << std::endl;
        std::cerr << "  example - demonstrating branch" << std::endl;
        std::cerr << "  test - run PageCache::test()" << std::endl;

        return 1;
    }

    Storage storage(argv[2]);

    if (std::string("init") == argv[1]) {
        return init(storage);
    }

    if (!storage.is_present()) {
        std::cerr << "Please initialize database first." << std::endl;
        return 2;
    }

    PageCache pageCache {storage, {PAGE_CACHE_COUNT_LIMIT, PAGE_CACHE_PAGE_COUNT}};
    auto pageCachePtr = std::make_shared<PageCache>(pageCache);

    if (std::string("example") == argv[1]) {
        example();
        return 0;
    }

    if (std::string("prompt") == argv[1]) {
        prompt(pageCachePtr);
        return 0;
    }

    if (std::string("file") == argv[1] && argc > 3) {
        file_prompt(argv[3]);
        return 0;
    }
    if (std::string("test") == argv[1]) {
        PageCache page_cache {storage, {1, 3}};
        page_cache.test();
        std::cerr << "Tests passed!" << std::endl;
    }

    return 0;
}


int init(Storage storage) {
    // TODO: реализовать инициализацию базы
    // 
    // В предположении, что папка пустая, нужно создать 
    // две мета-таблицы
    //
    // В одной будут храниться таблицы, в другой - поля
    //  
    // m_tables 
    //   INT qid (a.k.a. fileId)
    //   VARCHAR(50) name 
    //
    // m_columns 
    //   INT qid
    //   INT table_qid
    //   VARCHAR(50) name
    //   INT type_id - id-поле из структуры TypeInfo
    //   INT type_size - размер (если он изменяем, иначе 0)
    //
    //
    //  Данные метатаблиц тоже должны присутствовать в метатаблицах
    //

    if (storage.is_present()) {
        std::cerr << "You cannot initialize already initialized database." << std::endl;
        return 2;
    }
    if (!storage.can_initialize()) {
        std::cerr << "Please specify one of" << std::endl;
        std::cerr << "  - path to existing storage (to use it)" << std::endl;
        std::cerr << "  - path to absolutely empty directory (to create storage there)" << std::endl;
        std::cerr << "  - non-existing path (to create storage there)" << std::endl;

        return 2;
    }
    storage.initialize();

    PageCache pageCache {storage, {PAGE_CACHE_COUNT_LIMIT, PAGE_CACHE_PAGE_COUNT}};
    auto pageCachePtr = std::make_shared<PageCache>(pageCache);
    init_m_tables(pageCachePtr);

    return 0;
}

void example() {
    TypeInfo typeInfo = TypesRegistry::byId(0);
    std::cout << "id 0" << std::endl;
    std::cout << "name " << typeInfo.name << std::endl;  
    std::cout << "tag is TypeTag::INT " << (typeInfo.tag == TypeTag::INT) << std::endl;
    std::cout << "fixedSize " << typeInfo.fixedSize << std::endl;
    std::cout << std::endl;

    std::cout << "id 1" << std::endl;
    std::cout << "name " << TypesRegistry::byId(1).name << std::endl;
    std::cout << "tag " << (typeInfo.tag == TypeTag::LONG) << std::endl;
//    std::cout << "fixedSize " << typeInfo.fixedSize << std::endl;
//    std::cout << std::endl;
    std::cout << std::endl;


    ColumnScheme columnValue{"value", TypeTag::INT};
    std::cout << "columnValue" << std::endl;
    std::cout << "name " << columnValue.name() << std::endl;
    std::cout << "size " << columnValue.size() << std::endl;
    std::cout << "hasFixedSize " << columnValue.hasFixedSize() << std::endl;
    std::cout << std::endl;

    ColumnScheme columnName{"name", TypeTag::VARCHAR, 20};
    std::cout << "columnName" << std::endl;
    std::cout << "name " << columnName.name() << std::endl;
    std::cout << "size " << columnName.size() << std::endl;
    std::cout << "hasFixedSize " << columnName.hasFixedSize() << std::endl;
    std::cout << std::endl;

    auto columns = std::make_shared<std::vector<ColumnScheme>>();
    columns->push_back(columnValue);
    columns->push_back(columnName);
    TableScheme tableScheme = TableScheme{columns};
    std::cout << "totalSize " << tableScheme.totalSize()  << std::endl;
    std::cout << "name size " << tableScheme.column("name").size()  << std::endl;
    std::cout << std::endl;

    Table table{"data", std::make_shared<TableScheme>(tableScheme), FileId{15}};
    std::cout << "name " << std::endl;
    std::cout << std::endl;

    DenseTuple tuple{table.scheme()};
    tuple.setInt(0, 1234);
    tuple.setChar(1, "hello, world");

    std::cout << "[0]: " << tuple.getInt(0) << std::endl;
    std::cout << "[1]: " << tuple.getChar(1) << std::endl;

    std::cout << std::endl;

    tuple.print_header();
    tuple.print_values();
    std::cout << std::endl;
}

std::pair<std::map<std::string, std::shared_ptr<Table>>, std::int32_t>
        session_init(std::shared_ptr<PageCache> pageCache) {
    std::map<std::string, std::shared_ptr<Table>> tableDict;

    std::string m_tables_name("m_tables");
    std::string m_columns_name("m_columns");

    tableDict[m_tables_name] = std::make_shared<Table>(
            Table(m_tables_name, get_m_tables_scheme(), FileId { M_TABLES_FILE_ID }));
    tableDict[m_columns_name] = std::make_shared<Table>(
            Table(m_columns_name, get_m_columns_scheme(), FileId { M_COLUMNS_FILE_ID }));

    std::int32_t maxFileId = std::max(M_TABLES_FILE_ID, M_COLUMNS_FILE_ID);
    auto m_tables = select_all(pageCache, tableDict[m_tables_name]);

    std::map<std::int32_t, std::string> nameDict;
    std::map<std::int32_t, std::map<std::int32_t, ColumnScheme>> columnSchemesDict;

    for (auto m_table : m_tables) {
        std::int32_t i = m_table.getInt(0);
        maxFileId = std::max(maxFileId, i);

        nameDict[i] = m_table.getChar(1);
        columnSchemesDict[i] = std::map<std::int32_t, ColumnScheme>();
    }

    auto m_columns = select_all(pageCache, tableDict[m_columns_name]);

    for (auto m_column : m_columns) {
        auto columnId = m_column.getInt(0);
        auto tableId = m_column.getInt(1);
        auto columnName = m_column.getChar(2);
        auto typeId = m_column.getInt(3);
        auto typeSize = m_column.getInt(4);

        columnSchemesDict[tableId][columnId] = ColumnScheme(columnName, TypesRegistry::typeTag(typeId), typeSize);
    }

    for (auto m_table : m_tables) {
        std::int32_t id = m_table.getInt(0);
        auto tableName = m_table.getChar(1);

        auto currentColumnSchemesDict = columnSchemesDict[id];
        std::vector<ColumnScheme> columnSchemesVector;

        for (std::int32_t column_i = 0; currentColumnSchemesDict.count(column_i) > 0; ++column_i)
            columnSchemesVector.push_back(currentColumnSchemesDict[column_i]);

        auto columnSchemesVectorPtr = std::make_shared<std::vector<ColumnScheme>>(columnSchemesVector);
        TableScheme tableScheme(columnSchemesVectorPtr);
        auto tableSchemePtr = std::make_shared<TableScheme>(tableScheme);

        tableDict[tableName] = std::make_shared<Table>(Table(tableName, tableSchemePtr, FileId { id }));
    }

    return { tableDict, maxFileId };
}

void prompt(std::shared_ptr<PageCache> pageCache) {
/*
 *  TODO 
 * 
 *    - добавить логику из прошлого ДЗ с разбором запросов
 *    - вызывать функции из qengine для CREATE TABLE, INSERT, SELECT
 *    - для SELECT напечатать результат в виде простой таблицы
 *    - для CREATE TABLE напечатать 'table <name> created'
 *    - для INSERT напечатать количество записей после добавления
 *
 */

    auto sessionInitInfo = session_init(pageCache);
    auto tableDict = sessionInitInfo.first;
    auto maxFileId = sessionInitInfo.second;

    // Обработка запросов.

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
            // printf("= %d\n", ret->type());
            // ret->debug_print();

            switch (ret->type()) {
                case query::Type::SELECT: {
                    auto tableName = ret->select()->name();

                    if (tableDict.count(tableName) == 0) {
                        std::cout << "No such table." << std::endl;
                        break;
                    }

                    auto selected = select_all(pageCache, tableDict[tableName]); // TODO: SELECT с WHERE.

                    if (selected.size() == 0) {
                        std::cout << "Empty SELECT." << std::endl;
                        break;
                    }

                    bool hasHeader = false;
                    for (auto elem : selected) {
                        if (!hasHeader) {
                            elem.print_header();
                            hasHeader = true;
                        }
                        elem.print_values();
                    }
                }
                break;

                case query::Type::CREATE: {
                    auto tableName = ret->createTable()->name();

                    if (tableName.size() > MAX_NAME_LENGTH) {
                        std::cout << "Table name '" << tableName << "' is too long." << std::endl;
                        break;
                    }

                    if (tableDict.count(tableName) != 0) {
                        std::cout << "Table already exists." << std::endl;
                        break;
                    }

                    auto fieldDefs = ret->createTable()->fieldDefs();

                    std::vector<ColumnScheme> tableColumns;

                    bool brokenFor = false;
                    for (auto fieldDef : *fieldDefs) {
                        if (fieldDef->name().size() > MAX_NAME_LENGTH) {
                            std::cout << "Column name '" << fieldDef->name() << "' is too long." << std::endl;
                            brokenFor = true;
                            break;
                        }

                        auto dbType = fieldDef->db_type();
                        tableColumns.push_back(ColumnScheme(fieldDef->name(), dbType.first, dbType.second));
                    }
                    if (brokenFor)
                        break;

                    auto tableColumnsPtr = std::make_shared<std::vector<ColumnScheme>>(tableColumns);

                    TableScheme tableScheme(tableColumnsPtr);
                    auto tableSchemePtr = std::make_shared<TableScheme>(tableScheme);

                    auto table = create_table(pageCache, FileId { ++maxFileId }, tableName, tableSchemePtr);
                    tableDict[tableName] = table;

                    std::cout << "Table " << tableName << " created." << std::endl;
                }
                break;

                case query::Type::INSERT: {
                    auto tableName = ret->insert()->name();

                    if (tableDict.count(tableName) == 0) {
                        std::cout << "No such table." << std::endl;
                        break;
                    }

                    auto table = tableDict[tableName];

                    auto fieldVals = ret->insert()->values();
                    auto tableScheme = table->scheme();

                    if (tableScheme->columnsCount() != fieldVals->size()) {
                        std::cout << "Incorrect number of values." << std::endl;
                        break;
                    }

                    DenseTuple denseTuple(tableScheme);

                    bool brokenFor = false;
                    for (int i = 0; i < fieldVals->size(); ++i) {
                        auto dbVal = (*fieldVals)[i]->dbGet();
                        auto type = dbVal.first;
                        auto value = dbVal.second;

                        tableScheme->typeTag(i);

                        if (type == "INT") {
                            if (tableScheme->typeTag(i) == TypeTag::INT) {
                                denseTuple.setInt(i, std::stoi(value));
                            }
                            else if (tableScheme->typeTag(i) == TypeTag::DOUBLE) {
                                denseTuple.setDouble(i, std::stod(value));
                            }
                            else {
                                std::cout << "Incorrect value for column '" <<
                                          tableScheme->name(i) << "'." << std::endl;
                                brokenFor = true;
                                break;
                            }
                        }
                        else if (type == "REAL") {
                            if (tableScheme->typeTag(i) != TypeTag::DOUBLE) {
                                std::cout << "Incorrect value for column '" <<
                                          tableScheme->name(i) << "'." << std::endl;
                                brokenFor = true;
                                break;
                            }

                            denseTuple.setDouble(i, std::stod(value));
                        }
                        else if (type == "TEXT") {
                            if (tableScheme->typeTag(i) != TypeTag::CHAR &&
                                tableScheme->typeTag(i) != TypeTag::VARCHAR) {
                                std::cout << "Incorrect value for column '" <<
                                          tableScheme->name(i) << "'." << std::endl;
                                brokenFor = true;
                                break;
                            }

                            if (tableScheme->typeTag(i) == TypeTag::CHAR && value.size() != tableScheme->fieldSize(i)) {
                                std::cout << "Incorrect size (" << value.size() << ") of value for column '" <<
                                          tableScheme->name(i) << "', which is CHAR[" << tableScheme->fieldSize(i) <<
                                          "]." << std::endl;
                                brokenFor = true;
                                break;
                            }

                            if (tableScheme->typeTag(i) == TypeTag::VARCHAR &&
                                value.size() > tableScheme->fieldSize(i)) {
                                std::cout << "Incorrect size (" << value.size() << ") of value for column '" <<
                                          tableScheme->name(i) << "', which is VARCHAR[" << tableScheme->fieldSize(i) <<
                                          "]." << std::endl;
                                brokenFor = true;
                                break;
                            }

                            denseTuple.setChar(i, value);
                        }
                        else if (type == "BOOLEAN") {
                            if (tableScheme->typeTag(i) != TypeTag::BOOL) {
                                std::cout << "Incorrect value for column '" <<
                                          tableScheme->name(i) << "'." << std::endl;
                                brokenFor = true;
                                break;
                            }

                            denseTuple.setBool(i, value != "");
                        }
                        else {
                            std::cout << type << " is not supported." << std::endl;
                            brokenFor = true;
                            break;
                        }
                    }
                    if (brokenFor)
                        break;

                    auto denseTuplePtr = std::make_shared<DenseTuple>(denseTuple);
                    auto tupleNum = insert_tuple(pageCache, table, denseTuplePtr);
                    std::cout << "Table " << tableName << " now has " << tupleNum << " tuples." << std::endl;
                }
                break;

            }
        }

        yy_delete_buffer(state);
    }

}

void file_prompt(std::string filename) {
    // Не работает. И не планирует. Пользуйтесь интерактивным режимом.

    std::vector<std::string> query_lines = load_queries(filename);
    for (std::string query_line : query_lines) {
        printf("\n%s\n", query_line.c_str());
        fflush(stdout);

        if (query_line[0] == '\n') {
            continue;
        }

        YY_BUFFER_STATE state;

        if (!(state = yy_scan_bytes(query_line.c_str(), strcspn(query_line.c_str(), "\n")))) {
            continue;
        }

        query::Query* ret;

        if (yyparse(&ret) == 0) {
            // printf("= %d\n", ret->type());
            ret->debug_print();
        }

        yy_delete_buffer(state);
    }
}
