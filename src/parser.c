/*
* File:                 parser.c
* Description:          Parser of input programs control flow statement.
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

/*
* Function used at recursive descent parsing
* If actual token is acceptable acording to grammar rules, it is accepted.
*/
bool accepted ( TToken tok )
{
    if ( token.type == tok )
    {
        token = GetNextToken();
        return true;
    }

    return false;
}

/*
* Function used at recursive descent parsing
* If actual token is expected according to grammar rules, it is accepted.
* If actual token is not expected according to grammar rules, it is syntactic error.
*/
bool expect ( TToken tok )
{
    if ( token.type == tok )
    {
        token = GetNextToken();
        return true;
    }

    fprintf ( stderr, "parser.c - expect: Token: \" %s \" expected, but token \" %s \" found!\n", TOKENSIGN[tok], TOKENSIGN[token.type] );
    SetError ( E_SYN, "parser.c - expect: Unexpected token!" );

    return false;
}

/*
* Main parser function. It creates artificial "main" function
* and inserts coresponding instructions to instruction list.
* It also calls function statement, which is the begining
* of recursive descent parsing.
*
* Note: on key points there are ilustrations of instruction list parts
*/
void parser()
{

    // Set Active Static Table to GSTable ( GSTable reprezents "main" )
    ActStaticTab = GSTable;

    // Inserting FIRST instruction
    I = MakeInstruction ( I_LAB, NULL, NULL, NULL );
    listInsertFirst ( &L, I );
    listFirst ( &L );

    ////////////////////////////////////////////////////////////////////////////
    //  ->  BEGIN
    ////////////////////////////////////////////////////////////////////////////

    I = MakeInstruction ( I_LAB, NULL, NULL, NULL );
    TStructToken LAB_END_FUNC = GenerateLabelToken ( GenerateTmpVarName(), tLab, listPostInsert ( &L, I ) );
    TStructToken LAB_START_FUNC = GenerateLabelToken ( GenerateTmpVarName(), tLab, listPostInsert ( &L, I ) );
    TStructToken* PTR_LAB_END_FUNC = htInsertStatic ( LabTable,  LAB_END_FUNC.name, LAB_END_FUNC );
    htInsertStatic ( LabTable,  LAB_START_FUNC.name, LAB_START_FUNC );

    I = MakeInstruction ( I_GOTO, PTR_LAB_END_FUNC, NULL, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );
    listNext ( &L );

    ////////////////////////////////////////////////////////////////////////////
    //      BEGIN
    //      I_GOTO
    //  ->  LAB_START_FUNC
    //      LAB_END_FUNC
    ////////////////////////////////////////////////////////////////////////////

    // New record in function table
    TFuncItem NewFuncRecord;
    NewFuncRecord.fargc  = 0;
    NewFuncRecord.fargv  = NULL;
    NewFuncRecord.ftable = GSTable;
    NewFuncRecord.fstart = LAB_START_FUNC;
    NewFuncRecord.valid  = true;
    FuncHTInsert ( FTable, AllocateStr ( "@main" ), NewFuncRecord );


    // Begining of recursive descent
    token = GetNextToken();
    statement();

    // End of recursive descent
    if ( token.type != tEof )
    {
        SetError ( E_SYN, "parser.c - parser: End of file expected" );
    }

    ////////////////////////////////////////////////////////////////////////////
    //      BEGIN
    //      GOTO
    //      LAB_START_FUNC
    //          XXX
    //          YYY
    //  ->      ZZZ
    //      LAB_END_FUNC
    ////////////////////////////////////////////////////////////////////////////

    I = MakeInstruction ( I_RET, NULL, NULL, NULL );
    listPostInsert ( &L, I ); //8
    listNext ( &L ); //9
    listNext ( &L ); //10

    ////////////////////////////////////////////////////////////////////////////
    //      BEGIN
    //      GOTO
    //      LAB_START_FUNC
    //          XXX
    //          YYY
    //          ZZZ
    //          RETURN
    //  ->  LAB_END_FUNC
    ////////////////////////////////////////////////////////////////////////////


    TStructToken MAINFunc = GenerateLabelToken ( AllocateStr ( "@main" ), tFunction, NULL );
    TStructToken* PTRMAINFunc = htInsertStatic ( GSTable, MAINFunc.name, MAINFunc );

    I = MakeInstruction ( I_COPYTAB, PTRMAINFunc , NULL, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );

    I = MakeInstruction ( I_CALL, PTRMAINFunc, NULL, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );

    I = MakeInstruction ( I_STOP, NULL, NULL, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );

    ////////////////////////////////////////////////////////////////////////////
    //      BEGIN
    //      GOTO
    //      LAB_START_FUNC
    //          XXX
    //          YYY
    //          ZZZ
    //          RETURN
    //      LAB_END_FUNC
    //      COPYTAB
    //      CALL
    //  ->  STOP
    ////////////////////////////////////////////////////////////////////////////
}

