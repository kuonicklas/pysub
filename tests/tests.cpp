#include "pch.h"
#include "CppUnitTest.h"

#include <windows.h>
#include <limits>

#include "../pysub/lexical_analyzer.cpp"
#include "../pysub/input_parser.cpp"
#include "../pysub/execution.cpp"
#include <vcpkg_installed/x64-windows/x64-windows/include/magic_enum/magic_enum.hpp>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// microsoft defines these as macros, leading to name clashes
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min

namespace Microsoft::VisualStudio::CppUnitTestFramework {
	
	// equality asserts require a template specialization for ToString<> for error message purposes.
	// the process of conversion: using creating a wstringstream and using << operator.
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

	std::wstring ToWString(const std::string& utf8_string) {
		// wstringstream does not have default behavior for std::string (but it does for other primitive types).
		// there is no non-deprecated conversion function in the std library (as of C++20), so windows api is used.
		// input is const string& (rather than string_view) because a null-terminated string (which string::c_str provides) is required if length is determined by function.
		int buffer_size = MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, nullptr, 0);
		if (!buffer_size) {
			throw std::exception(ErrorToString(GetLastError()).c_str());
		}
		std::wstring converted(buffer_size, 0);
		MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, converted.data(), buffer_size);
		converted.pop_back();	// remove null terminator added by c_str call
		return converted;
	}

	std::wstringstream& operator<<(std::wstringstream& stream, const std::string& string) {
		stream << ToWString(string);
		return stream;
	}

	std::wstringstream& operator<<(std::wstringstream& stream, const Token& token) {
		// convert to wstring
		stream << '{';
		stream << ToWString(magic_enum::enum_name(token.category).data());
		stream << ',';
		std::visit([&](auto val) {stream << val; }, token.value);
		stream << '}';
		return stream;
	}

	template <typename T>
	std::wstringstream& operator<<(std::wstringstream& stream, const std::vector<T>& token_line) {
		for (auto iter = std::begin(token_line); iter != std::end(token_line); ++iter) {
			stream << *iter;
			if (iter != --std::end(token_line)) {
				stream << ',';
			}
		}
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
}

