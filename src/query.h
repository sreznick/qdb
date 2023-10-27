#pragma once

#include "table/types.h"

#include <string>
#include <variant>
#include <vector>
#include <iostream>

namespace query {

    class Expr {
    public:
        Expr() {};
        virtual std::string print(int depth) {
            return "";
        }
    };

    class LogicalExpr {
    public:
        LogicalExpr() {};
        virtual std::string print(int depth) {
            return "";
        };
    };

    class CharType {
    private:
        unsigned int _size;

    public:
        CharType(unsigned int size): _size(size) {}

        unsigned int size() {
            return _size;
        }
    };

    class VarCharType {
    private:
        unsigned int _size;

    public:
        VarCharType(unsigned int size): _size(size) {}

        unsigned int size() {
            return _size;
        }
    };

    enum BasicType { INT, REAL, TEXT, BOOLEAN, UNKNOWN_TYPE };

    class DataType {
    private:
        std::variant<CharType*, VarCharType*, BasicType> type;

    public:
        DataType(CharType* charType): type(charType) {}
        DataType(VarCharType* varCharType): type(varCharType) {}
        DataType(BasicType basicType): type(basicType) {}

        CharType* getAsChar() {
            if (!std::holds_alternative<CharType*>(type))
                return nullptr;
            return std::get<CharType*>(type);
        }

        VarCharType* getAsVarChar() {
            if (!std::holds_alternative<VarCharType*>(type))
                return nullptr;
            return std::get<VarCharType*>(type);
        }

        BasicType getAsBasic() {
            if (!std::holds_alternative<BasicType>(type))
                return BasicType::UNKNOWN_TYPE;
            return std::get<BasicType>(type);
        }
    };


    class FieldDefinition {
    private:
        std::string _name;
        DataType* _type;

    public:
        FieldDefinition(std::string name, DataType *type):
            _name(name), _type(type) {}

        std::string name() {
            return _name;
        }

        std::string type() {
            if (_type->getAsChar() != nullptr)
                return "CHAR[" + std::to_string(_type->getAsChar()->size()) + "]";
            if (_type->getAsVarChar() != nullptr)
                return "VARCHAR[" + std::to_string(_type->getAsVarChar()->size()) + "]";

            BasicType basic = _type->getAsBasic();
            switch (basic) {
                case BasicType::BOOLEAN:
                    return "BOOLEAN";
                case BasicType::INT:
                    return "INT";
                case BasicType::REAL:
                    return "REAL";
                case BasicType::TEXT:
                    return "TEXT";
                default:
                    return "UNKNOWN";
            }
        }

        std::pair<TypeTag, int> db_type() {
            if (_type->getAsChar() != nullptr)
                return { TypeTag::CHAR, _type->getAsChar()->size() };
            if (_type->getAsVarChar() != nullptr)
                return { TypeTag::VARCHAR, _type->getAsVarChar()->size() };

            BasicType basic = _type->getAsBasic();
            switch (basic) {
                case BasicType::BOOLEAN:
                    return { TypeTag::BOOL, 0 };
                case BasicType::INT:
                    return { TypeTag::INT, 0 };
                case BasicType::REAL:
                    return { TypeTag::DOUBLE, 0 };
                case BasicType::TEXT: {
                    std::cout << "TEXT is not supported.";
                    exit(1);
                }
                default: {
                    std::cout << "UNKNOWN is not supported.";
                    exit(1);
                }
            }
        }
    };

    class FieldValue {
    private:
        std::variant<int, float, std::string, bool> _value;

    public:
        FieldValue(int value) : _value(value) {}
        FieldValue(float value) : _value(value) {}
        FieldValue(std::string value) : _value(value) {}
        FieldValue(bool value) : _value(value) {}

        std::string getString() {
            if (std::holds_alternative<int>(_value))
                return "INT: " + std::to_string(std::get<int>(_value));
            else if (std::holds_alternative<float>(_value))
                return "REAL: " + std::to_string(std::get<float>(_value));
            else if (std::holds_alternative<std::string>(_value))
                return "TEXT: " + std::get<std::string>(_value) + "";
            else if (std::holds_alternative<bool>(_value)) {
                if (std::get<bool>(_value))
                    return "BOOLEAN: TRUE";
                return "BOOLEAN: FALSE";
            }
            return "UNKNOWN";
        }

        std::pair<std::string, std::string> dbGet() {
            if (std::holds_alternative<int>(_value))
                return { "INT", std::to_string(std::get<int>(_value)) };
            else if (std::holds_alternative<float>(_value))
                return { "REAL", std::to_string(std::get<float>(_value)) };
            else if (std::holds_alternative<std::string>(_value))
                return { "TEXT", std::get<std::string>(_value) };
            else if (std::holds_alternative<bool>(_value)) {
                if (std::get<bool>(_value))
                    return { "BOOLEAN", "1" };
                return{ "BOOLEAN", "" };
            }
            std::cout << "UNKNOWN is not supported.";
            exit(1);
        }
    };

