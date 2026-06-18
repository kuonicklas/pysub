#include "parser.hpp"

#include <iostream>
#include <stdexcept>
#include <cassert>

// return true and consume one token if curr token matches any of the inputs
bool Parser::Match(auto&&... input) {
	bool match_found = false;
	(
		[&] {
			if (!match_found) {
				match_found = Check(input) || match_found;
			}
		}(), ...
			);
	if (match_found) {
		IncrementToken();
	}
	return match_found;
}

void Parser::IncrementToken() {
	if (!IsAtEnd()) {
		++curr_token;
	}
}

bool Parser::Check(std::string_view s) const {
	if (IsAtEnd()) {
		return false;
	}
	return std::get<std::string>(curr_token->value) == s;
}

bool Parser::Check(Category category) const {
	if (IsAtEnd()) {
		return false;
	}
	return curr_token->category == category;
}

bool Parser::Check(const Token& compare_token) const {
	if (IsAtEnd()) {
		return false;
	}
	return *curr_token == compare_token;
}

bool Parser::IsAtEnd() const {
	return curr_token == std::end(tokens);
}

Token Parser::GetPreviousToken() const {
	assert(curr_token != std::begin(tokens));
	return *(curr_token - 1);
}

[[nodiscard]] std::unique_ptr<AST> Parser::BuildTree() {
	return BuildAST();
}

void Parser::CheckSyntax() {

}

std::unique_ptr<AST> Parser::BuildAST() {
	std::unique_ptr<AST> new_ast = std::make_unique<AST>();
	
	// skip empty lines
	while (Match(Category::Newline, Category::Comment)) {}
	if (!IsAtEnd()) {
		new_ast->statements = GetStatements();
	}
	return new_ast;
}

std::vector<std::unique_ptr<Statement>> Parser::GetStatements() {
	std::vector<std::unique_ptr<Statement>> statements{};
	assert(!Match(Category::Newline, Category::Comment));
	statements.push_back(GetStatement());
	while (!IsAtEnd()) {
		if (Match(Category::Newline, Category::Comment)) {
			continue;
		}
		statements.push_back(GetStatement());
	}
	return statements;
}

std::unique_ptr<Statement> Parser::GetStatement() {
	//if (Check("if") || Check("while")) {
	//	return GetCompoundStatement();
	//}
	std::unique_ptr<Statement> simple = GetSimpleStatement();
	Match(Category::Newline);	// optional
	return simple;
}

//CompoundStatement Parser::GetCompoundStatement() {
//	if (Check("if")) {
//		return GetIfStatement();
//	}
//	return GetWhileStatement();
//}

std::unique_ptr<Statement> Parser::GetSimpleStatement() {
	// assignment or expression (assignment omitted)
	return GetExpression();
}

std::unique_ptr<Expression> Parser::GetExpression() {
	std::unique_ptr<Expression> left = GetConjunction();
	while (Match(Token{ .value = "or", .category = Category::LogicalOperator })) {
		Token op = GetPreviousToken();
		std::unique_ptr<Expression> right = GetConjunction();
		left = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
	}
	return left;
}

std::unique_ptr<Expression> Parser::GetConjunction() {
	std::unique_ptr<Expression> left = GetInversion();
	while (Match(Token{ .value = "and", .category = Category::LogicalOperator })) {
		Token op = GetPreviousToken();
		std::unique_ptr<Expression> right = GetInversion();
		left = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
	}
	return left;
}

std::unique_ptr<Expression> Parser::GetInversion() {
	if (Match(Token{ .value = "not", .category = Category::LogicalOperator })) {
		Token op = GetPreviousToken();
		std::unique_ptr<Expression> expression = GetInversion();
		return std::make_unique<UnaryExpression>(std::move(expression), op);
	}
	return GetComparison();
}

std::unique_ptr<Expression> Parser::GetComparison() {
	std::unique_ptr<Expression> left = GetSum();
	while (Match(Category::RelationalOperator)) {
		Token op = GetPreviousToken();
		std::unique_ptr<Expression> right = GetSum();
		left = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
	}
	return left;
}

std::unique_ptr<Expression> Parser::GetSum() {
	std::unique_ptr<Expression> left = GetTerm();
	while (Match(Token{.value = "+", .category = Category::ArithmeticOperator}, Token{.value = "-", .category = Category::ArithmeticOperator})) {
		Token op = GetPreviousToken();
		std::unique_ptr<Expression> right = GetTerm();
		left = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
	}
	return left;
}

std::unique_ptr<Expression> Parser::GetTerm() {
	std::unique_ptr<Expression> left = GetFactor();
	while (Match(
		Token{ .value = "*", .category = Category::ArithmeticOperator },
		Token{ .value = "/", .category = Category::ArithmeticOperator },
		Token{ .value = "%", .category = Category::ArithmeticOperator }
		)) {
		Token op = GetPreviousToken();
		std::unique_ptr<Expression> right = GetFactor();
		left = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
	}
	return left;
}

std::unique_ptr<Expression> Parser::GetFactor() {
	if (Match(Token{ .value = "+", .category = Category::ArithmeticOperator }, Token{ .value = "-", .category = Category::ArithmeticOperator })) {
		Token op = GetPreviousToken();
		std::unique_ptr<Expression> expression = GetPrimary();
		return std::make_unique<UnaryExpression>(std::move(expression), op);
	}
	return GetPrimary();
}

std::unique_ptr<Expression> Parser::GetPrimary() {
	// get arguments (we omit for now)
	return GetAtom();
}