namespace tests
{
	TEST_CLASS(LexicalAnalyserTest)
	{
	public:
		TEST_METHOD(NumericLiteralValid) {
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens("012345");
			std::vector<Token> expected{ {12345, Category::NumericLiteral} };
			Assert::AreEqual(expected, actual);

			actual = LexicalAnalyzer::GenerateTokens("54321(");
			expected = { {54321, Category::NumericLiteral}, {"", Category::LeftParenthesis}};
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(NumericLiteralInvalid) {
			auto func = []() {LexicalAnalyzer::GenerateTokens("1prince"); };
			Assert::ExpectException<std::invalid_argument>(func);
		}
		TEST_METHOD(NumericLiteralOutOfRange) {
			// max
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens(std::to_string(std::numeric_limits<int>::max()));
			std::vector<Token> expected{ {std::numeric_limits<int>::max(), Category::NumericLiteral} };
			Assert::AreEqual(expected, actual);

			// min
			// this is actually max + 1 (because we negative sign becomes a token)
			actual = LexicalAnalyzer::GenerateTokens(std::to_string(std::numeric_limits<int>::min() + 1));
			expected = { {"-", Category::ArithmeticOperator}, { std::numeric_limits<int>::max(), Category::NumericLiteral }};
			Assert::AreEqual(expected, actual);

			// above
			auto above = []() {LexicalAnalyzer::GenerateTokens(std::to_string(static_cast<long>(std::numeric_limits<int>::max()) + 1) ); };
			Assert::ExpectException<std::out_of_range>(above);

			// below
			auto below = []() {LexicalAnalyzer::GenerateTokens(std::to_string(static_cast<long>(std::numeric_limits<int>::min()))); };
			Assert::ExpectException<std::out_of_range>(below);
		}
		TEST_METHOD(IdentifierValid)
		{
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens("prince");
			std::vector<Token> expected{ {"prince", Category::Identifier}};
			Assert::AreEqual(expected, actual);

			actual = LexicalAnalyzer::GenerateTokens("_prince23");
			expected = { {"_prince23", Category::Identifier} };
			Assert::AreEqual(expected, actual);

			actual = LexicalAnalyzer::GenerateTokens("prince+");
			expected = { {"prince", Category::Identifier}, {"+", Category::ArithmeticOperator}};
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(IdentifierInvalid) {
			auto func = []() {LexicalAnalyzer::GenerateTokens("prince~"); };
			Assert::ExpectException<std::invalid_argument>(func);
		}
		TEST_METHOD(IdentifierCategory) {
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens("print");
			std::vector<Token> expected{ {"print", Category::Keyword} };
			Assert::AreEqual(expected, actual);

			actual = LexicalAnalyzer::GenerateTokens("and");
			expected = { {"and", Category::LogicalOperator} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(StringLiteralValid) {
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens("\"lit\'eral\"");
			std::vector<Token> expected{ {"lit\'eral", Category::StringLiteral} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(StringLiteralInvalid) {
			auto one_quote = []() {LexicalAnalyzer::GenerateTokens("\""); };
			Assert::ExpectException<std::invalid_argument>(one_quote);

			auto not_matching_quote = []() {LexicalAnalyzer::GenerateTokens("\"abc\'"); };
			Assert::ExpectException<std::invalid_argument>(not_matching_quote);
		}
		TEST_METHOD(CommentValid) {
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens("#blorp \t1234");
			std::vector<Token> expected{ {"blorp \t1234", Category::Comment} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(ParenthesesValid) {
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens("()");
			std::vector<Token> expected{ {"", Category::LeftParenthesis}, {"", Category::RightParenthesis} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(ColonValid) {
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens(":");
			std::vector<Token> expected{ {"", Category::Colon} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(CommaValid) {
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens(",");
			std::vector<Token> expected{ {"", Category::Comma} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(RelationalAssignmentValid) {
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens("<<=>>=");
			std::vector<Token> expected{ {"<", Category::RelationalOperator}, { "<=", Category::RelationalOperator }, {">", Category::RelationalOperator}, {">=", Category::RelationalOperator} };
			Assert::AreEqual(expected, actual);

			actual = LexicalAnalyzer::GenerateTokens("=");
			expected = { {"", Category::AssignmentOperator} };
			Assert::AreEqual(expected, actual);

			actual = LexicalAnalyzer::GenerateTokens("==");
			expected = { {"==", Category::RelationalOperator} };
			Assert::AreEqual(expected, actual);

			actual = LexicalAnalyzer::GenerateTokens("!=");
			expected = { {"!=", Category::RelationalOperator} };
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(RelationalAssignmentInvalid) {
			auto func = []() {LexicalAnalyzer::GenerateTokens("!"); };
			Assert::ExpectException<std::invalid_argument>(func);
		}
		TEST_METHOD(ArithmeticOperatorValid) {
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens("+-*/%");
			std::vector<Token> expected{ {"+", Category::ArithmeticOperator}, {"-", Category::ArithmeticOperator},  {"*", Category::ArithmeticOperator},
				{"/", Category::ArithmeticOperator}, {"%", Category::ArithmeticOperator}};
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(IndentValid) {
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens("\t \t");
			std::vector<Token> expected{ {"", Category::Indent}};
			Assert::AreEqual(expected, actual);
		}
		TEST_METHOD(InvalidCharacter) {
			auto func = []() {LexicalAnalyzer::GenerateTokens("^"); };
			Assert::ExpectException<std::invalid_argument>(func);
		}
		TEST_METHOD(MultipleLines) {
			std::string input{ "if (1+1\t == 2):\n\tprint(\"hello world!\")" };
			std::vector<Token> actual = LexicalAnalyzer::GenerateTokens(input);
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
}

#pragma pop_macro("max")
#pragma pop_macro("min")