#ifndef PARSER_HPP
#define PARSER_HPP

#include "globals.hpp"

#include <vector>
#include <memory>

//struct Expression {
//
//};
//
//struct IfStatement {
//	Expression condition;
//	std::vector<Statement> block;
//};
//
//struct WhileStatement {
//
//};
//
//using CompoundStatement = std::variant<IfStatement, WhileStatement>;
//
//struct SimpleStatement {
//
//};
//
//using Statement = std::variant<CompoundStatement, SimpleStatement>;

/*
	either:
		-add a bumfuck virtual func (or see if one will be necessary anyways)
			-we may need to use the visitor pattern from the book anyways, i.e. a visitor class with various functions, and an accept() func for each struct that dispatches to the appropriate func
		-use variant:
			-not possible with raw data; only pointers.

	using ParsedType = std::variant<
		std::unique_ptr<Statement*>,
		std::unique_ptr<BinaryExpression*>,
		std::unique_ptr<UnaryExpression*>,
		std::unique_ptr<Grouping*>,
		std::unique_ptr<Atom>
	>;
*/

class Visitor;

struct Statement {
	virtual void Accept(const Visitor* visitor) const = 0;
	virtual ~Statement() = default;
};

struct Expression : Statement {
};

struct BinaryExpression : Expression {
	std::unique_ptr<Expression> left;
	Token op;
	std::unique_ptr<Expression> right;

	explicit BinaryExpression(std::unique_ptr<Expression> l, Token op, std::unique_ptr<Expression> r) : left(std::move(l)), op(op), right(std::move(r)) {};
	void Accept(const Visitor* visitor) const override;
};

struct UnaryExpression : Expression {
	std::unique_ptr<Expression> expression;
	Token op;

	explicit UnaryExpression(std::unique_ptr<Expression> exp, Token op) : expression(std::move(exp)), op(op) {};
	void Accept(const Visitor* visitor) const override;
};

struct Grouping : Expression {
	std::unique_ptr<Expression> expression;

	explicit Grouping(std::unique_ptr<Expression> exp) : expression(std::move(exp)) {};
	void Accept(const Visitor* visitor) const override;
};

struct Atom : Expression {
	Token value;

	explicit Atom(const Token& token) : value(token) {};
	void Accept(const Visitor* visitor) const override;
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