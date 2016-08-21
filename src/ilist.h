/*
* ilist.h castecne prevzat z jednoducheho interpretu uverejneneho na strankach predmetu
* File:                 ilist.h
* Description:          Header file for ilist.c
*
* Projekt:              IFJ-2012
* Team 34:
*                       Duban Michal    -   xduban01
*                       Hanselka Vaclav -   xhanse00
*                       Heczkova Petra  -   xheczk04
*                       Fabry Marko     -   xfabry01
*                       Wrona Jan       -   xwrona00
*/

// typy jednotlivych instrukci
typedef enum
{
    I_ADD,
    I_SUB,
    I_MUL,
    I_DIV,
    I_POW,

    I_EQ,
    I_NEQ,
    I_GTE,
    I_LTE,
    I_GT,
    I_LT,

    I_SUBSTR,

    I_STOP,
    I_LAB,
    I_IFGOTO,
    I_GOTO,

    I_INPUT,
    I_NUMERIC,
    I_PRINT,
    I_TYPEOF,
    I_LEN,
    I_FIND,
    I_SORT,

    I_COPYTAB,
    I_INSPAR,
    I_CALL,
    I_RET,
    I_ASS,
    I_INSERT,
} TInstr;

const char* INSTSIGN[] =
{
    [I_ADD]     = "I_ADD",
    [I_SUB]     = "I_SUB",
    [I_MUL]     = "I_MUL",
    [I_DIV]     = "I_DIV",
    [I_POW]     = "I_POW",

    [I_EQ]      = "I_EQ",
    [I_NEQ]     = "I_NEQ",
    [I_GTE]     = "I_GTE",
    [I_LTE]     = "I_LTE",
    [I_GT]      = "I_GT",
    [I_LT]      = "I_LT",

    [I_SUBSTR]  = "I_SUBSTR",

    [I_STOP]    = "I_STOP",
    [I_LAB]     = "I_LAB",
    [I_IFGOTO]  = "I_IFGOTO",
    [I_GOTO]    = "I_GOTO",

    [I_INPUT]   = "I_INPUT",
    [I_NUMERIC] = "I_NUMERIC",
    [I_PRINT]   = "I_PRINT",
    [I_TYPEOF]  = "I_TYPEOF",
    [I_LEN]     = "I_LEN",
    [I_FIND]    = "I_FIND",
    [I_SORT]    = "I_SORT",
    [I_COPYTAB] = "I_COPYTAB",

    [I_INSPAR]  = "I_INSPAR",
    [I_CALL]    = "I_CALL",
    [I_RET]     = "I_RET",
    [I_ASS]     = "I_ASS",
    [I_INSERT]  = "I_INSERT",
};

typedef struct
{
    TInstr instType;  // typ instrukce
    TStructToken* addr1; // adresa 1
    TStructToken* addr2; // adresa 2
    TStructToken* addr3; // adresa 3
} tInstr;

typedef struct listItem
{
    tInstr Instruction;
    struct listItem* ptr;        /* ukazatel na následující prvek seznamu */
}* tListItem;

typedef struct
{
    tListItem first;  // ukazatel na prvni prvek
    tListItem active; // ukazatel na aktivni prvek
} tListOfInstr;

void listInit ( tListOfInstr* L );
void listFree ( tListOfInstr* L );
void listInsertFirst ( tListOfInstr* L, tInstr I );
void listInsertLast ( tListOfInstr* L, tInstr I );
void* listPostInsert ( tListOfInstr* L, tInstr I );
void listPreInsert ( tListOfInstr* L, tInstr I );
void listFirst ( tListOfInstr* L );
void listLast ( tListOfInstr* L );
void listNext ( tListOfInstr* L );
void listPre ( tListOfInstr* L );
void listGoto ( tListOfInstr* L, void* gotoInstr );
void* listGetPointerLast ( tListOfInstr* L );
tInstr* listGetData ( tListOfInstr* L );



