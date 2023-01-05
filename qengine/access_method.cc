#include "access_method.h"

AccessMethod select_access_method(Table table, datatypes::Expression* expression) {
    if (table.indexFileId().id <= 0) return AccessMethod::SEQ_SCAN;
    if (expression == nullptr) return AccessMethod::SEQ_SCAN;

    auto scheme = table.scheme().get();

    if (*expression->operation == "=") {
        if (*expression->left->operation == "variable" && *expression->right->operation == "const") {
            if (scheme->typeName(expression->left->value->value) == "INT") return AccessMethod::BTREE;
        }
    }
    return AccessMethod::SEQ_SCAN;
}