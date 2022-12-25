#pragma once

#include <string>
#include <variant>
#include <vector>

namespace query {
    enum LogicalExpressionType {
        TRUE,
        FALSE,
        LOG_BRACKETS,
        EQUALS,
        NEQUALS,
        AND,
        OR,
        NOT
    };

    enum ExpressionType {
        INTEGER,
        EXPR_REAL,
        STRING,
        FIELD,
        LOGICAL_EXPR,
        BRACKETS,
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE
    };

    class LogicalExpression;

    class Expression {
        public:
            ExpressionType type; 
            int integerField;
            std::string stringField;
            float floatField;
            Expression* leftExpression;
            Expression* rightExpression;
            LogicalExpression* logicalExpression;

            Expression(ExpressionType _type) : type(_type) {}

            std::string print();
    };

    class LogicalExpression {
        public:
            LogicalExpressionType type;
            Expression* leftExpression;
            Expression* rightExpression;
            LogicalExpression* leftLogicalExpression;
            LogicalExpression* rightLogicalExpression;
    
            LogicalExpression(LogicalExpressionType _type) : type(_type) {}

            std::string print() {
                std::string left = "", right = "", mid = "";

                if (leftExpression != nullptr) {
                    left = leftExpression->print();
                }

                if (rightExpression != nullptr) {
                    right = rightExpression->print();
                }

                if (leftLogicalExpression != nullptr) {
                    left = leftLogicalExpression->print();
                }

                if (rightLogicalExpression != nullptr) {
                    right = rightLogicalExpression->print();
                }

                if (type == LogicalExpressionType::TRUE) {
                    mid = "TRUE";
                } else if (type == LogicalExpressionType::FALSE) {
                    mid = "FALSE";
                } else if (type == LogicalExpressionType::AND) {
                    mid = "&&";
                } else if (type == LogicalExpressionType::OR) {
                    mid = "||";
                } else if (type == LogicalExpressionType::EQUALS) {
                    mid = "==";
                } else if (type == LogicalExpressionType::NEQUALS) {
                    mid = "!=";
                } else if (type == LogicalExpressionType::NOT) {
                    mid = "!";
                } else if (type == LogicalExpressionType::LOG_BRACKETS) {
                    mid = "(" + left + ")";
                    left = "";
                }

                return left + mid + right;

            }
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

    enum SimpleType {
        INT,
        REAL,
        TEXT,
        BOOLEAN,
        UNKNOWN_TYPE
    };

    class DataType {
    private:
        std::variant<CharType*, VarCharType*, SimpleType> type;

    public:
        DataType(CharType* charType): type(charType) {
        }
        DataType(VarCharType* vchartype): type(vchartype) {
        }
        DataType(SimpleType simple): type(simple) {
        }

        CharType* getCharType() {
            if (!std::holds_alternative<CharType*>(type)) {
                return nullptr;
            }

            return std::get<CharType*>(type);
        }

        VarCharType* getVarcharType() {
            if (!std::holds_alternative<CharType*>(type)) {
                return nullptr;
            }

            return std::get<VarCharType*>(type);
        }

        SimpleType getSimpleType() {
            if (!std::holds_alternative<SimpleType>(type)) {
                return SimpleType::UNKNOWN_TYPE;
            }

            return std::get<SimpleType>(type);
        }
    };


    class FieldDefinition {
    private:
        std::string _name;
        DataType* _type;

    public:
        FieldDefinition(std::string name, DataType* type): _name(name), _type(type) {
        }

        std::string name() {
            return _name;
        }

        std::string type() {
            if (_type->getCharType() != nullptr) {
                return "CharType with size: " + std::to_string(_type->getCharType()->size());
            } else if (_type->getVarcharType() != nullptr) {
                return "VarCharType with size: " + std::to_string(_type->getCharType()->size());
            } 
            
            SimpleType tp =  _type->getSimpleType();

            switch (tp) {
            case SimpleType::BOOLEAN:
                return "BOOLEAN";
                break;
                
            case SimpleType::INT:
                return "INT";
                break;

            case SimpleType::REAL:
                return "REAL";
                break;

            case SimpleType::TEXT:
                return "TEXT";
                break;
            
            default:
                break;
            }

            return "UNKNOWN TYPE";
        }
    };

    class FieldValue {
        private:
            std::variant<int, float, std::string, bool> _value;
        
        public:
            FieldValue(int ivalue) : _value(ivalue) {}
            FieldValue(float fvalue) : _value(fvalue) {}
            FieldValue(std::string svalue) : _value(svalue) {}
            FieldValue(bool bvalue) : _value(bvalue) {}

            std::string type() {
                if (std::holds_alternative<int>(_value)) {
                    return "Integer: " + std::to_string(std::get<int>(_value));
                } else if (std::holds_alternative<float>(_value)) {
                    return "Float: " + std::to_string(std::get<float>(_value));
                } else if (std::holds_alternative<std::string>(_value)) {
                    return "String: \"" + std::get<std::string>(_value) + "\"";
                } else if (std::holds_alternative<bool>(_value)) {
                    return "Boolean: " + std::to_string(std::get<bool>(_value));
                }

                return "UNKNOWN";
        }
    };

    class FieldSetup {
        private:
            std::string _name;
            Expression* _expression;
        public:
            FieldSetup(std::string name, Expression* expression) : _name(name), _expression(expression) {}

            std::string name() {
                return _name;
            }

            Expression* expression() {
                return _expression;
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

        std::vector<FieldDefinition*>* defs() {
            return _fieldDefs;
        }        
    };

    class Insert {
    private:
        std::string _name;
        std::vector<FieldValue*>* _field_values;
    public:
        Insert(std::string name, std::vector<FieldValue*>* values): _name(name), _field_values(values) {}

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
        LogicalExpression* _logical;
    public:
        Delete(std::string name) : _name(name) {}
        Delete(std::string name, LogicalExpression* logical): _name(name), _logical(logical) {}

        std::string name() {
            return _name;
        }

        LogicalExpression* logical() {
            return _logical;
        }
    };

    class Update {
    private:
        std::string _name;
        LogicalExpression* _logical;
        std::vector<FieldSetup*>* _setups;
    public:
        Update(std::string name, LogicalExpression* logical, std::vector<FieldSetup*>* setups): _name(name), _logical(logical), _setups(setups) {}

        std::string name() {
            return _name;
        }

        std::vector<FieldSetup*>* setups() {
            return _setups;
        }

        LogicalExpression* logical() {
            return _logical;
        }
    };

    class Select {
    private:
        std::string _name;
        LogicalExpression* _logical;
        std::vector<Expression*>* _expressions;
    public:
        Select(std::string name) : _name(name) {}
        Select(std::string name, LogicalExpression* logical): _name(name), _logical(logical) {}
        Select(std::string name, LogicalExpression* logical, std::vector<Expression*>* expressions): _name(name), _logical(logical), _expressions(expressions) {}

        std::string name() {
            return _name;
        }

        std::vector<Expression*>* expressions() {
            return _expressions;
        }

        LogicalExpression* logical() {
            return _logical;
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
