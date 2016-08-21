/*
 * File:                 scanner.h
 * Description:          scanner
 * Author:               Wrona Jan       -   xwrona00
 *
 * Projekt:              IFJ-2012
 * Team 34:
 *                       Duban Michal    -   xduban01
 *                       Hanselka Vaclav -   xhanse00
 *                       Heczkova Petra  -   xheczk04
 *                       Fabry Marko     -   xfabry01
 *                       Wrona Jan       -   xwrona00
 */


#ifndef LEX_H
#define LEX_H

typedef enum
{
    IDENTIFIER, NUMLIT, STRING
} SYMBOLCLASS;

int scanner ( FILE* source );
void append ( char** string, int symbol );
int symbolClass ( int symbol );
int identType ( char* _tokenstr );

#endif