/*
* Recursive function which handles the "FIRST" set of LL Grammar, that means
* it handles the tokens which are allowed to be on the beginning of the NEW LINE
*/
void statement ( void )
{

    if ( token.type == tIdent )
    {
        handleIdent();
    }

    else if ( accepted ( tIf ) )
    {
        handleIf();
    }

    else if ( accepted ( tWhile ) )
    {
        handleWhile();
    }

    else if ( accepted ( tFunction ) )
    {
        handleFunctionDeclaration();
    }

    else if ( accepted ( tReturn ) )
    {
        handleReturn();
    }

    else if ( ( token.type == tEnd ) || ( token.type == tElse ) || ( token.type == tEof ) )
    {
        return;     // return u are expected ...
    }

    else
    {
        fprintf ( stderr, "parser.c - statement: Token %s was found on new line!\n", TOKENSIGN[token.type] );
        SetError ( E_SYN, "parser.c - statement: Unexpected token!" );
    }

    statement();    // keep working man, fetch me next line, go go go !!!
}

/*
* Function which handles line begining with "identificator"
* id = E
* id = func()
* id = str[ num1 : num2 ]
*/
void handleIdent()
{
    char* key = token.name;

    accepted ( tIdent );
    expect ( tAssign );

    // check if variable has not name like (previously defined) function
    TStructToken* PtrCheck = htRead ( GSTable, key );

    if ( PtrCheck != NULL && PtrCheck->type == tFunction )
    {
        SetError ( E_SEM_OTHER, "parser.c - handleIdent: Variable cannot have name like existing function!" );
    }

    // call expression parser to check wheter we are dealing with:
    // a) assigment
    // b) substring command
    // c) function call
    TStructToken* PtrExpr = expression();

    // check if variable is already defined
    TStructToken* PtrVar = htRead ( ActStaticTab, key );

    if ( PtrVar == NULL )
    {
        TStructToken Var;
        Var.name = key;
        Var.type = tIdent;
        Var.data = NULL;

        PtrVar = htInsertStatic ( ActStaticTab, key, Var );
    }

    I = MakeInstruction ( I_ASS, PtrVar, PtrExpr, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );

    ////////////////////////////////////////////////////////////////////////////
    //      XXX
    //  ->  ASSIGN
    ////////////////////////////////////////////////////////////////////////////

    expect ( tEol );
}


/*
* Function which handles "if" construction
*/
void handleIf()
{
    TStructToken* PtrCond = expression();
    expect ( tEol );

    I = MakeInstruction ( I_LAB, NULL, NULL, NULL );

    TStructToken LAB_END  = GenerateLabelToken ( GenerateTmpVarName(), tLab, listPostInsert ( &L, I ) );
    TStructToken LAB_ELSE = GenerateLabelToken ( GenerateTmpVarName(), tLab, listPostInsert ( &L, I ) );

    TStructToken* PTR_LAB_END  = htInsertStatic ( LabTable, LAB_END.name, LAB_END );
    TStructToken* PTR_LAB_ELSE = htInsertStatic ( LabTable, LAB_ELSE.name, LAB_ELSE );

    I = MakeInstruction ( I_GOTO, PTR_LAB_END, NULL, NULL );
    listPostInsert ( &L, I );

    I = MakeInstruction ( I_IFGOTO, PTR_LAB_ELSE, PtrCond, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );

    ////////////////////////////////////////////////////////////////////////////
    //      EXPR (COND)
    //  ->  IFGOTO
    //
    //      GOTO
    //      LAB_ELSE
    //
    //      LAB_END
    ////////////////////////////////////////////////////////////////////////////

    //fprintf(stderr, "info: entering inner if statement!\n");
    IS_IN_FUNC++;
    statement();
    IS_IN_FUNC--;
    //fprintf(stderr, "info: exitting inner if statement!\n");

    listNext ( &L );
    listNext ( &L );

    ////////////////////////////////////////////////////////////////////////////
    //      XXX
    //      IFGOTO
    //          AAA
    //          BBB
    //          CCC
    //      GOTO
    //  ->  LAB_ELSE
    //
    //      LAB_END
    ////////////////////////////////////////////////////////////////////////////

    expect ( tElse );
    expect ( tEol );

    //fprintf(stderr, "info: entering inner else statement!\n");
    IS_IN_FUNC++;
    statement();
    IS_IN_FUNC--;
    //fprintf(stderr, "info: exitting inner else statement!\n");

    listNext ( &L );

    ////////////////////////////////////////////////////////////////////////////
    //      XXX
    //      IFGOTO
    //          AAA
    //          BBB
    //          CCC
    //      GOTO
    //      LAB_ELSE
    //          HHH
    //          JJJ
    //          KKK
    //  ->  LAB_END
    ////////////////////////////////////////////////////////////////////////////

    expect ( tEnd );
    expect ( tEol );
}