    class FieldSet {
    private:
        std::string _name;
        Expr* _expr;

    public:
        FieldSet(std::string name, Expr* expr) : _name(name), _expr(expr) {}

        std::string name() {
            return _name;
        }

        Expr* expr() {
            return _expr;
        }
    };

    class ExprOp : public Expr {
    public:
        Expr* _leftExpr;
        Expr* _rightExpr;
        std::string _op;

        ExprOp(Expr* leftExpr, Expr* rightExpr, std::string op) :
            _leftExpr(leftExpr), _rightExpr(rightExpr), _op(op) {};

        std::string print(int depth) {
            return _leftExpr->print(depth + 2) +
                std::string(depth, ' ') + _op + '\n' +
                _rightExpr->print(depth + 2);
        }
    };

    class ExprMinus : public Expr {
    public:
        Expr* _expr;

        ExprMinus(Expr* expr) : _expr(expr) {};

        std::string print(int depth) {
            return std::string(depth, ' ') + "-\n" + _expr->print(depth + 2);
        }
    };

    class ExprBrackets : public Expr {
    public:
        Expr* _expr;

        ExprBrackets(Expr* expr) : _expr(expr) {};

        std::string print(int depth) {
            return std::string(depth, ' ') + "()\n" + _expr->print(depth + 2);
        }
    };

    class ExprValue : public Expr {
    public:
        FieldValue _value;

        ExprValue(int value) : _value(FieldValue(value)) {};
        ExprValue(float value) : _value(FieldValue(value)) {};
        ExprValue(std::string value) : _value(FieldValue(value)) {};

        std::string print(int depth) {
            return std::string(depth, ' ') + _value.getString() + '\n';
        }
    };

    class ExprField : public Expr {
    public:
        std::string _field;

        ExprField(std::string field) : _field(field) {};

        std::string print(int depth) {
            return std::string(depth, ' ') + "UNKNOWN\n";
        }
    };

    class ExprLogical : public Expr {
    public:
        LogicalExpr* _logicalExpr;

        ExprLogical(LogicalExpr* logicalExpr) : _logicalExpr(logicalExpr) {};

        std::string print(int depth) {
            return _logicalExpr->print(depth);
        }
    };

    class LogicalExprBool : public LogicalExpr {
    public:
        FieldValue _value;

        LogicalExprBool(bool value) : _value(FieldValue(value)) {};

        std::string print(int depth) {
            return std::string(depth, ' ') + _value.getString() + '\n';
        }
    };

    class LogicalExprOp : public LogicalExpr {
    public:
        Expr* _leftExpr;
        Expr* _rightExpr;
        std::string _op;

        LogicalExprOp(Expr* leftExpr, Expr* rightExpr, std::string op) :
            _leftExpr(leftExpr), _rightExpr(rightExpr), _op(op) {};

        std::string print(int depth) {
            return _leftExpr->print(depth + 2) +
                   std::string(depth, ' ') + _op + '\n' +
                   _rightExpr->print(depth + 2);
        }
    };

    class LogicalExprLogicalOp : public LogicalExpr {
    public:
        LogicalExpr* _leftExpr;
        LogicalExpr* _rightExpr;
        std::string _op;

        LogicalExprLogicalOp(LogicalExpr* leftExpr, LogicalExpr* rightExpr, std::string op) :
                _leftExpr(leftExpr), _rightExpr(rightExpr), _op(op) {};

        std::string print(int depth) {
            return _leftExpr->print(depth + 2) +
                   std::string(depth, ' ') + _op + '\n' +
                   _rightExpr->print(depth + 2);
        }
    };

    class LogicalExprNot : public LogicalExpr {
    public:
        LogicalExpr* _expr;

        LogicalExprNot(LogicalExpr* expr) : _expr(expr) {};

        std::string print(int depth) {
            return std::string(depth, ' ') + "NOT\n" + _expr->print(depth + 2);
        }
    };

    class LogicalExprBrackets : public LogicalExpr {
    public:
        LogicalExpr* _expr;

        LogicalExprBrackets(LogicalExpr* expr) : _expr(expr) {};

        std::string print(int depth) {
            return std::string(depth, ' ') + "()\n" + _expr->print(depth + 2);
        }
    };

    enum Type {CREATE, INSERT, SELECT, DELETE, UPDATE, UNKNOWN};

    class Query2 {
    };

    class CreateTable : Query2 {
    private:
        std::string _name;
        std::vector<FieldDefinition*>* _fieldDefs = nullptr;
    public:
        CreateTable(std::string name, std::vector<FieldDefinition*>* fieldDefs):
            _name(name), _fieldDefs(fieldDefs) {}

        std::string name() {
            return _name;
        }

