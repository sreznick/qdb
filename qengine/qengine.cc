#include "qengine.h"
#include "pagecache/pagecache.cc"
#include "common/common.cc"
#include "table/types.h"
#include "src/datatype.h"

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
    denseTuple.setInt(2, 0);
    denseTuple.setInt(3, 0);
    denseTuple.setChar(4, tableName);

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

    int offset = insert_table_tuple(pageCachePtr, pageId, data);
    if (table.get()->has_index()) { update_index(pageCachePtr, table, data, pageId, offset); }
    pageCachePtr.get()->sync();
}

void update_index(std::shared_ptr<PageCache> pageCachePtr,
                  std::shared_ptr<Table> tablePtr,
                  std::shared_ptr<DenseTuple> data,
                  PageId pageId,
                  int offset) {
    auto pageCache = pageCachePtr.get();
    auto table = tablePtr.get();
    auto tuple = data.get();

    auto metaPage = pageCache->read_page({{table->indexFileId()}, 0});
    auto rootPage = pageCache->read_page({{table->indexFileId()}, 1});

    auto btree = BTree(metaPage, rootPage, pageCachePtr, table->indexFileId());
    btree.insert({tuple->getInt(table->indexFieldPos()), offset});
    pageCache->write({{table->indexFileId()}, 0}, metaPage);
    pageCache->write({{table->indexFileId()}, 1}, rootPage);
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
        std::string table_name = tuple->getChar(4);
        char name = *table_name.c_str();
        auto tableMeta = new TableMeta{tuple->getInt(0), tuple->getInt(1), tuple->getInt(2), tuple->getInt(3), name};


        if (tableMetaVector.find(table_name) == tableMetaVector.end()) {
            tableMetaVector.insert(std::pair<std::string, TableMeta*>(table_name, tableMeta));
        }

    }

    std::map<std::string, std::vector<ColumnScheme>*>::iterator iter;

    for (iter = heap.begin(); iter != heap.end(); iter++) {
        auto name = iter->first;
        TableScheme s = TableScheme(std::make_shared<std::vector<ColumnScheme>>(*iter->second));
        auto tableMeta = tableMetaVector.at(name);
        Table table = Table(name, std::make_shared<TableScheme>(s), {tableMeta->fileId}, tableMeta->lastPageId, tableMeta->indexFileId, tableMeta->indexFieldPos);
        registry.insert(std::pair<std::string, Table>(name, table));
    }

    return registry;
}

// always uses seq scan
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

