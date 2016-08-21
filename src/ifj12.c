/*
* File:                 ifj12.c
* Description:          Main file for ifj12 language compiler
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>

#include "ifj12.h"
#include "ial.h"
#include "stack.h"
#include "scaner.h"
#include "ilist.h"
#include "expressions.h"
#include "parser.h"
#include "interpret.h"
#include "htable.h"


//// GLOBAL VARIABLES ////

// input file descriptor
FILE* f_source = NULL;

// token
TStructToken token;


// table of functions
tFuncHTable FTable[HTSIZE];

// table of labels
tHTable* LabTable;

// global symbol table (main static table)
tHTable* GSTable;

// active (actual) static table
tHTable* ActStaticTab;

// active (actual) dynamic table
tHTable* ActDynamicTab;

// list of instructions
tListOfInstr L;

// instruction
tInstr I;

// stack of called functions
TStack MyStack;

// counter for compiler tmp variables
unsigned long COUNTER = 0;

// counter for level of recursion in parser
unsigned long IS_IN_FUNC = 0;

// array of allocated blocks
void** ArrOfPtr  = NULL;

// index of allocated block
size_t ArrIndex  = 0;


#include "scaner.c"
#include "stack.c"
#include "ial.c"
#include "ilist.c"
#include "parser.c"
#include "expressions.c"
#include "interpret.c"
#include "htable.c"

/*
* MAIN
*/
int main ( int argc, char* argv[] )
{
    //// Opening input file
    if ( argc == 1 )
    {
        fprintf ( stderr, "ifj12.c - main: No source file!\n" );
        exit ( 99 );
    }

    else if ( argc == 2 )
    {
        if ( ( f_source = fopen ( argv[1], "r" ) ) == NULL )
        {
            fprintf ( stderr, "ifj12.c - main: Cannot open source file!\n" );
            exit ( 99 );
        }
    }

    else if ( argc > 2 )
    {
        fprintf ( stderr, "ifj12.c - main: Too much parameters!\n" );
        exit ( 99 );
    }


    //// Initializations of main structures

    // Instruction list initialization
    listInit ( &L );

    // Global symbol table initialization
    GSTable = malloc ( sizeof ( tHTable[HTSIZE] ) );
    htInit ( GSTable );

    // Function table initialization
    FuncHTInit ( FTable );

    // Built in functions definition and declaration
    InsertBuiltInFunctions();

    // Label table initialization
    LabTable = malloc ( sizeof ( tHTable[HTSIZE] ) );
    htInit ( LabTable );

    // Call Stack initialization
    ST_Init ( &MyStack );
    ////////////////////////////////////////////////////////////////////////////

    // Calling parser
    //fprintf ( stderr, "-----------------  PARSER  -----------------\n" );
    parser();

    // Check function definitions
    FuncHTCheck ( FTable );

    // Calling interpret
    //fprintf ( stderr, "----------------- LETS GO! -----------------\n" );
    inter ( &L );

    // THE END
    //SetError ( E_OK, "All right!" );
    MrProper();
    exit ( 0 );
}

/*----------------------------------------------------------------------------*/

/*
* Function for registering blocks of allocated memory
*/
void RegPtr ( void* haha )
{
    const size_t BLOCKSIZE = 32;
    void* tmp = NULL;

    if ( ArrIndex % BLOCKSIZE == 0 )
    {
        tmp = realloc ( ArrOfPtr, ( ArrIndex + BLOCKSIZE + 1 ) * sizeof ( void* ) );

        if ( tmp == NULL )
        {
            SetError ( E_ERR_OTHER, "ifj12.c - RegPtr: Memory allocation error!" );
        }

        ArrOfPtr = tmp;
    }

    ArrOfPtr[ArrIndex++] = haha;
}

/*
* Clean up function, frees allocated memory and closes input file
*/
void MrProper ( void )
{
    //fprintf ( stderr, "----------------- Mr. Proper! -----------------\n" );

    if ( Stack != NULL )
    {
        free ( Stack );
    }

    FuncHTClearAll ( FTable );

    listFree ( &L );

    htClearAllStatic ( LabTable );
    free ( LabTable );

    for ( unsigned long i = 0; i < ArrIndex; i++ )
    {
        free ( ArrOfPtr[i] );
    }

    free ( ArrOfPtr );

    ST_Dispose ( &MyStack );

    if ( ( ( fclose ( f_source ) ) == EOF ) && ( f_source != NULL ) )
    {
        SetError ( E_ERR_OTHER, "ifj12.c - MrProper: Error when closing source file!" );
    }
}

/*
* Function for handling errors and exitcodes
*/
void SetError ( TError err, const char msg[] )
{
    fprintf ( stderr, "EXIT: %d : %s\n", err, msg );
    MrProper();
    exit ( err );
}
