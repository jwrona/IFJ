/*
* File:                 ial.h
* Description:          Header file for ial.c
*
* Author:               Fabry Marko     -   xfabry01 (hash table)
*                       Hanselka Vaclav -   xhanse00 (QS, KMP alg.)
*
* Projekt:              IFJ-2012
* Team 34:
*                       Duban Michal    -   xduban01
*                       Hanselka Vaclav -   xhanse00
*                       Heczkova Petra  -   xheczk04
*                       Fabry Marko     -   xfabry01
*                       Wrona Jan       -   xwrona00
*/

#define HTSIZE 13
typedef char* tKey;

typedef TStructToken tData;

typedef struct tHTItem
{
    tKey   key;
    tData  data;
    struct tHTItem* ptrnext;
} tHTItem;

typedef tHTItem* tHTable[HTSIZE];

unsigned long      hashCode ( tKey key );
void     htInit ( tHTable* ptrht );
tHTItem* htSearch ( tHTable* ptrht, tKey key );
tData*   htInsertStatic ( tHTable* ptrht, tKey key, tData data );
tData*   htInsertDynamic ( tHTable* ptrht, tKey key, tData data );
tData*   htRead ( tHTable* ptrht, tKey key );
void     htDelete ( tHTable* ptrht, tKey key );
void     htClearAllStatic ( tHTable* ptrht );
void     htClearAllDynamic ( tHTable* ptrht );