std::unique_ptr<Expression> Parser::GetAtom() {
	if (Match(Category::Identifier, Category::NumericLiteral)) {
		return std::make_unique<Atom>(GetPreviousToken());
	}
	return GetGrouping();
}

std::unique_ptr<Expression> Parser::GetGrouping() {
	// because this is the final rule in parsing an expression (before recursing back to the start), we perform additional checks
	if (Match(Category::LeftParenthesis)) {
		std::unique_ptr<Expression> expression = GetExpression();
		if (!Match(Category::RightParenthesis)) {
			throw std::runtime_error("expected right parenthesis after expression");
		}
		return std::make_unique<Grouping>(std::move(expression));
	}
	if (IsAtEnd()) {
		throw UnexpectedEndOfFile();
	}
	throw std::runtime_error("expected expression");
}


//IfStatement Parser::GetIfStatement() {
//	IfStatement new_if_statement{};
//	if (!Match("if")) {
//		throw std::runtime_error("\'if\' expected!");
//	}
//	new_if_statement.condition = GetExpression();
//	if (!Match("")) {
//
//	}
//}
//
//WhileStatement Parser::GetWhileStatement() {
//	
//}

/* Statement */

bool Statement::operator==(const Statement& other) const {
	if (typeid(other) != typeid(*this)) {
		return false;
	}
	return BaseCaseEquals(other);
}

bool Statement::BaseCaseEquals(const Statement& other) const {
	// "base case" for derived class calls to operator== (to prevent infinite recursion)
	other;
	return true;
}

/* Expression */

bool Expression::operator==(const Statement& other) const {
	if (typeid(other) != typeid(*this)) {
		return false;
	}
	const Expression* other_casted = dynamic_cast<const Expression*>(&other);
	return *this == *other_casted;
}

bool Expression::operator==(const Expression& other) const {
	return Statement::BaseCaseEquals(other);
}

/* BinaryExpression */

void BinaryExpression::Accept(const Visitor& visitor) const {
	visitor.VisitBinaryExpression(this);
}

bool BinaryExpression::operator==(const Statement& other) const {
	if (typeid(other) != typeid(*this)) {
		return false;
	}
	const BinaryExpression* other_casted = dynamic_cast<const BinaryExpression*>(&other);
	return *this == *other_casted;
}

bool BinaryExpression::operator==(const BinaryExpression& other) const {
	assert(other.left);
	assert(other.right);
	const Statement* left_virtual_downcast = dynamic_cast<const Statement*>(left.get());
	const Statement* right_virtual_downcast = dynamic_cast<const Statement*>(right.get());
	return *left_virtual_downcast == *other.left
		&& op == other.op
		&& *right_virtual_downcast == *other.right
		&& Expression::operator==(other);
}

/* UnaryExpression */

void UnaryExpression::Accept(const Visitor& visitor) const {
	visitor.VisitUnaryExpression(this);
}

bool UnaryExpression::operator==(const Statement& other) const {
	if (typeid(other) != typeid(*this)) {
		return false;
	}
	const UnaryExpression* other_casted = dynamic_cast<const UnaryExpression*>(&other);
	return *this == *other_casted;
}
bool UnaryExpression::operator==(const UnaryExpression& other) const {
	assert(other.expression);
	const Statement* exp_virtual_downcast = dynamic_cast<const Statement*>(expression.get());
	return *exp_virtual_downcast == *other.expression
		&& op == other.op
		&& Expression::operator==(other);
}

/* Grouping */

void Grouping::Accept(const Visitor& visitor) const {
	visitor.VisitGrouping(this);
}

bool Grouping::operator==(const Statement& other) const {
	if (typeid(other) != typeid(*this)) {
		return false;
	}
	const Grouping* other_casted = dynamic_cast<const Grouping*>(&other);
	return *this == *other_casted;
}
bool Grouping::operator==(const Grouping& other) const {
	assert(other.expression);
	const Statement* exp_virtual_downcast = dynamic_cast<const Statement*>(expression.get());
	return *exp_virtual_downcast == *other.expression
		&& Expression::operator==(other);
}

/* Atom */

void Atom::Accept(const Visitor& visitor) const {
	visitor.VisitAtom(this);
}

bool Atom::operator==(const Statement& other) const {
	if (typeid(other) != typeid(*this)) {
		return false;
	}
	const Atom* other_casted = dynamic_cast<const Atom*>(&other);
	return *this == *other_casted;
}
bool Atom::operator==(const Atom& other) const {
	return value == other.value
		&& Expression::operator==(other);
}

/* Visitor */

void Visitor::Visit(const Expression* expression) const {
	expression->Accept(*this);
}

void Visitor::VisitBinaryExpression(const BinaryExpression* binary_expression) const {
	std::cout << "visited binary expression: ";
	Visit(binary_expression->left.get());
	std::cout << std::get<std::string>(binary_expression->op.value) << std::endl;
	Visit(binary_expression->right.get());
	std::cout << std::endl;
}

void Visitor::VisitUnaryExpression(const UnaryExpression* unary_expression) const {
	std::cout << "visited unary expression: ";
	std::cout << std::get<std::string>(unary_expression->op.value) << std::endl;
	Visit(unary_expression->expression.get());
	std::cout << std::endl;
}

void Visitor::VisitAtom(const Atom* atom) const {
	std::cout << "visited atom: ";
	std::visit([&](const auto& val) {std::cout << val; }, atom->value.value);
	std::cout << std::endl;
}

void Visitor::VisitGrouping(const Grouping* grouping) const {
	std::cout << "visited grouping: " << std::endl;
	Visit(grouping->expression.get());
	std::cout << std::endl;
}