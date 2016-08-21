/*
* File:                 htable.c
* Description:          Header file for htable.h
* Author:               Fabry Marko     -   xfabry01
*
* Projekt:              IFJ-2012
* Team 34:
*                       Duban Michal    -   xduban01
*                       Hanselka Vaclav -   xhanse00
*                       Heczkova Petra  -   xheczk04
*                       Fabry Marko     -   xfabry01
*                       Wrona Jan       -   xwrona00
*/

typedef struct
{
    TStructToken fstart;    // token, data su ukazatel do instrukcnej pasky
    unsigned int fargc;     // pocet argumentov
    TStructToken* fargv;    // argumenty (pole tokenov)
    tHTable*      ftable;   // "vzorova" tabulka
    bool valid;
} TFuncItem;


typedef char* tFuncKey;
typedef TFuncItem tFuncData;

// polozka FuncHT
typedef struct tFuncHTItem
{
    tFuncKey   key;
    tFuncData  data;
    struct tFuncHTItem* ptrnext;
} tFuncHTItem;

// FuncHT
typedef tFuncHTItem* tFuncHTable[HTSIZE];

void     FuncHTInit ( tFuncHTable* ptrht );
tFuncHTItem* FuncHTSearch ( tFuncHTable* ptrht, tFuncKey key );
tFuncData*   FuncHTInsert ( tFuncHTable* ptrht, tFuncKey key, tFuncData data );
tFuncData*   FuncHTRead ( tFuncHTable* ptrht, tFuncKey key );
void     FuncHTDelete ( tFuncHTable* ptrht, tFuncKey key );
void     FuncHTClearAll ( tFuncHTable* ptrht );
