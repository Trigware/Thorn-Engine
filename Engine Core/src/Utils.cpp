#include "Utils.h"

namespace ThornEngine {

char StrUtils::GetCase(char ch, bool toLower) {
	char lowerBound = 'a', highBound = 'z';
	if (toLower) {
		lowerBound -= capitalDiff;
		highBound -= capitalDiff;
	}

	bool inAffectedRange = ch >= lowerBound && ch <= highBound;
	if (!inAffectedRange) return ch;
	return toLower ? ch + capitalDiff : ch - capitalDiff;
}

std::string StrUtils::GetCase(const std::string& str, bool toLower) {
	std::string result = "";
	for (int i = 0; i < str.size(); i++)
		result += GetCase(str[i], toLower);
	return result;
}

std::string StrUtils::MakeSpaces(int count) {
	std::string result = "";
	for (int i = 0; i < count; i++) result += ' ';
	return result;
}

}