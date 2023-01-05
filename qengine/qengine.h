#pragma once

#include "pagecache/pagecache.h"
#include "table/table.h"
#include <memory>
#include "src/datatype.h"
#include "filter.h"
#include "btree/btree.h"
#include "access_method.h"


const FileId CATALOG_COLUMNS_FILE_ID = FileId{1};
const PageId CATALOG_COLUMNS_PAGE_ID = PageId{CATALOG_COLUMNS_FILE_ID, 0};

const FileId CATALOG_TABLES_FILE_ID = FileId{2};
const PageId CATALOG_TABLES_PAGE_ID = PageId{CATALOG_TABLES_FILE_ID, 0};

std::shared_ptr<Table> get_table_ptr(std::shared_ptr<TableScheme> tableScheme);
std::shared_ptr<Table> create_table(std::shared_ptr<PageCache> pageCache,
                                    FileId fileId,
                                    std::shared_ptr<TableScheme> tableScheme,
                                    std::string tableName);

void insert_tuple(std::shared_ptr<PageCache> pageCache,
                  std::shared_ptr<Table> table,
                  std::shared_ptr<DenseTuple> data,
                  PageId pageId);

std::vector<DenseTuple> select_all(std::shared_ptr<PageCache> pageCache,
                                   std::shared_ptr<TableScheme>,
                                           FileId fileId, int lastPageId);

std::shared_ptr<DenseTuplesRepr> select(
        std::shared_ptr<PageCache> pageCache,
        Table table,
        datatypes::Expression*);
void init_db(std::shared_ptr<PageCache> pageCachePtr);
FileId next_file_id(std::shared_ptr<PageCache> pageCachePtr);

std::map<std::string, Table> get_tables(std::shared_ptr<PageCache> pageCachePtr);


std::pair<int, DenseTuple*> find_table_record_offset(std::shared_ptr<PageCache>, Table);
void initialize_btree(std::shared_ptr<PageCache>, Table, int);
void populate_btree(std::shared_ptr<PageCache> pageCachePtr, Table table, int fieldPos, std::byte*, std::byte*);
