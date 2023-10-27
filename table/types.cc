#include "types.h"

std::array<TypeInfo, 7> TypesRegistry::_data = {
    TypeInfo{0, TypeTag::INT, "INT", 4},
    TypeInfo{1, TypeTag::LONG, "LONG", 8},
    TypeInfo{2, TypeTag::DOUBLE, "DOUBLE", 8},
    TypeInfo{3, TypeTag::BOOL, "BOOL", 1},
    TypeInfo{4, TypeTag::CHAR, "CHAR", 0},
    TypeInfo{5, TypeTag::VARCHAR, "VARCHAR", 0},
    TypeInfo{6, TypeTag::TEXT, "TEXT", 0},
};

