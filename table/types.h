#pragma once

#include <array>
#include <string>

#include <cctype>
#include "types.h"

/*
 *  Short values corresponding with base SQL types 
 */
enum TypeTag {UNKNOWN, INT, LONG, DOUBLE, BOOL,
              CHAR, VARCHAR, TEXT};


/*
 * More details about base SQL types
 */
struct TypeInfo {
    int typeId;        // hard-coded id to be used in meta table
                       // describing field types
    TypeTag tag;       // mnemonic tag to be used in C++ code
    std::string name;  // mnemonic name for use representation
    int fixedSize;     // 0 for types without fixed size,
                       // positive value for fixed-size types
};


/*
 * Class providing mapping between tag and id
 * ways of referencing type
 */
class TypesRegistry {
private:
    static std::array<TypeInfo, 8> _data;
public:
    static TypeInfo byId(int typeId) {
        return _data[typeId];
    }

    static TypeTag typeTag(int typeId) {
        return byId(typeId).tag;
    }

    static TypeTag typeTagByName(std::string name) {
        for (int i = 0; i < _data.size(); i++) {
            transform(name.begin(), name.end(), name.begin(), ::toupper);
            if (_data[i].name == name) {
                return _data[i].tag;
            }
        }

        return TypeTag::UNKNOWN;
    }

    static int fixedSize(int typeId) {
        return byId(typeId).fixedSize;
    }

    static int hasFixedSize(int typeId) {
        return byId(typeId).fixedSize != 0;
    }

    static std::string name(int typeId) {
        return byId(typeId).name;
    }

    static TypeInfo byTag(TypeTag typeTag) {
        for (int i = 0; i < _data.size(); ++i) {
            if (_data[i].tag == typeTag) return _data[i];
        }
        return {};
    }

    static int typeId(TypeTag typeTag) {
        return byTag(typeTag).typeId;
    }

    static int fixedSize(TypeTag typeTag) {
        return byTag(typeTag).fixedSize;
    }

    static int hasFixedSize(TypeTag typeTag) {
        return byTag(typeTag).fixedSize != 0;
    }

    static std::string name(TypeTag typeTag) {
        return byTag(typeTag).name;
    }
};

