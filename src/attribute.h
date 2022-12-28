#include <string>

namespace attrs {
    class Attribute {
    public:
        std::string datatype;
        std::string identifier;

        Attribute(std::string identifier, std::string datatype) : datatype(datatype), identifier(identifier) {}

        std::string tos() {
            return ":" + datatype + ":" + identifier + ":";
        }
    };
};