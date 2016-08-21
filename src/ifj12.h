/*
* File:                 ifj12.h
* Description:          Header file for ifj12.c
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

// Token types
typedef enum
{
    tIf,            // if
    tWhile,         // while
    tFunction,      // function
    tReturn,        // return   3

    tElse,          // else
    tEnd,           // end  5

    tIdent,         // identifikator    6

    tAdd,           // +
    tMul,           // *
    tSub,           // -
    tDiv,           // /
    tPow,           // **   11

    tEq,            // ==
    tNEq,           // !=
    tGt,            // >
    tLt,            // <
    tGtEq,          // >=
    tLtEq,          // <=   17

    tL_Br,          // (
    tR_Br,          // )    19

    tNumeric,       // data = double
    tString,        // data = str
    tBoolean,       // data = 0.0 / 1.0
    tNil,           // value = NULL 23

    tEol,           // EndOfLine

    tL_Sq_Br,       // [
    tR_Sq_Br,       // ]    29

    tColon,         // :
    tComma,         // ,

    tTrue,          // 1.0
    tFalse,         // 0.0

    tEof,           // EndOfFile    27

    tAssign,        // =    32

    tReserved,      //      33

    tExpress,       // ???  34
    tError,         // ???  35
    tMark,          // ???  36

    tLab,

} TToken;

// Token signs
const char* TOKENSIGN[] =
{
    [tIdent]         = "identifikator",
    [tIf]            = "if",
    [tWhile]         = "while",
    [tFunction]      = "function",
    [tReturn]        = "return",
    [tElse]          = "else",
    [tEnd]           = "end",
    [tAssign]        = "=",
    [tAdd]           = "+",
    [tMul]           = "*",
    [tSub]           = "-",
    [tDiv]           = "/",
    [tPow]           = "**",
    [tEq]            = "==",
    [tNEq]           = "!=",
    [tGt]            = ">",
    [tLt]            = "<",
    [tGtEq]          = ">=",
    [tLtEq]          = "<=",
    [tL_Br]          = "(",
    [tR_Br]          = ")",
    [tL_Sq_Br]       = "[",
    [tR_Sq_Br]       = "]",
    [tComma]         = ",",
    [tColon]         = ":",
    [tNumeric]       = "Numeric",
    [tString]        = "String",
    [tBoolean]       = "Bool",
    [tNil]           = "nil",
    [tTrue]          = "true",
    [tFalse]         = "false",
    [tEol]           = "EndOfLine",
    [tEof]           = "EndOfFile",
    [tReserved]      = "Reserved",
    [tExpress]       = "Expression",
    [tError]         = "Error",
    [tMark]          = "<",
    [tLab]          = "Label",
};

// Reserved words
const char* RESERVED[] =
{
    "as",
    "def",
    "directive",
    "export",
    "from",
    "import",
    "launch",
    "load",
    "macro",
};

// Reserved words count
int RESERVED_LEN = ( sizeof ( RESERVED ) / sizeof ( char* ) );

// Built-in function types
typedef enum
{
    func_input,
    func_numeric,
    func_print,
    func_typeOf,
    func_len,
    func_find,
    func_sort,
    func_other,
} TBuiltFunc;

// Built-in function signs
const char* BuiltFuncSIGN[] =
{
    "input",
    "numeric",
    "print",
    "typeOf",
    "len",
    "find",
    "sort",
};

// Exitcodes
typedef enum
{
    E_OK                = 0,
    E_LEX               = 1,
    E_SYN               = 2,
    E_SEM_UNDEF_VAR     = 3,
    E_SEM_UNDEF_FUNC    = 4,
    E_SEM_OTHER         = 5,
    E_INT_DIV_BY_ZERO   = 10,
    E_INT_TYPE          = 11,
    E_INT_NUMERIC       = 12,
    E_INT_OTHER         = 13,
    E_ERR_OTHER         = 99,
} TError;

// Token definition
typedef struct
{
    char*               name;
    TToken              type;
    void*               data;
} TStructToken;

// Function declarations
int main ( int argc, char* argv[] );

void SetError ( TError err, const char msg[] );
void MrProper ( void );
void RegPtr ( void* Ptr );


