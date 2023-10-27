#pragma once

#include "pagecache/pagecache.h"
#include "table/table.h"
#include "index/index.h"
#include "index/btree.h"
#include <memory>

#define M_TABLES_FILE_ID 1
#define M_COLUMNS_FILE_ID 2

#define MAX_NAME_LENGTH 50

void init_m_tables(std::shared_ptr<PageCache> pageCache);

std::shared_ptr<TableScheme> get_m_tables_scheme();
std::shared_ptr<TableScheme> get_m_columns_scheme();

std::shared_ptr<Table> create_table(std::shared_ptr<PageCache> pageCache,
                                    FileId fileId, std::string name,
                                    std::shared_ptr<TableScheme> tableScheme);

int insert_tuple(std::shared_ptr<PageCache> pageCache,
                  std::shared_ptr<Table> table,
                  std::shared_ptr<DenseTuple> data);

std::vector<DenseTuple> select_all(std::shared_ptr<PageCache> pageCache, std::shared_ptr<Table> table);

std::vector<DenseTuple> select(std::shared_ptr<Storage> pageCache, std::shared_ptr<Table> table);

void create_index(std::shared_ptr<PageCache> pageCache,
                  std::shared_ptr<Table> table,
                  std::string fieldName,
                  IndexType indexType);

