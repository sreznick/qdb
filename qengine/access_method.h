#ifndef QDB_ACCESS_METHOD_H
#define QDB_ACCESS_METHOD_H

#include "table/table.h"
#include "src/datatype.h"

enum AccessMethod {
    SEQ_SCAN,
    BTREE
};

AccessMethod select_access_method(Table table, datatypes::Expression* expression);

#endif //QDB_ACCESS_METHOD_H
