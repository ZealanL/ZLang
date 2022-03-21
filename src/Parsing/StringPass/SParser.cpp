#include "SParser.h"

int SParser::GetCharType(char c) {
	if (isalnum(c) || c == '_')
		return SToken::CHARTYPE_ALNUM;
	else if (isblank(c) || c == '\n' || c == '\r')
		return SToken::CHARTYPE_BLANK;

	constexpr char opSymbols[] = "+-/*%=<>!";
	for (char symbol : opSymbols)
		if (symbol == c)
			return SToken::CHARTYPE_OP_SYMBOL;

	return SToken::CHARTYPE_SYMBOL;
}

vector<SToken> SParser::GetStringTokens(const string& lineStr, int lineNum) {
	vector<SToken> tokens;

	string currentTokenBuffer;
	int tokenBufferCharType = SToken::CHARTYPE_INVALID;

	char lastChar = 0;
	int lastCharType = SToken::CHARTYPE_INVALID;

	int charIndex = 0;
	const auto fnFlushTokenBuffer = [=, &tokens, &currentTokenBuffer]() {
		if (!currentTokenBuffer.empty()) {
			ZLT_Str str = ZLT_Str(currentTokenBuffer);
			SToken newToken = SToken(str, tokenBufferCharType, lineNum, charIndex);
			tokens.push_back(newToken);
			currentTokenBuffer.clear();
		}
	};

	for (; charIndex < lineStr.size(); charIndex++) {
		char c = lineStr[charIndex];
		auto type = GetCharType(c);
		bool isDigit = isdigit(c);

		// Inline comment encountered, we're done here
		if (FW::S::InTextMatch("//", lineStr.c_str(), charIndex))
			break;

		bool bypassFlush = false;

		bool inNumber = !currentTokenBuffer.empty() && (isdigit(currentTokenBuffer[0]) || currentTokenBuffer[0] == '.');
		if (inNumber && (lastChar == '.' || c == '.'))
			bypassFlush = true;

		// Decide if we should flush or not
		if ((
			lastCharType == SToken::CHARTYPE_SYMBOL // Symbols are always single-split
			|| lastCharType != type // Different type
			) && !bypassFlush
			) {
			fnFlushTokenBuffer();
			goto finish;
		}

	finish:
		if (!isblank(c)) {
			if (currentTokenBuffer.empty())
				tokenBufferCharType = type;
			currentTokenBuffer += c;
		}
		lastChar = c;
		lastCharType = type;
	}

	fnFlushTokenBuffer();

	return tokens;
}