#include <string>
#include <iostream>

namespace datatypes {
    class Datatype {
    public:
        std::string name;
        int size;

        Datatype(std::string name, int size) : name(name), size(size) {
        }

        std::string to_s() {
            if (size > 0) {
                return "datatype: " + name + "(" + std::to_string(size) + ")\n";
            } else {
                return "datatype: " + name + "\n";
            }
        }
    };

    class Column {
    public:
        std::string name;
        Datatype* datatype;

        Column(std::string name, Datatype* datatype) : name(name), datatype(datatype) {
        }
    };

    class Literal {
    public:
        std::string value;
        std::string type;

        Literal(std::string value, std::string type): value(value), type(type) {}
    };

    class ExpressionValue {
    public:
        std::string value;
        std::string type;

        ExpressionValue(std::string value, std::string type): value(value), type(type) {}
    };


    class Expression {
    public:
        std::string* operation;
        ExpressionValue* value;
        Expression* left;
        Expression* right;

        Expression(Expression* left, Expression* right, std::string* operation): left(left), right(right), operation(operation) {}
        Expression(Expression* left, std::string* operation): left(left), operation(operation) {}
        Expression(ExpressionValue* value, std::string* operation): value(value), operation(operation) {}
        Expression(std::string* operation): operation(operation) {}
    };

    class Assignment {
    public:
        std::string identifier;
        datatypes::Expression* expression;

        Assignment(std::string identifier, datatypes::Expression* expression): identifier(identifier), expression(expression) {}
    };
};