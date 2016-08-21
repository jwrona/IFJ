/*
* File:                 parser.h
* Description:          Header file for parser.c
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

const unsigned int PARAM_COUNT = 8;

// Function declarations
bool  accepted ( TToken tok );
bool  expect ( TToken tok );


char* AllocateStr ( char* tmp );
char* GenerateTmpVarName();

TStructToken GenerateDoubleToken ( char* name, TToken type, double data );
TStructToken GenerateStringToken ( char* name, TToken type, char* data );
TStructToken GenerateLabelToken ( char* name, TToken type, void* data );

TStructToken* InsertTerm ( TStructToken Literal );

tInstr MakeInstruction ( TInstr instr, TStructToken* addr1, TStructToken* addr2, TStructToken* addr3 );

bool SetArgPtr ( TStructToken** PtrArg );
bool SetArgPtrFunc ( TStructToken** PtrArg );

int getFunctionType ( char* funcstr );
void parser();

void statement ( void );

void handleIdent();
void handleIf();
void handleWhile();
void handleReturn();
void handleFunctionDeclaration();
TStructToken* handleFunctionCall ( TStructToken Dest );

void UserFunctionCall ( TStructToken Dest );
void BuiltInFunctionCall_0 ( TInstr I_type );
void BuiltInFunctionCall_1 ( TInstr I_type );
void BuiltInFunctionCall_2 ( TInstr I_type );
void BuiltInFunctionCall_N ( TInstr I_type );
void BuiltInFunctionCallTypeOf ( TInstr I_type );

