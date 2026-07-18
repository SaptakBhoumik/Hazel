#pragma once
#include "mir.hpp"

namespace Hazel {
namespace Snap{
namespace IR {
class MirPoisonLiteralExpr: public MirLiteralExpr{
    public:
    MirPoisonLiteralExpr(LiteralExprPtr literal_expr);
    bool is_immediate() const override;
    std::string to_string() const override;
};

class MirNamedLiteralExpr: public MirLiteralExpr{
    public:
    MirNamedLiteralExpr(LiteralExprPtr literal_expr);
    std::string get_name() const; 
    bool is_immediate() const override;
    std::string to_string() const override;
};

class MirNumLiteralExpr: public MirLiteralExpr{
    std::int64_t value;
    std::uint8_t decimal_places;//Number of decimal places for decimal type. For int type it is 0. For decimal type it is the number of decimal places in the number. For example 1.23 has 2 decimal places
    public:
    MirNumLiteralExpr(LiteralExprPtr literal_expr);
    std::int64_t get_value() const;
    std::uint8_t get_decimal_places() const;
    bool is_immediate() const override;
    std::string to_string() const override;
};

class MirStringLiteralExpr: public MirLiteralExpr{
    std::string value;
    public:
    MirStringLiteralExpr(LiteralExprPtr literal_expr);
    std::string get_value() const;//The actual value. Like currently stuff like "Hello\n" is stored raw. We will convert it to actual value i.e add newline. 
    bool is_immediate() const override;
    std::string to_string() const override;
};

class MirArrayLiteralExpr: public MirLiteralExpr{
    std::vector<MirLiteralExprPtr> elements;
    public:
    MirArrayLiteralExpr(LiteralExprPtr literal_expr, std::vector<MirLiteralExprPtr> elements);
    std::vector<MirLiteralExprPtr> get_elements() const;
    bool is_immediate() const override;//If every element is immediate
    std::string to_string() const override;
};

class MirStructLiteralExpr: public MirLiteralExpr{
    std::vector<MirLiteralExprPtr> fields;
    public:
    MirStructLiteralExpr(LiteralExprPtr literal_expr, std::vector<MirLiteralExprPtr> fields);
    std::vector<MirLiteralExprPtr> get_fields() const;
    bool is_immediate() const override;//If every field is immediate
    std::string to_string() const override;
};

class MirLabelLiteralExpr: public MirLiteralExpr{
    std::vector<MirLiteralExprPtr> args;
    public:
    MirLabelLiteralExpr(LiteralExprPtr literal_expr, std::vector<MirLiteralExprPtr> args);
    std::vector<MirLiteralExprPtr> get_args() const;
    bool is_immediate() const override;//If every arg is immediate
    std::string to_string() const override;
};

class MirFuncLiteralExpr: public MirLiteralExpr{
    std::vector<MirLiteralExprPtr> args;
    public:
    MirFuncLiteralExpr(LiteralExprPtr literal_expr, std::vector<MirLiteralExprPtr> args);
    std::vector<MirLiteralExprPtr> get_args() const;
    bool is_immediate() const override;//If every arg is immediate
    std::string to_string() const override;
};
}
}
}