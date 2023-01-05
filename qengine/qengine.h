#pragma once

#include "pagecache/pagecache.h"
#include "table/table.h"
#include <memory>
#include "src/datatype.h"
#include "filter.h"


const FileId CATALOG_COLUMNS_FILE_ID = FileId{1};
const PageId CATALOG_COLUMNS_PAGE_ID = PageId{CATALOG_COLUMNS_FILE_ID, 0};

const FileId CATALOG_TABLES_FILE_ID = FileId{2};
const PageId CATALOG_TABLES_PAGE_ID = PageId{CATALOG_TABLES_FILE_ID, 0};

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
        std::shared_ptr<TableScheme>,
        FileId fileId,
        int lastPageId,
        datatypes::Expression*);
void init_db(std::shared_ptr<PageCache> pageCachePtr);
FileId next_file_id(std::shared_ptr<PageCache> pageCachePtr);

std::map<std::string, Table> get_tables(std::shared_ptr<PageCache> pageCachePtr);

bool check_predicate(datatypes::Expression* expression, DenseTuple tuple);