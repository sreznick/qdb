#include "qengine.h"
#include "pagecache/pagecache.cc"
#include "common/common.cc"
#include "table/types.h"
#include "src/datatype.h"

void commit(std::shared_ptr<PageCache> pageCachePtr) {
    pageCachePtr.get()->sync();
}

std::shared_ptr<Table> get_table_ptr(std::shared_ptr<TableScheme> tableScheme) {
    auto table = Table{"users", tableScheme, FileId{2}, 0};

    return std::make_shared<Table>(table);
}

void init_db(std::shared_ptr<PageCache> pageCachePtr) {
    create_columns_table(pageCachePtr);
    create_tables_table(pageCachePtr);
}

std::shared_ptr<Table> create_table(std::shared_ptr<PageCache> pageCachePtr,
                                    FileId fileId, std::shared_ptr<TableScheme> tableSchemePtr,
                                    std::string tableName) {
    auto pageCache = pageCachePtr.get();
    auto tableScheme = tableSchemePtr.get();

    // Insert record into oracle table
    std::byte* page = pageCache->read_page(CATALOG_COLUMNS_PAGE_ID);
    PageMeta* pageMeta = read_page_meta(page);

    std::shared_ptr<TableScheme> oracleSchemePtr = get_columns_table_scheme();
    auto columnsTableScheme = oracleSchemePtr.get();

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

        insert_table_tuple(pageCachePtr, CATALOG_COLUMNS_PAGE_ID, denseTuplePtr);
    }

    // insert table metarecord
    auto tablesTableSchemePtr = get_tables_table_scheme();
    DenseTuple denseTuple = DenseTuple(tablesTableSchemePtr);
    denseTuple.setInt(0, fileId.id);
    denseTuple.setInt(1, 0);
    denseTuple.setChar(2, tableName);

    auto denseTuplePtr = std::make_shared<DenseTuple>(denseTuple);
    insert_table_tuple(pageCachePtr, CATALOG_TABLES_PAGE_ID, denseTuplePtr);

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

std::map<std::string, Table> get_tables(std::shared_ptr<PageCache> pageCachePtr) {
    std::map<std::string, Table> registry;
    std::map<std::string, std::vector<ColumnScheme>*> heap;
    std::map<std::string, TableMeta*> tableMetaVector;

    auto columns = select_all(pageCachePtr, get_columns_table_scheme(), CATALOG_COLUMNS_FILE_ID, 0);
    auto tables = select_all(pageCachePtr, get_tables_table_scheme(), CATALOG_TABLES_FILE_ID, 0);

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

    for (auto tuple = tables.rbegin(); tuple != tables.rend(); ++tuple) {
        char name = *tuple->getChar(2).c_str();
        auto tableMeta = new TableMeta{tuple->getInt(0), tuple->getInt(1), name};

        if (tableMetaVector.find(tuple->getChar(2)) == tableMetaVector.end()) {
            tableMetaVector.insert(std::pair<std::string, TableMeta*>(tuple->getChar(2), tableMeta));
        }

    }

    std::map<std::string, std::vector<ColumnScheme>*>::iterator iter;

    for (iter = heap.begin(); iter != heap.end(); iter++) {
        auto name = iter->first;
        TableScheme s = TableScheme(std::make_shared<std::vector<ColumnScheme>>(*iter->second));
        auto tableMeta = tableMetaVector.at(name);
        Table table = Table(name, std::make_shared<TableScheme>(s), {tableMeta->fileId}, tableMeta->lastPageId);
        registry.insert(std::pair<std::string, Table>(name, table));
    }

    return registry;
}

std::vector<DenseTuple> select_all(std::shared_ptr<PageCache> pageCachePtr, std::shared_ptr<TableScheme> tableSchemePtr, FileId fileId, int lastPageId) {
     auto pageCache = pageCachePtr.get();
     auto tableScheme = tableSchemePtr.get();

     std::vector<DenseTuple> tuples;

     for (int j = 0; j <= lastPageId; j++) {
         std::byte *page = pageCache->read_page({{fileId}, j});
         PageMeta *pageMeta = read_page_meta(page);

         for (int i = 0; i < pageMeta->tuplesCount; i++) {
             int size = 0;
             int offset = 0;
             ::memcpy(&size, page + PAGE_META_SIZE + 8 * i, 4);
             ::memcpy(&offset, page + PAGE_META_SIZE + 8 * i + 4, 4);

             tuples.push_back(DenseTuple(tableSchemePtr, page + offset));
         }
     }

     return tuples;
}

FileId next_file_id(std::shared_ptr<PageCache> pageCachePtr) {
    auto pageCache = pageCachePtr.get();
    auto allTables = get_tables(pageCachePtr);

    int result = 2;
    std::map<std::string, Table>::iterator iter;

    for (iter = allTables.begin(); iter != allTables.end(); iter++) {
        if (iter->second.fileId().id > result) result = iter->second.fileId().id;
    }

    return FileId{result + 1};
}

std::shared_ptr<DenseTuplesRepr> select(
        std::shared_ptr<PageCache> pageCachePtr,
        std::shared_ptr<TableScheme> tableSchemePtr,
        FileId fileId,
        int lastPageId,
        datatypes::Expression* whereExpr) {

    auto tuples = select_all(pageCachePtr, tableSchemePtr, fileId, lastPageId);
    std::vector<DenseTuple> filteredTuples = *filter_tuples(whereExpr, tuples).get();

    auto tuplesRepr = DenseTuplesRepr(filteredTuples, tableSchemePtr);
    auto tuplesReprPtr = std::make_shared<DenseTuplesRepr>(tuplesRepr);
    return tuplesReprPtr;
}

