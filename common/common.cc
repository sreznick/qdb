#include <table/table.h>
#include "common.h"
#include "qengine/qengine.h"

// FIXME: memoize scheme once
std::shared_ptr<TableScheme> get_columns_table_scheme() {
    ColumnScheme columnScheme_tableName = ColumnScheme("table_name", TypeTag::CHAR, 16);
    ColumnScheme columnScheme_name = ColumnScheme("name", TypeTag::CHAR, 16);
    ColumnScheme columnScheme_typeId = ColumnScheme("type_id", TypeTag::INT);
    ColumnScheme columnScheme_size = ColumnScheme("size", TypeTag::INT);

    std::vector<ColumnScheme> columnsVector;
    columnsVector.push_back(columnScheme_tableName);
    columnsVector.push_back(columnScheme_name);
    columnsVector.push_back(columnScheme_typeId);
    columnsVector.push_back(columnScheme_size);

    std::shared_ptr<std::vector<ColumnScheme>> ptr = std::make_shared<std::vector<ColumnScheme>>(columnsVector);

    TableScheme tableScheme = TableScheme(ptr);

    return std::make_shared<TableScheme>(tableScheme);
}

// FIXME: memoize scheme once
std::shared_ptr<TableScheme> get_tables_table_scheme() {
    ColumnScheme columnScheme_fileId = ColumnScheme("file_id", TypeTag::INT);
    ColumnScheme columnScheme_lastPageId = ColumnScheme("last_page_id", TypeTag::INT);
    ColumnScheme columnScheme_indexFileId = ColumnScheme("index_fid", TypeTag::INT);
    ColumnScheme columnScheme_indexFieldPos = ColumnScheme("index_fpos", TypeTag::INT);
    ColumnScheme columnScheme_name = ColumnScheme("name", TypeTag::CHAR, 16);

    std::vector<ColumnScheme> columnsVector;
    columnsVector.push_back(columnScheme_fileId);
    columnsVector.push_back(columnScheme_lastPageId);
    columnsVector.push_back(columnScheme_indexFileId);
    columnsVector.push_back(columnScheme_indexFieldPos);
    columnsVector.push_back(columnScheme_name);

    std::shared_ptr<std::vector<ColumnScheme>> ptr = std::make_shared<std::vector<ColumnScheme>>(columnsVector);

    TableScheme tableScheme = TableScheme(ptr);

    return std::make_shared<TableScheme>(tableScheme);
}

void create_tables_table(std::shared_ptr<PageCache> pageCachePtr) {
    auto pageCache = pageCachePtr.get();
    PageId pageId = pageCache->create_page({2});

    auto page = pageCache->read_page(pageId);
    PageMeta pageMeta = PageMeta{PAGE_META_SIZE, DEFAULT_PAGE_SIZE, 0};
    write_page_meta(page, pageMeta);

    int result = pageCache->write(pageId, page);
    if (result < 0) {
        std::cout << "FAIL" << std::endl;
        exit(EXIT_FAILURE);
    }
    pageCache->sync();
}

void create_columns_table(std::shared_ptr<PageCache> pageCachePtr) {
    auto pageCache = pageCachePtr.get();
    PageId pageId = pageCache->create_page({1});
    auto page = pageCache->read_page(pageId);
    PageMeta pageMeta = PageMeta{PAGE_META_SIZE, DEFAULT_PAGE_SIZE, 0};
    write_page_meta(page, pageMeta);

    int result = pageCache->write(pageId, page);
    if (result < 0) {
        std::cout << "FAIL" << std::endl;
        exit(EXIT_FAILURE);
    }
    pageCache->sync();
}

void print_page_meta(PageMeta pageMeta) {
    std::cout << "left: " << pageMeta.pointerLeft << std::endl;
    std::cout << "right: " << pageMeta.pointerRight << std::endl;
    std::cout << "tuples count: " << pageMeta.tuplesCount << std::endl;
}

void print_table_meta(TableMeta tableMeta) {
    std::cout << "name: " << tableMeta.name << std::endl;
    std::cout << "file ID: " << tableMeta.fileId << std::endl;
    std::cout << "last page ID: " << tableMeta.lastPageId << std::endl;
    std::cout << "index page ID: " << tableMeta.indexFileId << std::endl;
    if (tableMeta.indexFileId > 0) {
        std::cout << "index fiels pos: " << tableMeta.indexFieldPos << std::endl;
    }
}

PageMeta* read_page_meta(std::byte* data) {
    std::int32_t left, right, tuplesCount;

    ::memcpy(&left, data, 4);
    ::memcpy(&right, data + 4, 4);
    ::memcpy(&tuplesCount, data + 8, 4);

    return new PageMeta{left, right, tuplesCount};
}

