#include "types.h"

std::array<TypeInfo, 8> TypesRegistry::_data = {
    TypeInfo{0, TypeTag::UNKNOWN, "UNKNOWN", -1},
    TypeInfo{1, TypeTag::INT, "INT", 4},
    TypeInfo{2, TypeTag::LONG, "LONG", 8},
    TypeInfo{3, TypeTag::DOUBLE, "DOUBLE", 8},
    TypeInfo{4, TypeTag::BOOL, "BOOLEAN", 1},
    TypeInfo{5, TypeTag::CHAR, "CHAR", 0},
    TypeInfo{6, TypeTag::VARCHAR, "VARCHAR", 0},
    TypeInfo{7, TypeTag::TEXT, "TEXT", 0},
};

