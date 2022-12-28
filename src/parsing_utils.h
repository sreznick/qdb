#pragma once

#include <iostream>

namespace parsing_utils {
    inline int extract_int_from_datatype(std::string datatype) {
        std::string::size_type p = datatype.find('(');
        std::string::size_type pp = datatype.find(')', p + 2);

        return atoi(datatype.substr(p + 1, pp - p - 1).c_str());
    }


    inline void printExpression(const std::string& prefix, const datatypes::Expression* node, bool isLeft)
    {
        if( node != nullptr )
        {
            std::cout << prefix;
            if (node->value != nullptr) {
                std::cout << node->value->value << " :: " << node->value->type << std::endl;
            } else {
                std::string* val = node->operation;
                std::cout << *val << std::endl;
            };

            printExpression(prefix +  "    ", node->left, true);
            printExpression(prefix +  "    ", node->right, false);
        }
    }
    inline void printExpression(const datatypes::Expression* node)
    {
        printExpression("", node, false);
    }
}