TableMeta* read_table_meta(std::byte* data) {
    std::int32_t fileID, lastPageID, indexFileID, indexFieldPos;
    char* name[16];

    ::memcpy(&fileID, data, 4);
    ::memcpy(&lastPageID, data + 4, 4);
    ::memcpy(&indexFileID, data + 8, 4);
    ::memcpy(&indexFieldPos, data + 12, 4);
    ::memcpy(&name, data + 16, 16);

    // FIXME: table_name
    return new TableMeta{fileID, lastPageID, indexFileID, indexFieldPos, "users"};
}

void write_page_meta(std::byte* dest, PageMeta pageMeta) {
    memcpy(dest, &pageMeta.pointerLeft, sizeof pageMeta.pointerLeft);
    memcpy(dest + sizeof pageMeta.pointerLeft, &pageMeta.pointerRight, sizeof pageMeta.pointerRight);
    memcpy(dest + sizeof pageMeta.pointerLeft + sizeof pageMeta.pointerRight, &pageMeta.tuplesCount, sizeof pageMeta.tuplesCount);
}

void write_table_meta(std::byte* dest, TableMeta tableMeta) {
    memcpy(dest, &tableMeta.fileId, 4);
    memcpy(dest + 4, &tableMeta.lastPageId, 4);
    memcpy(dest + 8, &tableMeta.indexFileId, 4);
    memcpy(dest + 12, &tableMeta.indexFieldPos, 4);
    memcpy(dest + 16, &tableMeta.name, 16);
}

PageId allocate_new_page(std::shared_ptr<PageCache> pageCachePtr, FileId fileId) {
    auto pageCache = pageCachePtr.get();
    auto pageId = pageCache->create_page(fileId);

    auto page = pageCache->read_page(pageId);
    auto pageMeta = new PageMeta{PAGE_META_SIZE, DEFAULT_PAGE_SIZE, 0};
    write_page_meta(page, *pageMeta);

    auto allTables = get_tables(pageCachePtr);

    std::map<std::string, Table>::iterator iter;
    Table* target = nullptr;
    for (iter = allTables.begin(); iter != allTables.end(); iter++) {
        if (iter->second.fileId().id == fileId.id) {
            target = &iter->second;
        }
    }

    if (target == nullptr) {
        std::cout << "FATAL ERROR" << std::endl;
        exit(EXIT_FAILURE);
    }

    auto newLastPageId = PageId{{fileId}, target->lastPageId().id + 1};

    auto tablesTableSchemePtr = get_tables_table_scheme();
    DenseTuple denseTuple = DenseTuple(tablesTableSchemePtr);
    denseTuple.setInt(0, target->fileId().id);
    denseTuple.setInt(1, newLastPageId.id);
    denseTuple.setChar(2, target->name());

    auto denseTuplePtr = std::make_shared<DenseTuple>(denseTuple);
    insert_table_tuple(pageCachePtr, CATALOG_TABLES_PAGE_ID, denseTuplePtr);

    return newLastPageId;
}


int insert_table_tuple(std::shared_ptr<PageCache> pageCachePtr, PageId pageId, std::shared_ptr<DenseTuple> denseTuplePtr) {
    PageCache* pageCache = pageCachePtr.get();
    DenseTuple* tuple = denseTuplePtr.get();

    std::byte* page = pageCache->read_page(pageId);
    PageMeta* pageMeta = read_page_meta(page);

    std::int32_t tupleLength = tuple->getTotalSize();
    std::int32_t tupleOffset = pageMeta->pointerRight - tupleLength;

    if (pageMeta->pointerRight - pageMeta->pointerLeft < tupleLength + 8) {
        auto newPageId = allocate_new_page(pageCachePtr, pageId.fileId);
        return insert_table_tuple(pageCachePtr, newPageId, denseTuplePtr);
    }

    memcpy(page + pageMeta->pointerLeft, &tupleLength, sizeof tupleLength);
    memcpy(page + pageMeta->pointerLeft + sizeof tupleLength, &tupleOffset, sizeof tupleOffset);
    memcpy(page + pageMeta->pointerRight - tupleLength, tuple->getData(), tupleLength);

    pageMeta->pointerLeft += 8;
    pageMeta->pointerRight -= tupleLength;
    pageMeta->tuplesCount += 1;

    write_page_meta(page, *pageMeta);
    int result = pageCache->write(pageId, page);
    if (result < 0) {
        std::cout << "ERROR: pageCache#write failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    return pageMeta->pointerRight;
}

