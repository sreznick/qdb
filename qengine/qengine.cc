#include "qengine.h"
#include "pagecache/pagecache.cc"
#include "common/common.cc"
#include "table/types.h"
#include "src/datatype.h"

void commit(std::shared_ptr<PageCache> pageCachePtr) {
    pageCachePtr.get()->sync();
}

std::shared_ptr<Table> get_table_ptr(std::shared_ptr<TableScheme> tableScheme) {
    auto table = Table{"users", tableScheme, FileId{2}};

    return std::make_shared<Table>(table);
}

void init_db(std::shared_ptr<PageCache> pageCachePtr) {
     create_oracle_table(pageCachePtr);
}

std::shared_ptr<Table> create_table(std::shared_ptr<PageCache> pageCachePtr,
                                    FileId fileId, std::shared_ptr<TableScheme> tableSchemePtr,
                                    std::string tableName) {
    auto pageCache = pageCachePtr.get();
    auto tableScheme = tableSchemePtr.get();

    // Insert record into oracle table
    std::byte* page = pageCache->read_page(ORACLE_TABLE_PAGE_ID);
    PageMeta* pageMeta = read_page_meta(page);
    // print_page_meta(*pageMeta);

    std::shared_ptr<TableScheme> oracleSchemePtr = get_columns_table_scheme();
    auto columnsTableScheme = oracleSchemePtr.get();
    std::cout << "count: " << tableScheme->columnsCount() << std::endl;


    for (int i = 0; i < tableScheme->columnsCount(); i++) {
        int size = 0;
        if (!TypesRegistry::hasFixedSize(tableScheme->typeTag(i))) {
            size = tableScheme->fieldSize(i);
        }
        DenseTuple denseTuple = DenseTuple(oracleSchemePtr);
        denseTuple.setChar(0, tableName);
        denseTuple.setChar(1, tableScheme->name(i));
        denseTuple.setInt(2, tableScheme->typeTag(i));
        denseTuple.setInt(3, size);

        auto denseTuplePtr = std::make_shared<DenseTuple>(denseTuple);

        insert_table_tuple(pageCachePtr, ORACLE_TABLE_PAGE_ID, denseTuplePtr);
    }

    // Create file for new table
    auto pageId = pageCache->create_page(fileId);
    page = pageCache->read_page(pageId);
    pageMeta = new PageMeta{PAGE_META_SIZE, DEFAULT_PAGE_SIZE, 0};
    write_page_meta(page, *pageMeta);

    int result = pageCache->write(pageId, page);
    if (result < 0) {
        std::cout << "FAIL" << std::endl;
        exit(EXIT_FAILURE);
    }


    pageCache->sync();

    return std::shared_ptr<Table>(nullptr);

}

void insert_tuple(std::shared_ptr<PageCache> pageCachePtr,
                  std::shared_ptr<Table> table,
                  std::shared_ptr<DenseTuple> data,
                  PageId pageId) {

    insert_table_tuple(pageCachePtr, pageId, data);
    pageCachePtr.get()->sync();
}

std::map<std::string, TableScheme> get_tables(std::shared_ptr<PageCache> pageCachePtr) {
    std::map<std::string, TableScheme> registry;
    std::map<std::string, std::vector<ColumnScheme>*> heap;

    auto columns = select_all(pageCachePtr, get_columns_table_scheme(), ORACLE_TABLE_PAGE_ID);

    for (int i = 0; i < columns.size(); i++) {
       auto column = columns.at(i);
       std::string tableName = column.getChar(0);
       tableName.erase(std::find(tableName.begin(), tableName.end(), '\0'), tableName.end());
       std::string name = column.getChar(1);
       int typeId = column.getInt(2);
       int size = column.getInt(3);

       if (heap.find(tableName) == heap.end()) {
           heap.insert(std::pair<std::string, std::vector<ColumnScheme>*>(tableName, new std::vector<ColumnScheme>()));
       }
       auto tableColumns = heap.at(tableName);
       tableColumns->push_back(ColumnScheme(name, TypesRegistry::typeTag(typeId), size));
    }

    std::map<std::string, std::vector<ColumnScheme>*>::iterator iter;

    for (iter = heap.begin(); iter != heap.end(); iter++) {
        auto f = iter->first;
        TableScheme s = TableScheme(std::make_shared<std::vector<ColumnScheme>>(*iter->second));
        registry.insert(std::pair<std::string, TableScheme>(f, s));
    }

    return registry;
}

std::vector<DenseTuple> select_all(std::shared_ptr<PageCache> pageCachePtr, std::shared_ptr<TableScheme> tableSchemePtr, PageId pageId) {
     auto pageCache = pageCachePtr.get();
     auto tableScheme = tableSchemePtr.get();

     std::vector<DenseTuple> tuples;
     std::byte* page = pageCache->read_page(pageId);

     PageMeta* pageMeta = read_page_meta(page);

     std::vector<DenseTuple> denseTuples;

     for (int i = 0; i < pageMeta->tuplesCount; i++) {
         int size = 0;
         int offset = 0;
         ::memcpy(&size, page + PAGE_META_SIZE + 8 * i, 4);
         ::memcpy(&offset, page + PAGE_META_SIZE + 8 * i + 4, 4);

         denseTuples.push_back(DenseTuple(tableSchemePtr, page + offset));
     }

     return denseTuples;
}

std::vector<DenseTuple> select(
        std::shared_ptr<Storage> pageCachePtr,
        std::shared_ptr<TableScheme> tableSchemePtr,
        PageId pageId,
        datatypes::Expression* exps) {

    return std::vector <DenseTuple>{};
}
