#pragma once

#include <string>
#include <variant>
#include <vector>

namespace query {
    class CharType {
    private:
        unsigned int _size;

    public:
        CharType(unsigned int size): _size(size) {}
    };

    class DataType {
    private:
        std::variant<CharType> type;

    public:
        DataType(CharType charType): type(charType) {
        }
    };


    class FieldDefinition {
    private:
        std::string *_name;
        DataType *_type;

    public:
        FieldDefinition(std::string *name, DataType* type): _name(name), _type(type) {
        }
    };

    enum Type {CREATE, INSERT, SELECT, DELETE, UPDATE, UNKNOWN};

    class Query2 {
    };

    class CreateTable : Query2 {
    private:
        std::string _name;
        std::vector<FieldDefinition*>* _fieldDefs;
    public:
        CreateTable(std::string name, std::vector<FieldDefinition*>* fieldDefs): _name(name), _fieldDefs(fieldDefs) {
        }

        std::string name() {
            return _name;
        }
    };

    class Insert {
    private:
        std::string _name;
    public:
        Insert(std::string name): _name(name) {
        }

        std::string name() {
            return _name;
        }
    };

    class Delete {
    private:
        std::string _name;
    public:
        Delete(std::string name): _name(name) {
        }

        std::string name() {
            return _name;
        }
    };

    class Update {
    private:
        std::string _name;
    public:
        Update(std::string name): _name(name) {
        }

        std::string name() {
            return _name;
        }
    };

    class Select {
    private:
        std::string _name;
    public:
        Select(std::string name): _name(name) {
        }

        std::string name() {
            return _name;
        }

    };

    class Query {
    private:
        std::variant<CreateTable*, Insert*, Delete*, Update*, Select*> query;
    public:
        Query() {
        }

        Query(CreateTable* createTable): query(createTable) {
        }

        Query(Insert* insert): query(insert) {
        }

        Query(Select* select): query(select) {
        }

        Query(Delete* deleteQuery): query(deleteQuery) {
        }

        Query(Update* update): query(update) {
        }


        Type type() {
            if (query.index() == 0) {
                return Type::CREATE;
            } else if (query.index() == 1) {
                return Type::INSERT;
            } else if (query.index() == 2) {
                return Type::DELETE;
            } else if (query.index() == 3) {
                return Type::UPDATE;
            } else if (query.index() == 4) {
                return Type::SELECT;
            }
            return Type::UNKNOWN;
        }

        CreateTable* createTable() {
            return std::get<CreateTable*>(query);
        }

        Insert* insert() {
            return std::get<Insert*>(query);
        }

        Delete* deleteQuery() {
            return std::get<Delete*>(query);
        }

        Select* select() {
            return std::get<Select*>(query);
        }

        Update* update() {
            return std::get<Update*>(query);
        }

        void debug_print() {
        }
    };

};

