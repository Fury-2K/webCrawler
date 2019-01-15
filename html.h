// FILE: html.h
//
//
// HTML parser utility implementation
// see html.h for detail usage

#ifndef _HTML__H_
#define _HTML__H_

int extract_url(char*,char*,char**);

int GetNextURL(char* html, char* urlofthispage, char* result, int pos);

//! \brief Make all letters in word be in lower cases.
void NormalizeWord(char* word);

//! \brief normalize URL 
//! 
//! \param URL to url to be normalized
//! \return 1 if this url is pure text format (html/php/jsp), 0 if it is of other type (pdf/jpg........)

int NormalizeURL(char* URL);

//! \brief removes all white space char's in a large string
void removeWhiteSpace(char* html);

#define IS_ALPHA(c) ((('a'<=(c))&&((c)<='z'))||(('A'<=(c))&&((c)<='Z'))) 

#endif
