#include "pch.h"
#include "CppUnitTest.h"

#include <windows.h>
#include <limits>

#include "../pysub/lexer.cpp"
#include "../pysub/execution.cpp"
#include "../pysub/globals.cpp"
#include "../pysub/parser.cpp"
#include <vcpkg_installed/x64-windows/x64-windows/include/magic_enum/magic_enum.hpp>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// microsoft defines these as macros, leading to name clashes
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min

namespace Microsoft::VisualStudio::CppUnitTestFramework {
	
	// equality asserts require a template specialization for ToString<> for error message purposes.
	// the process of conversion: creating a wstringstream and using << operator.
	// to add a new type: (1) overload wstringstream << operator, (2) add template specialization.

	std::string ErrorToString(DWORD error) {
		// stolen from stackoverflow: https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
		if (!error) {
			return {};
		}
		LPSTR message_buffer{};
		size_t message_size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, error, 0, (LPSTR)&message_buffer, 0, nullptr);
		if (!message_size) {
			throw GetLastError();	// can't be a message, sadly (or it could loop forever)
		}
		std::string message(message_buffer, message_size);
		LocalFree(message_buffer);
		return message;
	}

	std::wstring ToWString(const std::string_view utf8_string) {
		// wstringstream does not have default behavior for std::string (but it does for other primitive types).
		// there is no non-deprecated conversion function in the std library (as of C++20), so windows api is used.
		
		// a null-terminated string is required to determine string length
		int buffer_size = MultiByteToWideChar(CP_UTF8, 0, utf8_string.data(), -1, nullptr, 0);
		if (!buffer_size) {
			throw std::exception(ErrorToString(GetLastError()).c_str());
		}
		std::wstring converted(buffer_size, 0);
		MultiByteToWideChar(CP_UTF8, 0, utf8_string.data(), -1, converted.data(), buffer_size);
		converted.pop_back();	// remove null terminator added by c_str call
		return converted;
	}

	std::wstringstream& operator<<(std::wstringstream& stream, const std::string& string) {
		stream << ToWString(string);
		return stream;
	}

	//std::wstringstream& operator<<(std::wstringstream& stream, const ValueType& value) {
	//	//if (std::holds_alternative<std::string>(value)) {
	//	//	stream << ToWString(std::get<std::string>(value));
	//	//}
	//	//else {
	//	//	assert(std::holds_alternative<int>(value));
	//	//	stream << ToString(std::get<int>(value));
	//	//}
	//	return stream;
	//}

	std::wstringstream& operator<<(std::wstringstream& stream, const Token& token) {
		stream << '{';
		stream << ToWString(magic_enum::enum_name(token.category));
		stream << ',';
		std::visit([&](auto val) {stream << val; }, token.value);
		stream << '}';
		return stream;
	}

	template <typename T>
	std::wstringstream& operator<<(std::wstringstream& stream, const std::vector<T>& vector) {
		for (auto iter = std::begin(vector); iter != std::end(vector); ++iter) {
			stream << *iter;
			if (iter != --std::end(vector)) {
				stream << ',';
			}
		}
		return stream;
	}

	std::wstringstream& operator<<(std::wstringstream& stream, const std::unique_ptr<Statement>& statement) {
		Statement* statement_ptr = statement.get();
		// atom
		Atom* atom_ptr = dynamic_cast<Atom*>(statement_ptr);
		if (atom_ptr) {
			stream << atom_ptr;
			return stream;
		}
		Grouping* grouping_ptr = dynamic_cast<Grouping*>(statement_ptr);
		if (grouping_ptr) {
			stream << grouping_ptr;
			return stream;
		}
		Expression* expression_ptr = dynamic_cast<Expression*>(statement_ptr);
		if (expression_ptr) {
			// the subclasses of Expression are handled elsewhere
			stream << expression_ptr;
			return stream;
		}
		throw std::invalid_argument("Statement cannot be printed!");
	}

	std::wstringstream& operator<<(std::wstringstream& stream, Atom* atom) {
		stream << atom->value;
		return stream;
	}

	std::wstringstream& operator<<(std::wstringstream& stream, Grouping* grouping) {
		stream << grouping->expression;
		return stream;
	}

	std::wstringstream& operator<<(std::wstringstream& stream, Expression* expression) {
		UnaryExpression* unary_ptr = dynamic_cast<UnaryExpression*>(expression);
		if (unary_ptr) {
			stream << unary_ptr;
			return stream;
		}
		BinaryExpression* binary_ptr = dynamic_cast<BinaryExpression*>(expression);
		if (binary_ptr) {
			stream << binary_ptr;
		}
		return stream;
	}

	std::wstringstream& operator<<(std::wstringstream& stream, UnaryExpression* unary) {
		stream << std::string{ "exp: " };
		stream << unary->expression;
		stream << ',';
		stream << std::string{ "op: " };
		stream << unary->op;
		return stream;
	}

	std::wstringstream& operator<<(std::wstringstream& stream, BinaryExpression* binary) {
		stream << std::string{ "left: " };
		stream << binary->left;
		stream << ',';
		stream << std::string{ "op: " };
		stream << binary->op;
		stream << ',';
		stream << std::string{ "right: " };
		stream << binary->right;
		return stream;
	}

	template <typename T>
	std::wstring ConstructWideString(const T& narrow_type) {
		// helper for ToString template specializations
		std::wstringstream stream{};
		stream << narrow_type;
		return stream.str();
	}

	// ToString template specializations
	template<> inline std::wstring ToString<Token>(const Token& token) {
		return ConstructWideString(token);
	}

	template<> inline std::wstring ToString<std::vector<Token>>(const std::vector<Token>& token) {
		return ConstructWideString(token);
	}

	template<> inline std::wstring ToString<std::vector<std::unique_ptr<Statement>>>(const std::vector<std::unique_ptr<Statement>>& statements) {
		return ConstructWideString(statements);
	}

	template<> inline std::wstring ToString<std::unique_ptr<Statement>>(const std::unique_ptr<Statement>& statement) {
		return ConstructWideString(statement);
	}
}

