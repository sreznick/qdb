#pragma once

#include "pagecache/pagecache.h"
#include "table/table.h"
#include <memory>


const FileId ORACLE_TABLE_FILE_ID = FileId{1};
const PageId ORACLE_TABLE_PAGE_ID = PageId{ORACLE_TABLE_FILE_ID, 0};

void commit(std::shared_ptr<PageCache> pageCachePtr);
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
                                           PageId pageId);

std::vector<DenseTuple> select(
        std::shared_ptr<Storage> pageCache,
        std::shared_ptr<TableScheme>,
        PageId pageId);
void init_db(std::shared_ptr<PageCache> pageCachePtr);

// TODO: implement with using datatypes namespace
std::map<std::string, TableScheme> get_tables(std::shared_ptr<PageCache> pageCachePtr);