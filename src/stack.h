/*
* File:                 stack.h
* Description:          Header file for stack.c (ADT Stack)
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

// Stack data object definition
typedef struct
{
    TStructToken label;
    tHTable*     table;
} TMyStackItem;
typedef TMyStackItem tStackData;

// Stack item definition
typedef struct tStackItem
{
    tStackData data;
    struct tStackItem* next;
} TStackItem;

// Stack definiton
typedef struct
{
    TStackItem* Top;
} TStack;

// Function declarations
void StackInit ( TStack* Stack );
void StackPush ( TStack* Stack, tStackData data );
tStackData StackPop ( TStack* Stack );
void StackDispose ( TStack* Stack );