/*
* Function which handles "while" construction
*/
void handleWhile()
{
    I = MakeInstruction ( I_LAB, NULL, NULL, NULL );
    TStructToken LAB_END   = GenerateLabelToken ( GenerateTmpVarName(), tLab, listPostInsert ( &L, I ) );
    TStructToken LAB_START = GenerateLabelToken ( GenerateTmpVarName(), tLab, listPostInsert ( &L, I ) );
    listNext ( &L );

    ////////////////////////////////////////////////////////////////////////////
    //      XXX
    //  ->  LAB_START
    //
    //      LAB_END
    ////////////////////////////////////////////////////////////////////////////

    TStructToken* PtrCond = expression();
    expect ( tEol );

    ////////////////////////////////////////////////////////////////////////////
    //      XXX
    //      LAB_START
    //  ->  EXPR (COND)
    //
    //      LAB_END
    ////////////////////////////////////////////////////////////////////////////

    TStructToken* PTR_LAB_END   = htInsertStatic ( LabTable,  LAB_END.name, LAB_END );
    TStructToken* PTR_LAB_START = htInsertStatic ( LabTable, LAB_START.name, LAB_START );

    I = MakeInstruction ( I_GOTO, PTR_LAB_START, NULL, NULL );
    listPostInsert ( &L, I );

    I = MakeInstruction ( I_IFGOTO, PTR_LAB_END, PtrCond, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );

    ////////////////////////////////////////////////////////////////////////////
    //      XXX
    //      LAB_START
    //      EXPR (COND)
    //  ->  IFGOTO
    //
    //      IGOTO
    //      LAB_END
    ////////////////////////////////////////////////////////////////////////////

    //fprintf(stderr, "info: entering inner while statement!\n");
    IS_IN_FUNC++;
    statement();
    IS_IN_FUNC--;
    //fprintf(stderr, "info: exitting inner while statement!\n");

    listNext ( &L );
    listNext ( &L );

    ////////////////////////////////////////////////////////////////////////////
    //      XXX
    //      LAB_START
    //      EXPR (COND)
    //      IFGOTO
    //          XXX
    //          YYY
    //          ZZZ
    //      IGOTO
    //  ->  LAB_END
    ////////////////////////////////////////////////////////////////////////////

    expect ( tEnd );
    expect ( tEol );
}

/*
* Function which handles "return" construction
*/
void handleReturn()
{
    if ( IS_IN_FUNC )
    {
        TStructToken* PTR_RETURN = htRead ( ActStaticTab, "@return" );
        TStructToken* PTR_EXPR   = expression();

        I = MakeInstruction ( I_ASS, PTR_RETURN, PTR_EXPR, NULL );
        listPostInsert ( &L, I );
        listNext ( &L );

        I = MakeInstruction ( I_RET, NULL, NULL, NULL );
        listPostInsert ( &L, I );
        listNext ( &L );

        ////////////////////////////////////////////////////////////////////////
        //      XXX
        //      EXPR
        //      ASSIGN
        //  ->  RETURN
        ////////////////////////////////////////////////////////////////////////
    }

    else
    {
        expression();

        I = MakeInstruction ( I_STOP, NULL, NULL, NULL );
        listPostInsert ( &L, I );
        listNext ( &L );

        ////////////////////////////////////////////////////////////////////////
        //      XXX
        //      (( EXPR ))
        //  ->  STOP
        ////////////////////////////////////////////////////////////////////////
    }

    expect ( tEol );
}

