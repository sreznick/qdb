#include "table/table.h"

#define BOOL_STR(b) ((b)?"TRUE":"FALSE")

void pretty_print_header(std::shared_ptr<TableScheme> tableSchemePtr) {
    auto scheme = tableSchemePtr.get();

    for (int i = 0; i < scheme->columnsCount(); i++) {
        std::cout << scheme->name(i) << " (" << scheme->typeName(i) << ")\t";
    }
    std::cout << std::endl;
}

void pretty_print_tuple(DenseTuple* tuple) {
    auto scheme = tuple->getScheme();

    for (int i = 0; i < scheme->columnsCount(); i++) {
        if (scheme->typeName(i) == "INT") {
            std::cout << std::to_string(tuple->getInt(i)) << "\t";
        } else if (scheme->typeName(i) == "CHAR") {
            std::cout << tuple->getChar(i) << "\t";
        } else if (scheme->typeName(i) == "BOOLEAN") {
            std::cout << BOOL_STR(tuple->getBool(i)) << "\t";
        } else if (scheme->typeName(i) == "DOUBLE") {
            std::cout << tuple->getDouble(i) << "\t";
        } else if (scheme->typeName(i) == "VARCHAR") {
            std::cout << tuple->getChar(i) << "\t";
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
    if (!tuplesWrapper->tuples().empty()) {
        pretty_print_header(tuplesWrapper->tuples().at(0).getScheme());
    }
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