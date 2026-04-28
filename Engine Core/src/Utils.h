#pragma once
#include <string>

namespace ThornEngine {

/// @brief Class containing exclusively static functions for string operations.
class StrUtils {
public:
	static std::string ToLower(const std::string& str) { return GetCase(str, true); }
	static std::string ToUpper(const std::string& str) { return GetCase(str, false); }
	static inline bool IsNumber(char ch) { return ch >= '0' && ch <= '9'; }
	static inline bool IsLetter(char ch) { return ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z'; }
	static inline bool IsAlphaNum(char ch) { return IsLetter(ch) || IsNumber(ch); }
	static inline bool IsSymbol(char ch) { return !IsNumber(ch) && !IsLetter(ch); }
	static inline char ToLower(char ch) { return GetCase(ch, true); }
	static inline char ToUpper(char ch) { return GetCase(ch, false); }
	static char GetCase(char ch, bool toLower);
	static std::string GetCase(const std::string& str, bool toLower);
	static std::string MakeSpaces(int count);
private:
	static const int capitalDiff = 'a' - 'A';
};

}