/*
* Function which handles "function" construction (function declaration)
*/
void handleFunctionDeclaration()
{
    unsigned long i = 0;
    unsigned long j = 0;
    char* funckey = NULL;
    TStructToken* ArgArray = NULL;
    tHTable* NewTableRecord = NULL;
    TFuncItem NewFuncRecord;

    // check if declaration is not in "main"
    if ( IS_IN_FUNC )
    {
        SetError ( E_SYN, "parser.c - handleFunctionDeclaration: Functions must be declared at top level" );
    }

    if ( token.type == tIdent )
    {
        funckey = token.name;
    }

    expect ( tIdent );
    expect ( tL_Br );

    // check if symbol with this name exist
    TStructToken* PtrToken = htRead ( GSTable, funckey );

    // symbol exist and it is not function
    if ( PtrToken != NULL && PtrToken->type != tFunction )
    {
        SetError ( E_SEM_OTHER, "error: uz existuje premenna s menom tejto funkcie!" );
    }

    // symbol does not exist
    else if ( PtrToken == NULL )
    {
        TStructToken TmpFuncToken;
        TmpFuncToken.name = funckey;
        TmpFuncToken.type = tFunction;
        TmpFuncToken.data = NULL;

        htInsertStatic ( GSTable, funckey, TmpFuncToken );
    }

    // symbol exist and it is function
    TFuncItem* PtrFuncRec = FuncHTRead ( FTable, funckey );

    // function is valid -> redefiniton
    if ( PtrFuncRec != NULL && PtrFuncRec->valid )
    {
        SetError ( E_SEM_OTHER, "parser.c - handleFunctionDeclaration: Function redefined!" );
    }

    // function is defined for the first time
    else
    {
        // initialize new static table for function and set it as active
        NewTableRecord = malloc ( sizeof ( tHTable[HTSIZE] ) );

        if ( NewTableRecord == NULL )
        {
            SetError ( E_ERR_OTHER, "parser.c - handleFunctionDeclaration: Memory allocation error!" );
        }

        htInit ( NewTableRecord );
        ActStaticTab = NewTableRecord;

        // function was called before
        if ( PtrFuncRec != NULL && !PtrFuncRec->valid )
        {
            j        = PtrFuncRec->fargc;
            ArgArray = PtrFuncRec->fargv;
        }

        // never heard about this function before
        else
        {
            ArgArray = malloc ( sizeof ( TStructToken ) );

            if ( ArgArray == NULL )
            {
                SetError ( E_ERR_OTHER, "parser.c - handleFunctionDeclaration: Memory allocation error!" );
            }
        }

    }


    ////////////////////////////////////////////////////////////////////////////
    // HANDLING ARGUMENTS

    // place for return
    ArgArray[0] = GenerateDoubleToken ( AllocateStr ( "@return" ), tNil, 0.0 );
    htInsertStatic ( ActStaticTab, ArgArray[0].name, ArgArray[0] );

    do
    {
        if ( token.type == tIdent )
        {
            // check if argument was redefined
            if ( htRead ( ActStaticTab, token.name ) )
            {
                SetError ( E_SEM_OTHER, "parser.c - handleFunctionDeclaration: Argument " );
            }

            if ( i % PARAM_COUNT == 0 )
            {
                TStructToken* tmp = NULL;
                tmp = realloc ( ArgArray, sizeof ( TStructToken ) * ( i + PARAM_COUNT + 1 ) );
                ArgArray = tmp;
            }

            i++;

            // insert i-th argument and set it to NIL
            ArgArray[i] = GenerateDoubleToken ( token.name, tNil, 0.0 );
            htInsertStatic ( ActStaticTab, ArgArray[i].name, ArgArray[i] );

            accepted ( tIdent );
        }

    }
    while ( accepted ( tComma ) );

    expect ( tR_Br );
    expect ( tEol );

    ////////////////////////////////////////////////////////////////////////////
    // INSTRUCTIONS

    I = MakeInstruction ( I_LAB, NULL, NULL, NULL );

    TStructToken LAB_END_FUNC   = GenerateLabelToken ( GenerateTmpVarName(), tLab, listPostInsert ( &L, I ) );
    TStructToken LAB_START_FUNC = GenerateLabelToken ( GenerateTmpVarName(), tLab, listPostInsert ( &L, I ) );
    TStructToken* PTR_LAB_END_FUNC = htInsertStatic ( LabTable,  LAB_END_FUNC.name, LAB_END_FUNC );


    I = MakeInstruction ( I_GOTO, PTR_LAB_END_FUNC, NULL, NULL );
    listPostInsert ( &L, I ); //4
    listNext ( &L ); //5
    listNext ( &L ); //6

    I = MakeInstruction ( I_RET, NULL, NULL, NULL );
    listPostInsert ( &L, I ); //7

    ////////////////////////////////////////////////////////////////////////////

    NewFuncRecord.fargc  = i;
    NewFuncRecord.ftable = NewTableRecord;
    NewFuncRecord.fargv  = ArgArray;
    NewFuncRecord.fstart = LAB_START_FUNC;
    NewFuncRecord.valid  = true;
    FuncHTInsert ( FTable, funckey, NewFuncRecord );

    ////////////////////////////////////////////////////////////////////////////
    //      GOTO
    //  ->  LAB_START_FUNC
    //
    //          RETURN
    //      LAB_END_FUNC
    ////////////////////////////////////////////////////////////////////////////

    //fprintf ( stderr, "info: entering inner function statement!\n" );
    IS_IN_FUNC++;
    statement();
    IS_IN_FUNC--;
    //fprintf ( stderr, "info: exitting inner function statement!\n" );

    expect ( tEnd );
    expect ( tEol );

    listNext ( &L ); //8
    listNext ( &L ); //9

    ////////////////////////////////////////////////////////////////////////////
    //      GOTO
    //      LAB_START_FUNC
    //          XXX
    //          YYY
    //          ZZZ
    //          RETURN
    //  ->  LAB_END_FUNC
    ////////////////////////////////////////////////////////////////////////////

    // out of function, lets set "main"
    ActStaticTab = GSTable;
}


