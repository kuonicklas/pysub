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
    Indent
};

struct Token {
    std::variant<std::string, int> value;
    Category category;
};

using TokenLine = std::vector<Token>;

#endif