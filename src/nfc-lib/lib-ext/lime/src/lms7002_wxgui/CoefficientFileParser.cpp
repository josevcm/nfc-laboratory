/**
@file	CoefficientFileParser.cpp
@author	Lime Microsystems
@brief	Coefficient file parser functions
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "CoefficientFileParser.h"
#include <fstream>
using namespace std;

// ***************************************************************
// Check if 'c' is blank character.
// ***************************************************************
bool Parser::IsBlank(char c)
{
	char blankchar[] = ", \t\n\r";
	for(unsigned i=0; i<strlen(blankchar); i++)
			if(c == blankchar[i] ) return(true);

	return(false);
}

// ***************************************************************
// Check if 'c' is a digit.
// ***************************************************************
bool Parser::IsDigit(char c)
{
	char digit[] = ".0123456789+-";
	for(unsigned i=0; i<strlen(digit); i++)
			if(c == digit[i] ) return(true);
	return(false);
}

// ***************************************************************
//	Get integer value from the file
//	Returns:
//		0 upon success,
//		-1 if EOF or
//		-2 if syntax error
// ***************************************************************
int Parser::getValue(FILE *fp, float *v)
{
	char c, c1, str[256];
	int i, opencomments;

	/* Skip blanks, tabs and new lines */
	while( IsBlank( c = fgetc(fp) ) );

	/* Its end of file, nothing to read */
	if( c == EOF ) return(EOF);

	if( c == '/') {			/* check for comments */
		c1 = fgetc(fp);
		if( c1 == '/') {	/* C++ one line comment */
			while( (c = fgetc(fp)) != '\n' && c != EOF) ;
			if(c == EOF) return(EOF);
			else return(getValue(fp, v));
		} else if( c1 == '*') {		/* C like comment */
			opencomments = 1;
			c = fgetc(fp);
			if(c == EOF) return(EOF);
			while( (c1 = fgetc(fp)) != EOF ) {
				if( c == '/' && c1 == '*' ) opencomments++;
				if( c == '*' && c1 == '/' ) opencomments--;
				c = c1;
				if(opencomments == 0) break;
			}
			if(c1 == EOF) return(EOF);
			else return(getValue(fp, v));
		} else {
			ungetc(c1, fp);
		}
	}

	if( IsDigit(c) ) {
		i=0; str[i]=c; i++;
		while( IsDigit(c=fgetc(fp)) ) {
			str[i]=c; i++;
		}
		ungetc(c, fp);
		str[i]='\0';
		*v = atof(str);
		return(0);
	} else return(-2);
}

// ***************************************************************
// Get the coefficients from a file. Return values:
//	-2	syntax error within the file
//	-3	filename is empty string
//	-4	can not open the file
//	-5	too many coefficients in the file
//	>=0 	number of the coefficients read
// ***************************************************************
int Parser::getcoeffs(const char *filename, float *v, int max)
{
	int i, n;
	FILE *fp;

	if( strlen(filename) == 0) return(-3);
	if( (fp = fopen(filename, "r")) == NULL) return(-4);

	for(n=0; n < max; ) {
		i = getValue(fp, v);
		if( i == EOF ) {
			fclose(fp);
			return(n);
		} else if( i ==  -2 ) {
			fclose(fp);
			return(-2);
		} else if( i == 0 ) {
			n++; v++;
		}
	}
	fclose(fp);
	return(-5);
}

// ***************************************************************
// Get pair of the coefficients from a file. Return values:
//	-1	odd number of the coefficients found in the file
//	-2	syntax error within the file
//	-3	filename is empty string
//	-4	can not open the file
//	-5	too many coefficients in the file
//	>=0 	number of the coefficients read
// ***************************************************************
int Parser::getcoeffs2(const char *filename, float *v1, float *v2, int max)
{
	int i, n;
	FILE *fp;

	if( strlen(filename) == 0) return(-3);
	if( (fp = fopen(filename, "r")) == NULL) return(-4);

	for(n=0; n < max+1; ) {
		i = getValue(fp, v1);
		if( i == EOF ) {
			fclose(fp);
			return(n);
		} else if( i ==  -2 ) {
			fclose(fp);
			return(-2);
		} else if( i == 0 ) {
			v1++;
		}

		i = getValue(fp, v2);
		if( i == EOF ) {
			fclose(fp);
			return(-2);
		} else if( i ==  -2 ) {
			fclose(fp);
			return(-2);
		} else if( i == 0 ) {
			v2++; n++;
		}
	}

	fclose(fp);
	return(-5);
}

// ***************************************************************
// Saves given coefficients to fir file
// ***************************************************************
void Parser::saveToFile(const char * filename, const float *coefficients, int cCount)
{
    fstream fout;
    fout.open(filename, ios::out);

    char fname[80];
    char *name_pos = 0;

    name_pos = strrchr((char*)filename, '\\');
    if(name_pos == NULL)
        name_pos = strrchr((char*)filename, '/');

    fout << "/* ******************************************************************" << endl;
    fout << "   FILE:\t";
    if(name_pos != NULL)
    {
        strncpy(fname, name_pos+1, 80);
        fout << fname << endl;
    }
    else
        fout << "???" << endl;
    fout << "   DESCRIPTION:\t" << endl;
    fout << "   DATE:\t" << endl;
    fout << "   REVISIONS:\t" << endl;
    fout << "   ****************************************************************** */" << endl << endl;

    for(int i=0; i<cCount; ++i)
    {
        fout << "\t" << std::fixed << coefficients[i];
        if(i<cCount-1)
            fout << ',' << endl;
    }
    fout.close();
}
