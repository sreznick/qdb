#pragma once

#include "pagecache/pagecache.h"
#include "table/table.h"
#include <memory>

std::shared_ptr<Table> create_table(std::shared_ptr<PageCache> pageCache,
                                    std::shared_ptr<TableScheme> tableScheme);

void insert_tuple(std::shared_ptr<PageCache> pageCache,
                  std::shared_ptr<Table> table,
                  std::shared_ptr<DenseTuple> data);

std::vector<DenseTuple> select_all(std::shared_ptr<PageCache> pageCache, std::shared_ptr<Table>);

std::vector<DenseTuple> select(std::shared_ptr<Storage> pageCache, std::shared_ptr<Table>);

