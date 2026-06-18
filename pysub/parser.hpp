#ifndef PARSER_HPP
#define PARSER_HPP

#include "globals.hpp"

#include <vector>
#include <memory>
#include <cassert>

/*
steps to creating a new class:
constructor if needed
	any unique_ptr must be checked for not null
	unique_ptrs must be moved (just the rules)
override for bool operator==(const Statement&) const:
	check the types
	cast rhs into the same type, then compare
define comparison function for self, that compares members: bool operator==(const T&) const
	check members first, then call next base class's operator== to compare its own members.
		and so on up the chain, stopping at Statement::BaseCaseEquals, which prevents a virtual dispatch back down (creating an infinite loop).
			a downcast to Statement* is necessary to prevent C++ from overload matching a non-virtual memberwise operator== function.
			e.g. unique_ptr<Expression> can contain BinaryExpression, but simply doing *ptr == *other_ptr means 'ptr' has static type of Expression, ...
				...and because 'other_ptr' (BinaryExpression) is one conversion away from Expression (while Statement is two), the non-virtual memberwise compare function is selected.
			(Statement only has a virtual operator==, so it necessarily dispatches properly).
to make comparisons easier, assert that all unique_ptr (lhs or rhs) are not null (the former is done in constructor).
*/

class Visitor;

class Statement {
public:
	virtual ~Statement() = default;
	virtual void Accept(const Visitor& visitor) const = 0;
	virtual bool operator==(const Statement& other) const;
protected:
	bool BaseCaseEquals(const Statement& other) const;
};

class Expression : public Statement {
public:
	bool operator==(const Statement& other) const override;
	bool operator==(const Expression& other) const;
};

class BinaryExpression : public Expression {
public:
	std::unique_ptr<Expression> left;
	Token op;
	std::unique_ptr<Expression> right;

	explicit BinaryExpression(std::unique_ptr<Expression>&& _left, Token _op, std::unique_ptr<Expression>&& _right) :
		left(std::move(_left)), op(_op), right(std::move(_right)) {
		assert(left);
		assert(right);
	};
	void Accept(const Visitor& visitor) const override;
	bool operator==(const Statement& other) const override;
	bool operator==(const BinaryExpression& other) const;
};

class UnaryExpression : public Expression {
public:
	std::unique_ptr<Expression> expression;
	Token op;

	explicit UnaryExpression(std::unique_ptr<Expression>&& _exp, Token _op) : expression(std::move(_exp)), op(_op) {
		assert(expression);
	};
	void Accept(const Visitor& visitor) const override;
	bool operator==(const Statement& other) const override;
	bool operator==(const UnaryExpression& other) const;
};

class Grouping : public Expression {
public:
	std::unique_ptr<Expression> expression;

	explicit Grouping(std::unique_ptr<Expression>&& _exp) : expression(std::move(_exp)) {
		assert(expression);
	};
	void Accept(const Visitor& visitor) const override;
	bool operator==(const Statement& other) const override;
	bool operator==(const Grouping& other) const;
};

class Atom : public Expression {
public:
	Token value;

	explicit Atom(const Token& token) : value(token) {};
	void Accept(const Visitor& visitor) const override;
	bool operator==(const Statement& other) const override;
	bool operator==(const Atom& other) const;
};

struct AST {
	std::vector<std::unique_ptr<Statement>> statements{};
};

// credit to Robert Nystrom's "Crafting Interpreters" book for the basic structure:
// https://craftinginterpreters.com/
class Parser {
public:
	explicit Parser(const std::vector<Token>& _tokens) : tokens(_tokens){
		curr_token = std::cbegin(tokens);
	}
	[[nodiscard]] std::unique_ptr<AST> BuildTree();
	void CheckSyntax();

private:
	const std::vector<Token>& tokens;
	std::vector<Token>::const_iterator curr_token;

	std::unique_ptr<AST> BuildAST();
	std::vector<std::unique_ptr<Statement>> GetStatements();
	std::unique_ptr<Statement> GetStatement();
	//std::unique_ptr<CompoundStatement> GetCompoundStatement();
	std::unique_ptr<Statement> GetSimpleStatement();
	std::unique_ptr<Expression> GetExpression();
	std::unique_ptr<Expression> GetConjunction();
	std::unique_ptr<Expression> GetInversion();
	std::unique_ptr<Expression> GetComparison();
	std::unique_ptr<Expression> GetSum();
	std::unique_ptr<Expression> GetTerm();
	std::unique_ptr<Expression> GetFactor();
	std::unique_ptr<Expression> GetPrimary();
	std::unique_ptr<Expression> GetAtom();
	std::unique_ptr<Expression> GetGrouping();
	/*IfStatement GetIfStatement();
	WhileStatement GetWhileStatement();*/

	bool Match(auto&&... input);
	void IncrementToken();
	bool Check(std::string_view s) const;
	bool Check(Category category) const;
	bool Check(const Token& compare_token) const;
	bool IsAtEnd() const;
	Token GetPreviousToken() const;
};

class Visitor {
public:
	void Visit(const Expression* expression) const;
	void VisitBinaryExpression(const BinaryExpression* binary_expression) const;
	void VisitUnaryExpression(const UnaryExpression* unary_expression) const;
	void VisitAtom(const Atom* atom) const;
	void VisitGrouping(const Grouping* grouping) const;
};

#endif