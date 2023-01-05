#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <memory>
#include <cinttypes>

#include <string.h>

#include "types.h"
#include "storage/fileid.h"

int const PAGE_META_SIZE = 12;


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

    int position(std::string name) {
        return _pos_by_name.find(name)->second;
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
    int _lastPageId;
    int _indexFileId;
    int _indexFieldPos;

public:
    Table(std::string name,
          std::shared_ptr<TableScheme> tableScheme,
          FileId fileId,
          int lastPageId,
          int indexFileId,
          int indexFieldPos):
            _name(name),
            _tableScheme(tableScheme),
            _fileId(fileId),
            _lastPageId(lastPageId),
            _indexFileId(indexFileId),
            _indexFieldPos(indexFieldPos) {
    }

    std::shared_ptr<TableScheme> scheme() {
        return _tableScheme;
    }

    FileId fileId() {
        return _fileId;
    }

    std::string name() {
        return _name;
    }

    PageId lastPageId() const {
        return PageId{{_fileId}, _lastPageId};
    }

    FileId indexFileId() const {
        return FileId{_indexFileId};
    }

    int indexFieldPos() const {
        return _indexFieldPos;
    }

    bool has_index() {
        return _indexFileId > 0;
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

    DenseTuple(std::shared_ptr<TableScheme> scheme, std::byte* data) {
        _scheme = scheme;
        _data = data;
        _offsets.push_back(0);

        for (int i = 0; i < scheme->columnsCount() - 1; ++i) {
            int last = _offsets[_offsets.size() - 1];
            _offsets.push_back(last + scheme->fieldSize(i));
        }
    }

    // throws errors
    //    ~DenseTuple() {
    //        delete[] _data;
    //    }

    std::byte* getData() {
       return _data;
    }


    std::shared_ptr<TableScheme> getScheme() {
        return _scheme;
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
        result.erase(std::find(result.begin(), result.end(), '\0'), result.end());

        return result;
    }

    void setBool(int fieldPos, bool value) {
        ::memcpy(_data +_offsets[fieldPos], &value, 1);
    }

    bool getBool(int fieldPos) {
        bool result;
        ::memcpy(&result, _data + _offsets[fieldPos], 1);

        return result;
    }

    void setLong(int fieldPos, long value) {
        ::memcpy(_data + _offsets[fieldPos], &value, 8);
    }

    long getLong(int fieldPos) {
        long result;
        ::memcpy(&result, _data + _offsets[fieldPos], 8);

        return result;
    }

    int getTotalSize() {
        return _scheme.get()->totalSize();
    }

    void setDouble(int fieldPos, double value) {
        ::memcpy(_data + _offsets[fieldPos], &value, 8);
    }

    double getDouble(int fieldPos) {
        double result;

        ::memcpy(&result, _data + _offsets[fieldPos], 8);

        return result;
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
      std::int32_t fileId;
      std::int32_t lastPageId;
      std::int32_t indexFileId;
      std::int32_t indexFieldPos;
      char name[16];
};

// Every page consists of these parts:
// 1) constant-sized header
// 2) pointers (4 bytes of record size, 4 bytes of record offset)
// 3) free space
// 4) data
// pointerLeft points to pointers section end
// pointerRight points to data section start

struct PageMeta {
    std::int32_t pointerLeft;
    std::int32_t pointerRight;
    std::int32_t tuplesCount;
};

struct TupleMeta {
    int size;
    std::int32_t metaSize;
    std::byte remaining[0];
};

class DenseTuplesRepr {
    static const int MAX_COLUMNS = 50;
private:
    std::shared_ptr<TableScheme> _tableSchemePtr;
    std::vector<DenseTuple> _tuples;
    bool _visible[MAX_COLUMNS];
public:
    DenseTuplesRepr(std::vector<DenseTuple> tuples, std::shared_ptr<TableScheme> tableSchemePtr):
    _tuples(tuples),
    _tableSchemePtr(tableSchemePtr) {
        for (int i = 0; i < MAX_COLUMNS; i++) _visible[i] = true;
    }

    std::vector<DenseTuple> tuples() {
        return _tuples;
    }

    int visible_count() {
        int counter = 0;
        for (int i = 0; i < _tableSchemePtr.get()->columnsCount(); i++) {
            if (_visible[i]) counter += 1;
        }

        return counter;
    }

    bool is_visible(int i) {
        if (i >= MAX_COLUMNS || i < 0) {
            std::cout << "Invalid value" << std::endl;
            exit(EXIT_FAILURE);
        }

        return _visible[i];
    }
};
