#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <string>
#include <vector>
#include <variant>
#include <optional>

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

namespace Utilities {
    // string processing
    std::string ToLowerCase(std::string_view string);
    void ToLowerCase(std::string& string);
    void TrimLeadingAndTrailingWhitespaces(std::string& string);

    // analysis
    bool IsWhitespace(char c);
    bool IsSymbol(char c);
    bool IsOperator(char c);
    bool IsNewTokenChar(char c);
    bool IsNonLogicalOperatorKeyword(std::string_view string);
    bool IsLogicalOperatorKeyword(std::string_view string);
    bool IsRelationalOrAssignmentOperator(char c);
    bool IsArithmeticOperator(char c);

    // parsing
    std::optional<std::string> GetCommandArgument(const std::vector<Token>& token_line);

    // exceptions
    std::exception AddContext(const std::string& context, const std::exception& ex);
};

#endif