/*******************************************************************************
*
* HELPER FUNCTIONS
*
*******************************************************************************/

/*
* Helper function
* Allocates memory for string variable or constant
*/
char* AllocateStr ( char* tmp )
{
    char* str = ( void* ) malloc ( sizeof ( char ) * ( strlen ( tmp ) + 1 ) );

    if ( str == NULL )
    {
        SetError ( E_ERR_OTHER, "parser.c - AllocateStr: Memory allocation error!" );
    }

    RegPtr ( str );
    str = strcpy ( str, tmp );

    return str;
}

/*
* Helper function
* Generates unique string for temporary variables
* The string is integer number with prefix "@"
*/
char* GenerateTmpVarName()
{
    char* tmpvarname = ( char* ) malloc ( 20 * sizeof ( char ) );

    if ( tmpvarname == NULL )
    {
        SetError ( E_ERR_OTHER, "parser.c - GenerateTmpVarName: Memory allocation error!" );
    }

    RegPtr ( tmpvarname );

    sprintf ( tmpvarname, "@%lu", COUNTER++ );

    return tmpvarname;
}

/*
* Helper function
* Generates Token (Symbol) based on double (Numeric, Boolean, Nil)
*/
TStructToken GenerateDoubleToken ( char* name, TToken type, double data )
{
    double* dabl = ( void* ) malloc ( sizeof ( double ) );

    if ( dabl == NULL )
    {
        SetError ( E_ERR_OTHER, "parser.c - GenerateDoubleToken: Memory allocation error!" );
    }

    RegPtr ( dabl );


    *dabl = data;

    TStructToken tmp;
    tmp.name = name;
    tmp.type = type;
    tmp.data = dabl;
    return tmp;
}

/*
* Helper function
* Generates Token (Symbol) based on string (String)
*/
TStructToken GenerateStringToken ( char* name, TToken type, char* data )
{
    char* str = ( void* ) malloc ( sizeof ( char ) * ( strlen ( data ) ) );

    if ( str == NULL )
    {
        SetError ( E_ERR_OTHER, "parser.c - GenerateStringToken: Memory allocation error!" );
    }

    RegPtr ( str );

    str = strcpy ( str, data );

    TStructToken tmp;
    tmp.name = name;
    tmp.type = type;
    tmp.data = str;
    return tmp;
}

