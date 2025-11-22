#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <string>
#include <vector>
#include <variant>

enum class Category
{
    Keyword,
    Identifier,
    StringLiteral,
    NumericLiteral,
    AssignmentOperator,
    ArithmeticOperator,
    LogicalOperator,
    RelationalOperator,
    LeftParenthesis,
    RightParenthesis,
    Colon,
    Comma,
    Comment,
    Indent,
    Dedent,
    Newline
};

using ValueType = std::variant<std::string, int>;

struct Token {
    ValueType value;
    Category category;

    bool operator==(const Token& rhs) const {
        return this->category == rhs.category && this->value == rhs.value;
    }
};

#endif