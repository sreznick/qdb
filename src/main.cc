#include <stdio.h>
#include <fstream>

#include "storage/storage.h"
#include "table/table.h"
#include "pagecache/pagecache.h"

#include "query.h"
#include "lexer.h"
#include "parser.h"

void init();
void example();
void prompt();
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

    if (std::string("init") == argv[1]) {
        init();
        return 0;
    }

    if (std::string("example") == argv[1]) {
        example();
        return 0;
    }

    if (std::string("prompt") == argv[1]) {
        prompt();
        return 0;
    }

    if (std::string("file") == argv[1] && argc > 3) {
        file_prompt(argv[3]);
    if (std::string("test") == argv[1]) {
        PageCache page_cache {storage, {1, 3}};
        page_cache.test();
        std::cerr << "Tests passed!" << std::endl;
    }

    return 0;
}


void init() {
    // TODO: реализовать инициализацию базы
    // 
    // В предположении, что папка пустая, нужно создать 
    // две мета-таблицы
    //
    // В одной будут храниться таблицы, в другой - поля
    //  
    // m_tables 
    //   INT qid
    //   VARCHAR(50) name 
    //
    // m_columns 
    //   INT qid
    //   INT table_qid
    //   VARCHAR(50) name
    //   INT type_id - id-поле из структуры TypeInfo
    //
    //
    //  Данные метатаблиц тоже должны присутствовать в метатаблицах
    // 
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

void prompt() {
/*
 *  TODO 
 * 
 *    - добавить логику из прошлого ДЗ с разбором запросов
 *    - вызывать функции из qengine для CREATE TABLE, INSERT, SELECT
 *    - для SELECT напечатать результат в виде простой таблицы
 *    - для CREATE TABLE напечать 'table <name> created' 
 *    - для INSERT гапечатать количество записей после добавления
 *
 */
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

}

void file_prompt(std::string filename) {
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