/*
* Helper function
* Generates Token (Symbol) which holds pointer to instruction (Label)
*/
TStructToken GenerateLabelToken ( char* name, TToken type, void* data )
{
    TStructToken tmp;
    tmp.name = name;
    tmp.type = type;
    tmp.data = data;
    return tmp;
}

/*
* Helper function
* Generates three adress instruction from pointers to symbol table
*/
tInstr MakeInstruction ( TInstr instr, TStructToken* addr1,
                         TStructToken* addr2, TStructToken* addr3 )
{
    tInstr I;
    I.instType = instr;
    I.addr1    = addr1;
    I.addr2    = addr2;
    I.addr3    = addr3;
    return I;
}

/*
* Helper function
* Inserts literal or variable to Active symbol table
*/
TStructToken* InsertTerm ( TStructToken Literal )
{
    TStructToken* PtrLiteral = NULL;

    // variable
    if ( Literal.name != NULL )
    {
        // is in table?
        PtrLiteral = htRead ( ActStaticTab, Literal.name );

        // it is not in table -> is undefinded
        if ( PtrLiteral == NULL )
        {
            fprintf ( stderr, "parser.c - InsertTerm: Variable %s is undefined!\n", Literal.name );
            SetError ( E_SEM_UNDEF_VAR, "Undefined variable" );
        }
    }

    // literal
    else
    {
        // insert literal to symbol table
        Literal.name = GenerateTmpVarName();
        PtrLiteral   = htInsertStatic ( ActStaticTab, Literal.name, Literal );
    }

    return PtrLiteral;
}

/*
* Helper function
* Returns type of function:
* 0-6   - One of Built-in functions
* 7     - User defined function
*/
int getFunctionType ( char* funcstr )
{
    for ( int i = 0; i < func_other; i++ )
    {
        if ( strcmp ( funcstr, BuiltFuncSIGN[i] ) == 0 )
        {
            return i;
        }
    }

    return func_other;
}

/*
* Helper function
* Inserts definitions of built-in functions to symbol and function tables
*/
void InsertBuiltInFunctions()
{
    for ( int i = 0; i < func_other; i++ )
    {
        char* str = malloc ( sizeof ( char ) * 10 );

        if ( str == NULL )
        {
            SetError ( E_ERR_OTHER, "parser.c - InsertBuiltInFunctions: Memory allocation error!" );
        }

        str = strcpy ( str, BuiltFuncSIGN[i] );

        RegPtr ( str );


        TStructToken FuncName;
        FuncName.name = str;
        FuncName.type = tFunction;
        FuncName.data = NULL;

        htInsertStatic ( GSTable, FuncName.name, FuncName );

        TFuncItem FuncRec;
        FuncRec.valid = true;
        FuncRec.fargv = NULL;
        FuncRec.fargc = 0;
        FuncRec.ftable = NULL;
        FuncRec.fstart.name = NULL;
        FuncRec.fstart.type = tError;
        FuncRec.fstart.data = NULL;

        FuncHTInsert ( FTable, str, FuncRec );
    }
}

/*
* Helper function
* Inserts Arguments if found and inserted returns true,
* if not found returns false
*/
bool SetArgPtr ( TStructToken** PtrArg )
{
    *PtrArg = NULL;

    if ( token.type == tIdent || ( tNumeric <= token.type && token.type <= tNil ) )
    {
        *PtrArg = InsertTerm ( token );
        token = GetNextToken();
        return true;
    }

    else
    {
        if ( token.type == tFunction )
        {
            SetError ( E_SEM_OTHER, "parser.c - SetArgPtr: Invalid argument" );
        }

        return false;
    }
}

/*
* Helper function
* Inserts Arguments if found and inserted returns true,
* if not found returns false
* Function can be argument
*/
bool SetArgPtrFunc ( TStructToken** PtrArg )
{
    *PtrArg = NULL;

    if ( token.type == tIdent || ( tNumeric <= token.type && token.type <= tNil ) )
    {
        *PtrArg = InsertTerm ( token );
        token = GetNextToken();
        return true;
    }

    else if ( token.type == tFunction )
    {
        return true;
    }

    else
    {

        return false;
    }
}

/******************************************************************************/

