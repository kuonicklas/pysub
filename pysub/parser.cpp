#include "parser.hpp"

#include <stdexcept>

// return true and consume one token if curr token matches any of the inputs
bool Parser::Match(auto&&... input) {
	bool match_found = false;
	(
		[&] {
			if (!match_found) {
				match_found = Check(input) || match_found;
			}
		}() && ...
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

std::unique_ptr<AST> Parser::BuildAST() {
	std::unique_ptr<AST> new_ast = std::make_unique<AST>();
	if (!IsAtEnd()) {
		new_ast->statements = GetStatements();
	}
	return new_ast;
}

std::vector<std::unique_ptr<Statement>> Parser::GetStatements() {
	std::vector<std::unique_ptr<Statement>> statements{};
	statements.push_back(GetStatement());
	while (!IsAtEnd()) {
		statements.push_back(GetStatement());
	}
	return statements;
}

std::unique_ptr<Statement> Parser::GetStatement() {
	//if (Check("if") || Check("while")) {
	//	return GetCompoundStatement();
	//}
	return GetSimpleStatement();
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
	auto right = GetConjunction();
	while (Match(Token{ .value = "or", .category = Category::LogicalOperator })) {
		dynamic_cast<BinaryExpression*>(right.get()) = Token{ .value = "or", .category = Category::LogicalOperator };
		right->right = GetExpression();
	}
	return right;
}

std::unique_ptr<Expression> GetConjunction() {

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