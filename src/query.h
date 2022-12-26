#pragma once

#include <string>
#include <variant>
#include <vector>

namespace query {
    class Exp {
        public:
            int typev, typel; 
            int ivals;
            std::string svals;
            float fvalks;
            Exp* leftEx;
            Exp* rightEx;
            Exp* llogEx;
            Exp* rlogEx;

            Exp(int _typev, int _typel) : typev(_typev), typel(_typel) {}

            std::string print() {
                std::string lex = "", rex = "", connector = "";

                if (leftEx != nullptr) {
                    lex = leftEx->print();
                }

                if (rightEx != nullptr) {
                    rex = rightEx->print();
                }

                if (llogEx != nullptr) {
                    lex = llogEx->print();
                }

                if (rlogEx != nullptr) {
                    rex = rlogEx->print();
                }

                switch (typel) {
                case 1: 
                    connector = "TRUE";
                    break;
                case 2: 
                    connector = "FALSE";
                    break;
                case 3: 
                    connector = "(" + lex + ")";
                    lex = "";
                    break;
                case 4: 
                    connector = "==";
                    break;
                case 5:
                    connector = "!=";
                    break;
                case 6: 
                    connector = "&&";
                    break;
                case 7: 
                    connector = "||";
                    break;
                case 8: 
                    connector = "!";
                    break;
                default: 
                    break;
                }

                switch (typev) {
                case 1: 
                    connector = std::to_string(ivals);
                    break;
                case 2: 
                    connector = std::to_string(fvalks);
                    break;
                case 3: 
                case 4: 
                    connector = svals;
                    break;
                case 5: 
                    connector = llogEx->print();
                    break;
                case 6: 
                    connector = "(" + leftEx->print() + ")";
                    lex = "";
                    break;
                case 7: 
                    connector = "+";
                    break;
                case 8: 
                    connector = "-";
                    break;
                case 9: 
                    connector = "/";
                    break;
                case 0: 
                    connector = "*";
                    break;
                default: 
                    break;
                }

                return lex + connector + rex;

            }
    };

    class FieldDefinition {
    private:
        std::string _name;
        int _ftp;

    public:
        FieldDefinition(std::string name, int type): _name(name), _ftp(type) {
        }

        std::string name() {
            return _name;
        }

        int ftp() {
            return _ftp;
        }
    };

    class VariantType {
        private:
            std::variant<int, float, std::string, bool> _tvar;
        
        public:
            VariantType(int tvar) : _tvar(tvar) {}
            VariantType(float tvar) : _tvar(tvar) {}
            VariantType(std::string tvar) : _tvar(tvar) {}
            VariantType(bool tvar) : _tvar(tvar) {}

            std::string type() {
                if (std::holds_alternative<int>(_tvar)) {
                    return "Integer: " + std::to_string(std::get<int>(_tvar));
                } else if (std::holds_alternative<std::string>(_tvar)) {
                    return "String: \"" + std::get<std::string>(_tvar) + "\"";
                } else if (std::holds_alternative<bool>(_tvar)) {
                    return "Boolean: " + std::to_string(std::get<bool>(_tvar));
                } else if (std::holds_alternative<float>(_tvar)) {
                    return "Float: " + std::to_string(std::get<float>(_tvar));
                } 

                return "UNKNOWN";
        }
    };

    class CreateTable {
    private:
        std::string _name;
        std::vector<FieldDefinition*>* _fdef;
    public:
        CreateTable(std::string name, std::vector<FieldDefinition*>* fdef): _name(name), _fdef(fdef) {
        }

        std::string name() {
            return _name;
        }

        std::vector<FieldDefinition*>* fdef() {
            return _fdef;
        }        
    };

    class Insert {
    private:
        std::string _name;
        std::vector<VariantType*>* _vtypes;
    public:
        Insert(std::string name, std::vector<VariantType*>* values): _name(name), _vtypes(values) {}

        std::string name() {
            return _name;
        }

        std::vector<VariantType*>* vtypes() {
            return _vtypes;
        }
    };

    class Delete {
    private:
        std::string _name;
        Exp* _log;
    public:
        Delete(std::string name) : _name(name) {}
        Delete(std::string name, Exp* log): _name(name), _log(log) {}

        std::string name() {
            return _name;
        }

        Exp* log() {
            return _log;
        }
    };

    class Update {
    private:
        std::string _name;
        Exp* _log;
        std::vector<std::pair<Exp*, std::string>*>* _sets;

    public:
        Update(std::string name, Exp* log, std::vector<std::pair<Exp*, std::string>*>* sets): _name(name), _log(log), _sets(sets) {}

        std::string name() {
            return _name;
        }

        std::vector<std::pair<Exp*, std::string>*>* setups() {
            return _sets;
        }

        Exp* log() {
            return _log;
        }
    };

    class Select {
    private:
        std::string _name;
        Exp* _log;
        std::vector<Exp*>* _exps;
    public:
        Select(std::string name) : _name(name) {}
        Select(std::string name, Exp* log): _name(name), _log(log) {}
        Select(std::string name, Exp* log, std::vector<Exp*>* expressions): _name(name), _log(log), _exps(expressions) {}

        std::string name() {
            return _name;
        }

        std::vector<Exp*>* expressions() {
            return _exps;
        }

        Exp* log() {
            return _log;
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


        int type() {
            if (query.index() == 0) {
                return 0;
            } else if (query.index() == 1) {
                return 1;
            } else if (query.index() == 2) {
                return 3;
            } else if (query.index() == 3) {
                return 4;
            } else if (query.index() == 4) {
                return 2;
            }
            return 5;
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