/*
* Helper function
* Inserts Arguments if found and inserted returns true,
* if not found returns false
* Function can be argument
*/
TStructToken* handleFunctionCall ( TStructToken FuncName )
{
    expect ( tL_Br );

    TStructToken* PtrTmp = htRead ( GSTable, FuncName.name );

    if ( PtrTmp != NULL && PtrTmp->type != tFunction )
    {
        SetError ( E_SEM_OTHER, "error: vola sa nieco co nie je funkcia!" );
    }

    int functype = getFunctionType ( FuncName.name );

    switch ( functype )
    {
        case func_input:
        {
            BuiltInFunctionCall_0 ( I_INPUT );
        }
        break;

        case func_numeric:
        {
            BuiltInFunctionCall_1 ( I_NUMERIC );
        }
        break;

        case func_typeOf:
        {
            BuiltInFunctionCallTypeOf ( I_TYPEOF );
        }
        break;

        case func_len:
        {
            BuiltInFunctionCall_1 ( I_LEN );
        }
        break;

        case func_sort:
        {
            BuiltInFunctionCall_1 ( I_SORT );
        }
        break;

        case func_find:
        {
            BuiltInFunctionCall_2 ( I_FIND );
        }
        break;

        case func_print:
        {
            BuiltInFunctionCall_N ( I_PRINT );
        }
        break;

        default:
        case func_other:
        {
            UserFunctionCall ( FuncName );
        }
        break;
    }

    expect ( tR_Br );

    // miesto na výsledok funkcie
    TStructToken  Result    = GenerateDoubleToken ( AllocateStr ( "@result" ), tNil, 0.0 );
    TStructToken* PtrResult = htInsertStatic ( ActStaticTab, Result.name, Result );
    return PtrResult;
}

////////////////////////////////////////////////////////////////////////////////
/*
* Function which handles "id = func(a, b, c)" construction (function call)
*/
void UserFunctionCall ( TStructToken FuncName )
{
    TStructToken* PtrTmp      = NULL;


    TStructToken* PtrFuncName = htRead ( GSTable, FuncName.name );

    if ( PtrFuncName == NULL )
    {
        FuncName.type = tFunction;
        PtrFuncName = htInsertStatic ( GSTable, FuncName.name, FuncName );
    }

    else if ( ( PtrFuncName->type != tFunction ) )
    {
        SetError ( E_SEM_OTHER, "Function cannot have name like existing variable!" );
    }

    tFuncData* FuncRec = FuncHTRead ( FTable, FuncName.name );

    I = MakeInstruction ( I_COPYTAB, PtrFuncName, NULL, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );


    // function declaration was not found yet
    if ( FuncRec == NULL )
    {
        TStructToken* ArgArray = NULL;

        unsigned long q = 0;
        ArgArray = malloc ( ( sizeof ( TStructToken ) ) * PARAM_COUNT );

        if ( ArgArray == NULL )
        {
            SetError ( E_ERR_OTHER, "parser.c - UserFunctionCall: Memory allocation error!" );
        }


        while ( SetArgPtr ( &PtrTmp ) )
        {
            if ( q % PARAM_COUNT == 0 )
            {

                TStructToken* tmp = NULL;
                tmp = realloc ( ArgArray, ( sizeof ( TStructToken ) ) * ( q + PARAM_COUNT + 1 ) );

                if ( tmp == NULL )
                {
                    free ( ArgArray );
                    SetError ( E_ERR_OTHER, "parser.c - UserFunctionCall: Memory allocation error!" );
                }

                ArgArray = tmp;

            }

            q++;
            I = MakeInstruction ( I_INSPAR, & ( ArgArray[q] ), PtrTmp, NULL );
            listPostInsert ( &L, I );
            listNext ( &L );

            if ( !accepted ( tComma ) )
            {
                break;
            }
        }

        TFuncItem NewFuncRecord;
        NewFuncRecord.fargc  = q;
        NewFuncRecord.ftable = NULL;
        NewFuncRecord.fargv  = ArgArray;
        NewFuncRecord.valid  = false;
        FuncHTInsert ( FTable, AllocateStr ( FuncName.name ), NewFuncRecord );
    }

    else
    {
        unsigned long q = 0;

        while ( SetArgPtr ( &PtrTmp ) )
        {
            q++;

            if ( q <= FuncRec->fargc )
            {
                I = MakeInstruction ( I_INSPAR, & ( FuncRec->fargv[q] ), PtrTmp, NULL );
                listPostInsert ( &L, I );
                listNext ( &L );
            }

            if ( !accepted ( tComma ) )
            {
                break;
            }
        }
    }

    I = MakeInstruction ( I_CALL, PtrFuncName, NULL, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );
}