        std::vector<FieldDefinition*>* fieldDefs() {
            return _fieldDefs;
        }
    };

    class Insert {
    private:
        std::string _name;
        std::vector<FieldValue*>* _field_values = nullptr;
    public:
        Insert(std::string name, std::vector<FieldValue*>* values):
            _name(name), _field_values(values) {}

        std::string name() {
            return _name;
        }

        std::vector<FieldValue*>* values() {
            return _field_values;
        }
    };

    class Delete {
    private:
        std::string _name;
        LogicalExpr* _logical = nullptr;
    public:
        Delete(std::string name):
            _name(name) {}
        Delete(std::string name, LogicalExpr* logical):
            _name(name), _logical(logical) {}

        std::string name() {
            return _name;
        }

        LogicalExpr* logical() {
            return _logical;
        }
    };

    class Update {
    private:
        std::string _name;
        LogicalExpr* _logical = nullptr;
        std::vector<FieldSet*>* _sets = nullptr;
    public:
        Update(std::string name, LogicalExpr* logical, std::vector<FieldSet*>* sets):
            _name(name), _logical(logical), _sets(sets) {}

        std::string name() {
            return _name;
        }

        std::vector<FieldSet*>* sets() {
            return _sets;
        }

        LogicalExpr* logical() {
            return _logical;
        }
    };

    class Select {
    private:
        std::string _name;
        LogicalExpr* _logical = nullptr;
        std::vector<Expr*>* _exprs = nullptr;
    public:
        Select(std::string name):
            _name(name) {}
        Select(std::string name, LogicalExpr* logical):
            _name(name), _logical(logical) {}
        Select(std::string name, LogicalExpr* logical, std::vector<Expr*>* exprs):
            _name(name), _logical(logical), _exprs(exprs) {}

        std::string name() {
            return _name;
        }

        std::vector<Expr*>* exprs() {
            return _exprs;
        }

        LogicalExpr* logical() {
            return _logical;
        }

    };

    class Query {
    private:
        std::variant<CreateTable*, Insert*, Delete*, Update*, Select*> query;
        int depth = 2;
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
            std::cout << "Query: ";
            switch (this->type())
            {
                case query::Type::CREATE: {
                    query::CreateTable* query = this->createTable();

                    std::cout << "CREATE" << std::endl << "Table: " << query->name() << std::endl
                        << "Field definitions:" << std::endl;
                    std::vector<query::FieldDefinition*>* fieldDefs = query->fieldDefs();
                    for (query::FieldDefinition* fieldDef : *fieldDefs)
                        std::cout << std::string(depth, ' ') << fieldDef->type() << ": "
                            << fieldDef->name() << std::endl;
                    break;
                }
                case query::Type::UPDATE: {
                    query::Update* query = this->update();

                    std::cout << "UPDATE" << std::endl << "Table: " << query->name() << std::endl;
                    std::vector<query::FieldSet*>* sets = query->sets();
                    for (query::FieldSet* set : *sets)
                        std::cout << std::string(depth, ' ') << set->name() << ":"
                            << std::endl << set->expr()->print(depth + 2) << std::flush;

                    std::cout << "Where:" << std::endl << query->logical()->print(depth) << std::flush;
                    break;
                }
                case query::Type::DELETE: {
                    query::Delete* query = this->deleteQuery();

                    std::cout << "DELETE" << std::endl << "Table: " << query->name() << std::endl;

                    query::LogicalExpr* logical = query->logical();
                    if (logical != nullptr)
                        std::cout << "Where:" << std::endl << logical->print(depth) << std::flush;
                    break;
                }
                case query::Type::INSERT: {
                    query::Insert* query = this->insert();

                    std::cout << "INSERT" << std::endl << "Table: " << query->name() << std::endl
                        << "Values:" << std::endl;
                    std::vector<query::FieldValue*>* values = query->values();
                    for (query::FieldValue* value : *values)
                        std::cout << std::string(depth, ' ') << value->getString() << std::endl;
                    break;
                }
                case query::Type::SELECT: {
                    query::Select* query = this->select();

                    std::cout << "SELECT" << std::endl << "Table: " << query->name() << std::endl << "What:";
                    std::vector<query::Expr*>* exprs = query->exprs();
                    if (exprs == nullptr)
                        std::cout << " *" << std::endl;
                    else {
                        std::cout << std::endl;
                        for (query::Expr* expr : *exprs)
                            std::cout << expr->print(depth) << std::flush;
                    }

                    query::LogicalExpr* logical = query->logical();
                    if (logical != nullptr)
                        std::cout << "Where:" << std::endl << logical->print(depth) << std::flush;
                    break;
                }
                case query::Type::UNKNOWN: {
                    std::cout << "Unknown command" << std::endl;
                    break;
                }
                default:
                    break;
            }
        }
    };

};