namespace tests
{
	TEST_CLASS(LexicalAnalyserTest)
	{
	public:
		TEST_METHOD(NumericLiteralValid) {
			std::vector<Token> actual = Lexer::GenerateTokens("012345");
			std::vector<Token> expected{ {12345, Category::NumericLiteral} };
			Assert::AreEqual(expected, actual);

			actual = Lexer::GenerateTokens("54321(");
			expected = { {54321, Category::NumericLiteral}, {"", Category::LeftParenthesis}};
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(NumericLiteralInvalid) {
			auto func = []() {Lexer::GenerateTokens("1prince"); };
			Assert::ExpectException<std::invalid_argument>(func);
		}
		TEST_METHOD(NumericLiteralOutOfRange) {
			// max
			std::vector<Token> actual = Lexer::GenerateTokens(std::to_string(std::numeric_limits<int>::max()));
			std::vector<Token> expected{ {std::numeric_limits<int>::max(), Category::NumericLiteral} };
			Assert::AreEqual(expected, actual);

			// min
			// this is actually max + 1 (because we negative sign becomes a token)
			actual = Lexer::GenerateTokens(std::to_string(std::numeric_limits<int>::min() + 1));
			expected = { {"-", Category::ArithmeticOperator}, { std::numeric_limits<int>::max(), Category::NumericLiteral }};
			Assert::AreEqual(expected, actual);

			// above
			auto above = []() {Lexer::GenerateTokens(std::to_string(static_cast<long>(std::numeric_limits<int>::max()) + 1) ); };
			Assert::ExpectException<std::out_of_range>(above);

			// below
			auto below = []() {Lexer::GenerateTokens(std::to_string(static_cast<long>(std::numeric_limits<int>::min()))); };
			Assert::ExpectException<std::out_of_range>(below);
		}
		TEST_METHOD(IdentifierValid)
		{
			std::vector<Token> actual = Lexer::GenerateTokens("prince");
			std::vector<Token> expected{ {"prince", Category::Identifier}};
			Assert::AreEqual(expected, actual);

			actual = Lexer::GenerateTokens("_prince23");
			expected = { {"_prince23", Category::Identifier} };
			Assert::AreEqual(expected, actual);

			actual = Lexer::GenerateTokens("prince+");
			expected = { {"prince", Category::Identifier}, {"+", Category::ArithmeticOperator}};
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(IdentifierInvalid) {
			auto func = []() {Lexer::GenerateTokens("prince~"); };
			Assert::ExpectException<std::invalid_argument>(func);
		}
		TEST_METHOD(IdentifierCategory) {
			std::vector<Token> actual = Lexer::GenerateTokens("print");
			std::vector<Token> expected{ {"print", Category::Keyword} };
			Assert::AreEqual(expected, actual);

			actual = Lexer::GenerateTokens("and");
			expected = { {"and", Category::LogicalOperator} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(StringLiteralValid) {
			std::vector<Token> actual = Lexer::GenerateTokens("\"lit\'eral\"");
			std::vector<Token> expected{ {"lit\'eral", Category::StringLiteral} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(StringLiteralInvalid) {
			auto one_quote = []() {Lexer::GenerateTokens("\""); };
			Assert::ExpectException<std::invalid_argument>(one_quote);

			auto not_matching_quote = []() {Lexer::GenerateTokens("\"abc\'"); };
			Assert::ExpectException<std::invalid_argument>(not_matching_quote);
		}
		TEST_METHOD(CommentValid) {
			std::vector<Token> actual = Lexer::GenerateTokens("#blorp \t1234");
			std::vector<Token> expected{ {"blorp \t1234", Category::Comment} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(ParenthesesValid) {
			std::vector<Token> actual = Lexer::GenerateTokens("()");
			std::vector<Token> expected{ {"", Category::LeftParenthesis}, {"", Category::RightParenthesis} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(ColonValid) {
			std::vector<Token> actual = Lexer::GenerateTokens(":");
			std::vector<Token> expected{ {"", Category::Colon} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(CommaValid) {
			std::vector<Token> actual = Lexer::GenerateTokens(",");
			std::vector<Token> expected{ {"", Category::Comma} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(RelationalAssignmentValid) {
			std::vector<Token> actual = Lexer::GenerateTokens("<<=>>=");
			std::vector<Token> expected{ {"<", Category::RelationalOperator}, { "<=", Category::RelationalOperator }, {">", Category::RelationalOperator}, {">=", Category::RelationalOperator} };
			Assert::AreEqual(expected, actual);

			actual = Lexer::GenerateTokens("=");
			expected = { {"", Category::AssignmentOperator} };
			Assert::AreEqual(expected, actual);

			actual = Lexer::GenerateTokens("==");
			expected = { {"==", Category::RelationalOperator} };
			Assert::AreEqual(expected, actual);

			actual = Lexer::GenerateTokens("!=");
			expected = { {"!=", Category::RelationalOperator} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(RelationalAssignmentInvalid) {
			auto func = []() {Lexer::GenerateTokens("!"); };
			Assert::ExpectException<std::invalid_argument>(func);
		}
		TEST_METHOD(ArithmeticOperatorValid) {
			std::vector<Token> actual = Lexer::GenerateTokens("+-*/%");
			std::vector<Token> expected{ {"+", Category::ArithmeticOperator}, {"-", Category::ArithmeticOperator},  {"*", Category::ArithmeticOperator},
				{"/", Category::ArithmeticOperator}, {"%", Category::ArithmeticOperator}};
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(IndentValid) {
			std::vector<Token> actual = Lexer::GenerateTokens("\t \t");
			std::vector<Token> expected{ {"", Category::Indent}, {"", Category::Indent}, {"", Category::Indent} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(InvalidCharacter) {
			auto func = []() {Lexer::GenerateTokens("^"); };
			Assert::ExpectException<std::invalid_argument>(func);
		}
		TEST_METHOD(IndentDedentValid) {
			// equal indent
			std::vector<Token> actual = Lexer::GenerateTokens("\t\n\t");
			std::vector<Token> expected{ {"", Category::Indent}, {"", Category::Newline }};
			Assert::AreEqual(expected, actual);

			// multiple indents, one dedent
			actual = Lexer::GenerateTokens("\t\t\n\t");
			expected = { {"", Category::Indent}, {"", Category::Indent}, {"", Category::Newline}, {"", Category::Dedent}};
			Assert::AreEqual(expected, actual);

			// multiple dedents
			actual = Lexer::GenerateTokens("\t\t\n123");
			expected = { {"", Category::Indent}, {"", Category::Indent}, {"", Category::Newline}, {"", Category::Dedent}, {"", Category::Dedent}, { 123, Category::NumericLiteral } };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(MultipleLines) {
			std::string input{ "if (1+1\t == 2):\n\tprint(\"hello world!\")" };
			std::vector<Token> actual = Lexer::GenerateTokens(input);
			std::vector<Token> expected{
				{"if", Category::Keyword}, {"", Category::LeftParenthesis}, {1, Category::NumericLiteral},
				{"+", Category::ArithmeticOperator}, {1, Category::NumericLiteral}, {"==", Category::RelationalOperator}, {2, Category::NumericLiteral},
				{"", Category::RightParenthesis}, {"", Category::Colon}, {"", Category::Newline},
				{"", Category::Indent}, {"print", Category::Keyword}, {"", Category::LeftParenthesis}, {"hello world!", Category::StringLiteral}, {"", Category::RightParenthesis}
			};
			Assert::AreEqual(expected, actual);
		}
	};
	TEST_CLASS(ExecutionTest) {
	public:
		TEST_METHOD(ReadFile) {
			std::string expected = "#PROVIDED EXAMPLE\n"
				"print(\"hello\",\'world!\')\n"
				"x=3\n"
				"\n"
				"\n"
				"print(\"X is equal to:\",x)\n"
				"y = int(input(\"Enter value for y: \"))\n"
				"print(\"Y + 3 is:\", y + 3)";
			// test binary is in pysub/x64/Debug
			FileExecution file("../../files_for_testing/read_file_test.py");
			std::string actual = file.GetFileString();
			Assert::AreEqual(expected, actual);
		}
	};
	TEST_CLASS(ParserTest) {
	public:
		TEST_METHOD(EmptyValid) {
			std::vector<Token> tokens{};
			Parser p(tokens);
			auto tree = p.BuildTree();
			Assert::IsTrue(tree->statements.empty());
		}
		TEST_METHOD(NewlineValid) {
			//newlines and comments
			std::vector<Token> tokens{
				Token{.category = Category::Newline},
				Token{.category = Category::Comment}
			};
			Parser p(tokens);
			auto tree = p.BuildTree();
			Assert::IsTrue(tree->statements.empty());
		}
		TEST_METHOD(SingleAtomValid) {
			Token numeric_atom = Token{ .value = 1, .category = Category::NumericLiteral };
			std::vector<Token> tokens{
				numeric_atom
			};
			Parser p(tokens);
			auto tree = p.BuildTree();

			std::vector<std::unique_ptr<Statement>> res{};
			res.push_back(std::make_unique<Atom>(numeric_atom));
			
			Assert::AreEqual(tree->statements, res);

			//Token identifier_atom = Token{ .value = "variable", .category = Category::Identifier};
			//std::vector<Token> identifier_atom_tokens{
			//	identifier_atom
			//};
			//Parser parser_identifier(identifier_atom_tokens);
			//auto identifier_atom_tree = parser_identifier.BuildTree();
			//Assert::AreEqual(identifier_atom_tree->statements, { std::move(std::make_unique<Atom>(identifier_atom)) });
		}
		//TEST_METHOD(SingleAtomInvalid) {
		//	Token invalid_atom = Token{ .value = "+", .category = Category::ArithmeticOperator};
		//	std::vector<Token> tokens{
		//		invalid_atom
		//	};
		//	Parser p(tokens);
		//	auto func = [&]() {auto res = p.BuildTree(); };
		//	Assert::ExpectException<std::runtime_error>(func);
		//}
	};
}

#pragma pop_macro("max")
#pragma pop_macro("min")