////////////////////////////////////////////////////////////////////////////////

/*
* Built-in function generator with 0 arguments
*/
void BuiltInFunctionCall_0 ( TInstr I_type )
{
    TStructToken* PtrTmp = NULL;

    // check useless arguments
    do
    {
        SetArgPtr ( &PtrTmp );
    }
    while ( accepted ( tComma ) );

    I = MakeInstruction ( I_type, NULL, NULL, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );
}

/*
* Built-in function generator with 1 argument
*/
void BuiltInFunctionCall_1 ( TInstr I_type )
{
    TStructToken* PtrTmp  = NULL;
    TStructToken* PtrTmp1 = NULL;

    // if function doesnt have 1st argument
    if ( !SetArgPtr ( &PtrTmp1 ) )
    {
        // create a NIL which will be used as 1st argument
        PtrTmp1 = InsertTerm ( GenerateDoubleToken ( NULL, tNil, 0.0 ) );
    }

    else
    {
        if ( accepted ( tComma ) )
        {
            // check useless arguments
            do
            {
                SetArgPtr ( &PtrTmp );
            }
            while ( accepted ( tComma ) );
        }
    }

    I = MakeInstruction ( I_type, PtrTmp1, NULL, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );
}

/*
* Built-in function generator with 1 argument (TYPEOF)
*/
void BuiltInFunctionCallTypeOf ( TInstr I_type )
{
    TStructToken* PtrTmp  = NULL;
    TStructToken* PtrTmp1 = NULL;

    // if function doesnt have 1st argument
    if ( !SetArgPtrFunc ( &PtrTmp1 ) )
    {
        // create a NIL which will be used as 1st argument
        PtrTmp1 = InsertTerm ( GenerateDoubleToken ( NULL, tNil, 0.0 ) );
    }

    else
    {
        if ( accepted ( tComma ) )
        {
            // check useless arguments
            do
            {
                SetArgPtrFunc ( &PtrTmp );
            }
            while ( accepted ( tComma ) );
        }
    }

    I = MakeInstruction ( I_type, PtrTmp1, NULL, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );
}

/*
* Built-in function generator with 2 arguments
*/
void BuiltInFunctionCall_2 ( TInstr I_type )
{
    TStructToken* PtrTmp  = NULL;
    TStructToken* PtrTmp1 = NULL;
    TStructToken* PtrTmp2 = NULL;

    // if function doesnt have 1st argument
    if ( !SetArgPtr ( &PtrTmp1 ) )
    {
        // create a NIL which will be used as 1st argument
        PtrTmp1 = InsertTerm ( GenerateDoubleToken ( NULL, tNil, 0.0 ) );
    }

    else
    {
        if ( accepted ( tComma ) )
        {
            // if function doesnt have 2nd argument
            if ( !SetArgPtr ( &PtrTmp2 ) )
            {
                // create a NIL which will be used as 2nd argument
                PtrTmp2 = InsertTerm ( GenerateDoubleToken ( NULL, tNil, 0.0 ) );
            }

            else
            {
                if ( accepted ( tComma ) )
                {
                    // check useless arguments
                    do
                    {
                        SetArgPtr ( &PtrTmp );
                    }
                    while ( accepted ( tComma ) );
                }
            }
        }
    }

    I = MakeInstruction ( I_type, PtrTmp1, PtrTmp2, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );
}

/*
* Built-in function generator with N arguments (PRINT)
*/
void BuiltInFunctionCall_N ( TInstr I_type )
{
    TStructToken* PtrTmp = NULL;

    // check every argument and ...
    while ( SetArgPtr ( &PtrTmp ) )
    {
        // ...insert instruction
        I = MakeInstruction ( I_type, PtrTmp, NULL, NULL );
        listPostInsert ( &L, I );
        listNext ( &L );

        if ( !accepted ( tComma ) )
        {
            break;
        }
    }

    // one more instruction to be sure that NIL will be returned
    I = MakeInstruction ( I_type, NULL, NULL, NULL );
    listPostInsert ( &L, I );
    listNext ( &L );
}

/******************************************************************************/

