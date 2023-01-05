#include "src/datatype.h"
#include "table/table.h"

#ifndef QDB_FILTER_H
#define QDB_FILTER_H

std::shared_ptr<std::vector<DenseTuple>> filter_tuples(
        datatypes::Expression* expression,
        std::vector<DenseTuple> tuple);

bool check_predicate(datatypes::Expression* expression, DenseTuple tuple);

#endif //QDB_FILTER_H
