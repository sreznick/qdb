#pragma once

#include <iostream>
#include <sstream>

#include <map>
#include <vector>
#include <memory>
#include <cinttypes>

#include <string.h>

#include "types.h"
#include "storage/fileid.h"

struct TuplePos {
    std::int32_t pageId;   // number of page inside table
    std::int32_t tupleId;  // number of tuple inside page
};

class ColumnScheme {
private:
    TypeTag _typeTag;
    int _size;
    std::string _name;

public:
    ColumnScheme(std::string name, TypeTag typeTag) {
        _name = name;
        _typeTag = typeTag;
        _size = 0;
    }

    ColumnScheme(std::string name, TypeTag typeTag, int size) {
        _name = name;
        _typeTag = typeTag;
        _size = size;
    }

    bool ok() {
        if (TypesRegistry::hasFixedSize(_typeTag) && _size != 0) {
            return false;
        }

        return true;
    }

    std::string name() {
        return _name;
    }

    int size() {
        return _size + TypesRegistry::fixedSize(_typeTag);
    }

    bool hasFixedSize() {
        return TypesRegistry::hasFixedSize(_typeTag);
    }  

    TypeTag typeTag() {
        return _typeTag;
    }

    std::string typeName() {
        return TypesRegistry::name(_typeTag);
    }

    int outputSize() {
        int valueSize = 20;
        switch (_typeTag) {
            case TypeTag::INT:
                valueSize = 10;
        }
        return std::max(valueSize, static_cast<int>(_name.size()));
    }
};

class TableScheme {
private:
    std::shared_ptr<std::vector<ColumnScheme>> _columns;
    std::map<std::string, ColumnScheme> _by_name;
    std::map<std::string, int> _pos_by_name;

public:
    TableScheme(std::shared_ptr<std::vector<ColumnScheme>> columns): _columns(columns) {
        for (int i = 0; i < _columns->size(); ++i) {
            ColumnScheme value = (*columns)[i];
            _by_name.emplace(name(i), value);
            _pos_by_name[name(i)] = i;
        }
    }

    int fieldSize(int i) {
        return (*_columns)[i].size();
    } 

    int outputSize(int i) {
        return (*_columns)[i].outputSize();
    }

    int totalSize() {
        int result = 0;
        for (int i = 0; i < _columns->size(); ++i) {
            result += fieldSize(i);
        }
        return result;
    }

    TypeTag typeTag(int i) {
        return (*_columns)[i].typeTag();
    }

    std::string name(int i) {
        return (*_columns)[i].name();
    }

    std::string typeName(int i) {
        return (*_columns)[i].typeName();
    }

    int columnsCount() {
        return _columns->size();
    }

    ColumnScheme column(std::string name) {
        return (*_columns)[_pos_by_name.find(name)->second];
    }

    std::string typeName(std::string name) {
        return column(name).typeName();
    }
};

class Table {
private:
    std::string _name;
    std::shared_ptr<TableScheme> _tableScheme;
    FileId _fileId;

public:
    Table(std::string name,
          std::shared_ptr<TableScheme> tableScheme,
          FileId fileId): _name(name), _tableScheme(tableScheme), _fileId(fileId) {
    }

    std::shared_ptr<TableScheme> scheme() {
        return _tableScheme;
    }

    FileId fileId() {
        return _fileId;
    }
};

class DenseTuple {
private:
    std::shared_ptr<TableScheme> _scheme;
    std::byte* _data;
    std::vector<int> _offsets;

public:
    DenseTuple(std::shared_ptr<TableScheme> scheme) : _scheme{scheme} {
        _data = new std::byte[scheme->totalSize()];
        _offsets.push_back(0);

        for (int i = 0; i < scheme->columnsCount() - 1; ++i) {
            int last = _offsets[_offsets.size() - 1];
            _offsets.push_back(last + scheme->fieldSize(i));
        }
        
    }

    ~DenseTuple() {
        delete[] _data;
    }

    void setInt(int fieldPos, std::int32_t value) {
        ::memcpy(_data + _offsets[fieldPos], &value, 4);
    }

    std::int32_t getInt(int fieldPos) {
        int result;
        ::memcpy(&result, _data + _offsets[fieldPos], 4);

        return result;
    }


    void setChar(int fieldPos, std::string value) {
        ::memcpy(_data + _offsets[fieldPos], value.c_str(), value.size());
    }

    std::string getChar(int fieldPos) {
        std::string result;
        for (int i = 0; i < _scheme->fieldSize(fieldPos); ++i) {
            result.push_back(static_cast<char>(_data[_offsets[fieldPos] + i]));
        }

        return result;
    }

    std::string as_string(int fieldPos) {
        std::stringstream ss;
        switch (_scheme->typeTag(fieldPos)) {
            case TypeTag::INT: {
                std::string s = std::to_string(getInt(fieldPos));
                ss << s;
                break;
            }
            case TypeTag::VARCHAR: {
                ss << getChar(fieldPos);
            }

        }
        std::string result = ss.str();
        return result;
    }

    void print_header() {
        for (int i = 0; i < _scheme->columnsCount(); ++i) {
            std::string name = _scheme->name(i);
            int d = _scheme->outputSize(i) - name.size();
            std::cout << _scheme->name(i);
            for (int i = 0; i < d + 1; ++i) {
                std::cout << ' ';
            }
        }
        std::cout << std::endl;
    }

    void print_values() {
        for (int i = 0; i < _scheme->columnsCount(); ++i) {
            std::string value = as_string(i);
            int d = _scheme->outputSize(i) - value.size();
            std::cout << value;
            for (int i = 0; i < d + 1; ++i) {
                std::cout << ' ';
            }
        }
        std::cout << std::endl;
    }
};

/*
 * These classes with 'Meta' suffix are for reading meta data
 *
 * All kinds of metadata usually use both fixed-size and variable-size
 * parts.
 *
 * For example, for page meta data you have static fields like nTuples
 * and variable size like offsets of fields
 *
 * To cover this variable size parts following fields are introduced
 *   - size - total size of meaningful data (including fixed size part)
 *   - remaining - empty byte array as starting point of variable size part
 *
 * For example, if variable size part of TableMeta has size 10 you can
 * use remaining[0] up to remaining[9] to address these bytes
 */
struct TableMeta {
    std::int32_t size;

    std::int32_t metaSize;
    std::int32_t nPages;
    std::int32_t nTuples;

    std::byte remaining[0];
};

struct PageMeta {
    int size;

    std::int32_t metaSize;
    std::int32_t nTuples;

    std::byte remaining[0];
};

struct TupleMeta {
    int size;

    std::int32_t metaSize;

    std::byte remaining[0];
};

