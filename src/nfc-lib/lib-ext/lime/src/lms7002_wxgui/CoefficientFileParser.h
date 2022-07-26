#ifndef COEFFICIENT_FILE_PARSER_H
#define COEFFICIENT_FILE_PARSER_H

/**
@file	CoefficientFileParser.h
@author	Lime Microsystems
@brief	Just group parser functions into the same namespace.
*/

namespace Parser
{
	bool IsBlank(char);
	bool IsDigit(char);
	int getValue(FILE *, float *);
	int getcoeffs(const char *, float *, int);
	int getcoeffs2(const char *, float *, float *, int);
	void saveToFile(const char *, const float *, int);
}

#endif // COEFFICIENT_FILE_PARSER_H
