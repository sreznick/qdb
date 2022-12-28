#include "table/table.h"

#define BOOL_STR(b) ((b)?"TRUE":"FALSE")

void pretty_print_tuple(DenseTuple* tuple) {
    auto scheme = tuple->getScheme();

    for (int i = 0; i < scheme->columnsCount(); i++) {
        if (scheme->typeName(i) == "INT") {
            std::cout << std::to_string(tuple->getInt(i)) << " (INT)\t";
        } else if (scheme->typeName(i) == "CHAR") {
            std::cout << tuple->getChar(i) << " (CHAR)\t";
        } else if (scheme->typeName(i) == "BOOLEAN") {
            std::cout << BOOL_STR(tuple->getBool(i)) << " (BOOLEAN)\t";
        } else if (scheme->typeName(i) == "DOUBLE") {
            std::cout << tuple->getDouble(i) << " (DOUBLE)\t";
        } else if (scheme->typeName(i) == "VARCHAR") {
            std::cout << tuple->getChar(i) << " (VARCHAR)\t";
        }  else {
            std::cout << "(UNKNOWN) ";
        }
    }

    std::cout << std::endl;
}

void pretty_print_relation(std::shared_ptr<DenseTuplesRepr> denseTuplesReprPtr) {
    auto tuplesWrapper = denseTuplesReprPtr.get();

    std::cout << "RELATION: tuples: " << tuplesWrapper->tuples().size() << " , columns: " << tuplesWrapper->visible_count() << std::endl;
    std::cout << "----" << std::endl;
    for (int i = 0; i < tuplesWrapper->tuples().size(); i++) {
        pretty_print_tuple(&tuplesWrapper->tuples().at(i));
    }
    std::cout << "----" << std::endl;
}

void pretty_print_table_scheme(TableScheme tableScheme) {
    std::cout << "Table Scheme:" << std::endl;
    for (int i = 0; i < tableScheme.columnsCount(); i++) {
        std::cout << "name: " << tableScheme.name(i) << " type: " << tableScheme.typeName(i) << " size: " << tableScheme.fieldSize(i) << std::endl;
    }
}