std::vector<std::pair<DenseTuple, int>> _select_all_with_offset(
        std::shared_ptr<PageCache> pageCachePtr,
        std::shared_ptr<TableScheme> tableSchemePtr,
        FileId fileId, int lastPageId) {
    auto pageCache = pageCachePtr.get();
    auto tableScheme = tableSchemePtr.get();

    std::vector<std::pair<DenseTuple, int>> tuples;

    for (int j = 0; j <= lastPageId; j++) {
        std::byte *page = pageCache->read_page({{fileId}, j});
        PageMeta *pageMeta = read_page_meta(page);

        for (int i = 0; i < pageMeta->tuplesCount; i++) {
            int size = 0;
            int offset = 0;
            ::memcpy(&size, page + PAGE_META_SIZE + 8 * i, 4);
            ::memcpy(&offset, page + PAGE_META_SIZE + 8 * i + 4, 4);

            tuples.push_back(std::make_pair(DenseTuple(tableSchemePtr, page + offset), offset + PAGE_META_SIZE * j));
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
        Table table,
        datatypes::Expression* whereExpr) {

    auto pageCache = pageCachePtr.get();
    auto lastPageId = table.lastPageId().id;
    auto fileId = table.fileId();
    auto tableSchemePtr = table.scheme();
    auto scheme = tableSchemePtr.get();

    auto access_method = select_access_method(table, whereExpr);

    if (access_method == AccessMethod::BTREE) {
        std::cout << "[DEBUG]: Using BTree access method" << std::endl;

        auto indexFileId = table.indexFileId();
        std::int32_t depth, rootPageId;
        std::byte* metaPage = pageCache->read_page({indexFileId, 0});
        memcpy(&depth, metaPage, 4);
        memcpy(&rootPageId, metaPage + 4, 4);
        std::byte* rootPage = pageCache->read_page({{indexFileId}, rootPageId});
        auto btree = BTree(metaPage, rootPage, pageCachePtr, indexFileId);

        int value = atoi(whereExpr->right->value->value.c_str());
        auto record = btree.find(value);
        std::vector<DenseTuple> tuples;
        if (record.pointer > 0) {
            int pageId = record.pointer / DEFAULT_PAGE_SIZE;
            std::byte* page = pageCache->read_page({{table.fileId()}, pageId});

            auto tuple = DenseTuple(tableSchemePtr, page + record.pointer);
            tuples.push_back(tuple);
        }

        return std::make_shared<DenseTuplesRepr>(DenseTuplesRepr(tuples, tableSchemePtr));
    } else {
        auto tuples = select_all(pageCachePtr, tableSchemePtr, fileId, lastPageId);
        std::vector<DenseTuple> filteredTuples = *filter_tuples(whereExpr, tuples).get();

        auto tuplesRepr = DenseTuplesRepr(filteredTuples, tableSchemePtr);
        auto tuplesReprPtr = std::make_shared<DenseTuplesRepr>(tuplesRepr);
        return tuplesReprPtr;
    }
}

// BTREE


std::pair<int, DenseTuple*> find_table_record_offset(std::shared_ptr<PageCache> pageCachePtr, Table table) {
    auto pageCache = pageCachePtr.get();
    auto tableSchemePtr = get_tables_table_scheme();

    // FIXME: DRY
    auto page = pageCache->read_page(CATALOG_TABLES_PAGE_ID);
    PageMeta *pageMeta = read_page_meta(page);
    for (int i = 0; i < pageMeta->tuplesCount; i++) {
        int size = 0;
        int offset = 0;
        ::memcpy(&size, page + PAGE_META_SIZE + 8 * i, 4);
        ::memcpy(&offset, page + PAGE_META_SIZE + 8 * i + 4, 4);

        auto tuple = new DenseTuple(tableSchemePtr, page + offset);
        auto table_name = tuple->getChar(4);
        if (table_name == table.name()) {
            return std::make_pair(offset, tuple);
        }
    }

    return std::make_pair(-1, nullptr);
}

void initialize_btree(std::shared_ptr<PageCache> pageCachePtr, Table table, int fieldPos) {
    auto pageCache = pageCachePtr.get();

    auto page = pageCache->read_page(CATALOG_TABLES_PAGE_ID);
    auto pair = find_table_record_offset(pageCachePtr, table);
    auto offset = pair.first;
    if (offset < 0) {
        std::cout << "ERROR: Invariant violation" << std::endl;
        exit(EXIT_FAILURE);
    }
    auto tuple = pair.second;
    auto indexFileId = next_file_id(pageCachePtr);
    tuple->setInt(2, indexFileId.id);
    tuple->setInt(3, fieldPos); // ?
    memcpy(page + offset, tuple->getData(), tuple->getTotalSize());
    pageCache->write(CATALOG_TABLES_PAGE_ID, page);

    auto metaPageId = pageCache->create_page(indexFileId);
    auto metaPage = pageCache->read_page(metaPageId);

    std::int32_t depth = 0;
    std::int32_t fid = 1;
    memcpy(metaPage, &depth, 4);
    memcpy(metaPage + 4, &fid, 4); // root page ID
    pageCache->write(metaPageId, metaPage);
    pageCache->sync();

    // create root page
    auto rootPageId = pageCache->create_page(indexFileId);
    auto rootPage = pageCache->read_page(rootPageId);
    pageCache->write(rootPageId, rootPage);

    populate_btree(pageCachePtr, table, fieldPos, metaPage, rootPage);
    pageCache->write(rootPageId, rootPage);

    pageCache->sync();
}

void populate_btree(std::shared_ptr<PageCache> pageCachePtr, Table table, int fieldPos, std::byte* metaPage, std::byte* rootPage) {
    auto tuples = _select_all_with_offset(pageCachePtr, table.scheme(), table.fileId(), table.lastPageId().id);
    auto btree = BTree(metaPage, rootPage, pageCachePtr, table.indexFileId());

    for (int i = 0; i < tuples.size(); i++) {
        auto tuple = tuples.at(i);
        btree.insert({tuple.first.getInt(fieldPos), tuple.second});
    }
}