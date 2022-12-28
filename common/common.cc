#include <table/table.h>
#include "common.h"


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

void print_page_meta(PageMeta pageMeta) {
    std::cout << "left: " << pageMeta.pointerLeft << std::endl;
    std::cout << "right: " << pageMeta.pointerRight << std::endl;
    std::cout << "tuples count: " << pageMeta.tuplesCount << std::endl;
}


void create_oracle_table(std::shared_ptr<PageCache> pageCachePtr) {
    // Create columns scheme
    std::vector<ColumnScheme> columnsVector;

    ColumnScheme columnScheme_id = ColumnScheme("name", TypeTag::CHAR, 8);
    ColumnScheme columnScheme_name = ColumnScheme("type", TypeTag::CHAR, 8);

    columnsVector.push_back(columnScheme_id);
    columnsVector.push_back(columnScheme_name);

    std::shared_ptr<std::vector<ColumnScheme>> ptr = std::make_shared<std::vector<ColumnScheme>>(columnsVector);

    // Create table scheme
    TableScheme tableScheme = TableScheme(ptr);
    std::shared_ptr<TableScheme> tableSchemePtr = std::make_shared<TableScheme>(tableScheme);



    // Main
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

PageMeta* read_page_meta(std::byte* data) {
    std::int32_t left, right, tuplesCount;

    ::memcpy(&left, data, 4);
    ::memcpy(&right, data + 4, 4);
    ::memcpy(&tuplesCount, data + 8, 4);

    return new PageMeta{left, right, tuplesCount};
}

void write_page_meta(std::byte* dest, PageMeta pageMeta) {
    memcpy(dest, &pageMeta.pointerLeft, sizeof pageMeta.pointerLeft);
    memcpy(dest + sizeof pageMeta.pointerLeft, &pageMeta.pointerRight, sizeof pageMeta.pointerRight);
    memcpy(dest + sizeof pageMeta.pointerLeft + sizeof pageMeta.pointerRight, &pageMeta.tuplesCount, sizeof pageMeta.tuplesCount);
}


void insert_table_tuple(std::shared_ptr<PageCache> pageCachePtr, PageId pageId, std::shared_ptr<DenseTuple> denseTuplePtr) {
    PageCache* pageCache = pageCachePtr.get();
    DenseTuple* tuple = denseTuplePtr.get();

    std::byte* page = pageCache->read_page(pageId);
    PageMeta* pageMeta = read_page_meta(page);

    std::int32_t tupleLength = tuple->getTotalSize();
    std::int32_t tupleOffset = pageMeta->pointerRight - tupleLength;

    memcpy(page + pageMeta->pointerLeft, &tupleLength, sizeof tupleLength);
    memcpy(page + pageMeta->pointerLeft + sizeof tupleLength, &tupleOffset, sizeof tupleOffset);
    memcpy(page + pageMeta->pointerRight - tupleLength, tuple->getData(), tupleLength);

    pageMeta->pointerLeft += 8;
    pageMeta->pointerRight -= tuple->getTotalSize();
    pageMeta->tuplesCount += 1;

    write_page_meta(page, *pageMeta);
    int result = pageCache->write(pageId, page);
    if (result < 0) {
        std::cout << "ERROR: pageCache#write failed" << std::endl;
        exit(EXIT_FAILURE);
    }
}