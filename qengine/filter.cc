//
// Created by jaroslav on 28/12/22.
//

#include "filter.h"

// NOTE
// Current implementation only allows expressions like these:
// ... WHERE TRUE
// ... WHERE FALSE
// ... WHERE id = 1
// ... WHERE id > 4
// ... WHERE id < 10
// ... WHERE bool_field
// and so on. Also, no checks for types are performed

std::shared_ptr<std::vector<DenseTuple>> filter_tuples(
        datatypes::Expression* expression,
        std::vector<DenseTuple> tuples
        ) {

    std::vector<DenseTuple> out;

    if (expression == nullptr) {
        return std::make_shared<std::vector<DenseTuple>>(tuples);
    }

    for (int i = 0; i < tuples.size(); i++) {
        bool result = check_predicate(expression, tuples.at(i));
        if (result) out.push_back(tuples.at(i));
    }

    return std::make_shared<std::vector<DenseTuple>>(out);
}

int resolve_value(datatypes::ExpressionValue* value, DenseTuple tuple) {
    if (value->type == "INT") {
        return std::atoi(value->value.c_str());
    } else {
        auto scheme = tuple.getScheme().get();
        int pos = scheme->position(value->value);
        return tuple.getInt(pos);
    }
}


bool check_predicate(datatypes::Expression* expression, DenseTuple tuple) {
    if (expression == nullptr) return true;

    int left = resolve_value(expression->left->value, tuple);
    int right = resolve_value(expression->right->value, tuple);

    if (*expression->operation == std::string("=")) {
        return left == right;
    } else if (*expression->operation == std::string(">")) {
        return left > right;
    } else if (*expression->operation == std::string(">=")) {
        return left >= right;
    } else if (*expression->operation == std::string("<")) {
        return left < right;
    } else if (*expression->operation == std::string("<=")) {
        return left <= right;
    }

    if (expression->value->type == "BOOLEAN") {
        return expression->value->value == "TRUE";
    }


    std::cout << "WARN: Unsupported" << std::endl;
    return true;
}
