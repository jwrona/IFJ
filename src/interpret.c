/*
* File:                 interpret.c
* Description:          Interpretation of the sequence of instructions.
* Author:               Michal Duban     -   xduban01
*
* Projekt:              IFJ-2012
* Team 34:
*                       Duban Michal    -   xduban01
*                       Hanselka Vaclav -   xhanse00
*                       Heczkova Petra  -   xheczk04
*                       Fabry Marko     -   xfabry01
*                       Wrona Jan       -   xwrona00
*/

#define LEN_MAX 16

/**
 * Funkce alokujici pamet pro textove retezce.
 */
char* AllocateStrDynamic ( char* tmp )
{

    if ( tmp == NULL )
    {
        char* str = ( void* ) malloc ( sizeof ( char ) * ( 1 ) );

        if ( str == NULL )
        {
            SetError ( E_ERR_OTHER, "AllocateStrDynamic: Malloc error!" );
        }

        str[0] = '\0';
        return str;
    }

    char* str = ( void* ) malloc ( sizeof ( char ) * ( strlen ( tmp ) + 1 ) );

    if ( str == NULL )
    {
        SetError ( E_ERR_OTHER, "AllocateStrDynamic: Malloc error!" );
    }

    str = strcpy ( str, tmp );
    return str;
}

/**
 * Funkce alokujici pamet pro uchovani vysledku aritmetickych/bool hodnot, ktere
 * jsou vlozeny do noveho tokenu, ktery je funkci vracen.
 */
TStructToken GenerateDoubleTokenDynamic ( char* name, TToken type, double data )
{
    double* dabl = ( void* ) malloc ( sizeof ( double ) );

    if ( dabl == NULL )
    {
        SetError ( E_ERR_OTHER, "GenerateDoubleTokenDynamic: Malloc error!" );
    }

    *dabl = data;

    TStructToken tmp;
    tmp.name = name;
    tmp.type = type;
    tmp.data = dabl;
    return tmp;
}

/**
 * Funkce alokujici pamet pro uchovani vysledku pri praci s textovymi retezci, ktere
 * jsou vlozeny do noveho tokenu, ktery je funkci vracen.
 */
TStructToken GenerateStringTokenDynamic ( char* name, TToken type, char* data )
{
    // alokujeme pamet podle delky textoveho retezce
    char* str = ( void* ) malloc ( sizeof ( char ) * ( strlen ( data ) + 1 ) );

    if ( str == NULL )
    {
        SetError ( E_ERR_OTHER, "GenerateStringTokenDynamic: Malloc error!" );
    }

    str = strcpy ( str, data );

    TStructToken tmp;
    tmp.name = name;
    tmp.type = type;
    tmp.data = str;
    return tmp;
}

/**
 * Funkce pro samotnou interpretaci, ktera je realizovana nekonecnym cyklem,
 * ktery lze ukoncit nalezenim instrukce I_STOP, nebo nalezenim chyby.
 */
int inter ( tListOfInstr* instrList )
{
    listFirst ( instrList );
    tInstr* I;

    TStructToken* result;
    TStructToken* operand1;
    TStructToken* operand2;

    double* dabl1 = NULL;   // inicializace promennych pro vysledky ciselnych operaci
    double* dabl2 = NULL;
    double* dabl3 = NULL;

    char* str1 = NULL;  // inicializace promennych pro vysledky operaci nad textem
    char* str2 = NULL;
    char* str3 = NULL;

    tFuncData* FuncRec = FuncHTRead ( FTable, "@main" );
    ActDynamicTab = FuncRec->ftable;

    while ( 42 )
    {
        I = listGetData ( instrList );  // nacteme prvni instrukci

        result   = NULL;
        operand1 = NULL;
        operand2 = NULL;

        // kontrola zda jednotlive adresy s daty nejsou NULL
        if ( ( I->addr1 != NULL ) && ( I->addr1->name != NULL ) )
        {
            result = htRead ( ActDynamicTab, I->addr1->name );
        }

        if ( ( I->addr2 != NULL ) && ( I->addr2->name != NULL ) )
        {
            operand1 = htRead ( ActDynamicTab, I->addr2->name );
        }

        if ( ( I->addr3 != NULL ) && ( I->addr3->name != NULL ) )
        {
            operand2 = htRead ( ActDynamicTab, I->addr3->name );
        }

        switch ( I->instType )
        {
                // NULL NULL NULL
            case I_STOP:   // korektni ukonceni interpretace
            {
                return 0;
            }
            break;

            // NULL NULL NULL
            case I_LAB: // instrukce navesti
            {

            }
            break;

            // LABEL NULL NULL
            case I_GOTO:
            {
                listGoto ( instrList, I->addr1->data ); // skok na zadanou instrukci
            }
            break;

            // LABEL CONDITION NULL
            case I_IFGOTO: // instrukce podmineneho skoku
            {
                switch ( operand1->type )
                {
                        // pokud se jedna o cislo, bool, nil
                    case tNumeric:
                    case tBoolean:
                    case tNil:
                        dabl1 = operand1->data;

                        if ( *dabl1 == 0.0 )    // a podminka je TRUE
                        {
                            listGoto ( instrList, I->addr1->data ); //skaceme
                        }

                        break;

                        // pokud se jedna o typ string
                    case tString:
                        str1 = operand1->data;

                        if ( strcmp ( str1, "" ) )
                        {
                            listGoto ( instrList, I->addr1->data );
                        }

                        break;

                    default:    // pokud nemame ani jeden z vyse zminovanych typu - chyba
                        SetError ( E_INT_OTHER, "interpret.c: I_IFGOTO: Unknown type!" );
                        break;
                }
            }
            break;

            // FUNCTION NULL NULL
            case I_COPYTAB: // instrukce kopitujici tabulky
            {
                // pokud mame zadanou funkcni tabulku kterou mame kopirovat
                if ( I->addr1 != NULL )
                {
                    // precteme data
                    tFuncData* FuncRec = FuncHTRead ( FTable, I->addr1->name );

                    tHTable* table = malloc ( sizeof ( tHTable[HTSIZE] ) );

                    if ( table == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    htInit ( table );   // pokud je alokace v poradku inicializujeme s zkopitujeme tabulku
                    ActDynamicTab = htCopyTable ( table, FuncRec->ftable );
                }

                else    // pokud nemame zadanu kterou tabulku mame kopirovat - chyba
                {
                    SetError ( E_ERR_OTHER, "interpret.c: COPYTAB FATAL ERROR!" );
                }
            }
            break;

            // ARGUMENT_NAME ARGUMENT_VALUE NULL
            case I_INSPAR:
            {
                if ( ( I->addr1 != NULL ) && ( I->addr2 != NULL ) )
                {
                    operand1 = htRead ( MyStack.Top->data.table, I->addr2->name );

                    TStructToken Arg;
                    Arg.name = AllocateStrDynamic ( I->addr1->name );
                    Arg.type = operand1->type;

                    if ( operand1->type == tNumeric ||
                            operand1->type == tBoolean ||
                            operand1->type == tNil )
                    {
                        dabl2 = operand1->data;
                        dabl1 = ( void* ) malloc ( sizeof ( double ) );

                        *dabl1 = *dabl2;
                        Arg.data = dabl1;

                    }

                    else if ( operand1->type == tString )
                    {
                        str2 = operand1->data;
                        str1 = ( void* ) malloc ( sizeof ( char ) * strlen ( str2 ) + 1 );

                        str1 = strcpy ( str1, str2 );
                        Arg.data = str1;
                    }

                    htInsertDynamic ( ActDynamicTab, Arg.name, Arg );   // vlozime do tabulky
                }

                else
                {
                    SetError ( E_ERR_OTHER, "interpret.c: INSPAR FATAL ERROR!" );
                }
            }
            break;

            // FUNCTION NULL NULL
            case I_CALL:    // instrukce volani fce
            {
                //htPrintTable(ActDynamicTab);
                // nacteme data z prislusne tabulky
                tFuncData* FuncRec = FuncHTRead ( FTable, I->addr1->name );

                TMyStackItem It;
                It.table      = ActDynamicTab;
                It.label.data = instrList->active;

                // vlozime na zasobnik
                ST_Push ( &MyStack, It );

                // prejdeme na navesti zacatku funkce
                listGoto ( instrList, FuncRec->fstart.data );

                //htPrintTable(MyStack.Top->data.table);

                //fprintf ( stderr, "zanorujem sa***********************************\n" );
            }
            break;

            // NULL NULL NULL
            case I_RET: // instrukce navratu z funkce
            {
                //htPrintTable(MyStack.Top->data.table);

                if ( MyStack.Top != NULL )  // pokud nemame prazdny zasobnik
                {
                    listGoto ( instrList, MyStack.Top->data.label.data );

                    if ( MyStack.Top->next != NULL )    // pokud na zasobniku neni jedina polozka
                    {
                        TStructToken* ret = htRead ( MyStack.Top->data.table, "@return" );
                        TStructToken res;

                        if ( ret->type == tNumeric ||
                                ret->type == tBoolean ||
                                ret->type == tNil )
                        {

                            dabl1 = ret->data;

                            res = GenerateDoubleTokenDynamic ( AllocateStrDynamic ( "@result" ), ret->type, *dabl1 );
                        }

                        else if ( ret->type == tString )
                        {
                            str1 = ret->data;
                            res = GenerateStringTokenDynamic ( AllocateStrDynamic ( "@result" ), ret->type, str1 );
                        }

                        htInsertDynamic ( MyStack.Top->next->data.table, res.name , res );

                        //fprintf ( stderr, "interpret.c:  %s [%s] <= %s [%s]\n", res.name, TOKENSIGN[res.type], ret->name, TOKENSIGN[ret->type] );

                        //htPrintTable(MyStack.Top->next->data.table);

                        ActDynamicTab = MyStack.Top->next->data.table;
                    }
                }

                //htPrintTable(ActDynamicTab);

                //htPrintTable(MyStack.Top->data.table);

                ST_Pop ( &MyStack );    // odstranime polozku z vrcholu zasobniku

                //fprintf ( stderr, "vynorujem sa***********************************\n" );
            }
            break;

            // RESULT OPERAND NULL
            case I_ASS: // instrukce prirazeni
            {
                if ( operand1->type == tNumeric ||
                        operand1->type == tBoolean ||
                        operand1->type == tNil )
                {
                    // pokud mame cislo, bool nebo nil
                    dabl2 = operand1->data;
                    // naalokujeme misto pro vysledek
                    dabl1 = ( void* ) malloc ( sizeof ( double ) );

                    *dabl1 = *dabl2;    // priradime hodnoty

                    free ( result->data );
                    result->data = dabl1;
                    result->type = operand1->type;
                }

                else if ( operand1->type == tString )   // pokud mame text. retezec
                {
                    str2 = operand1->data;
                    str1 = ( void* ) malloc ( sizeof ( char ) * strlen ( str2 ) + 1 );

                    str1 = strcpy ( str1, str2 );

                    free ( result->data );
                    result->data = str1;
                    result->type = operand1->type;
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: ASS: Incompatible types!" );
                }

                //fprintf ( stderr, "interpret.c:  %s = %s\n", result->name, operand1->name );
                //fprintf ( stderr, "interpret.c:  %s = %s\n", TOKENSIGN[result->type], TOKENSIGN[operand1->type] );

                //htPrintTable(ActDynamicTab);
            }
            break;

            // RESULT OPERAND1 OPERAND2
            case I_ADD: // instrukce scitani
            {
                switch ( operand1->type )   // kontrola typu
                {
                    case tNumeric:
                    {
                        switch ( operand2->type )
                        {
                            case tNumeric:
                            {
                                dabl1 = ( double* ) malloc ( sizeof ( double ) );

                                if ( dabl1 == NULL )
                                {
                                    SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                                }

                                dabl2 = operand1->data;
                                dabl3 = operand2->data;

                                *dabl1 = *dabl2 + *dabl3;

                                free ( result->data );
                                result->data = dabl1;
                                result->type = tNumeric;

                                //fprintf ( stderr, "interpret.c:  %g + %g = %g\n", *dabl2, *dabl3, *dabl1 );
                            }
                            break;

                            default:    // pri pokusu od + u nekorektnich typu chyba
                            {
                                SetError ( E_INT_TYPE, "interpret.c: ADD: Incompatible types!" );
                            }
                            break;
                        }
                    }
                    break;

                    case tString:
                    {
                        int delka1 = 0;
                        int delka2 = LEN_MAX;
                        str1 = operand1->data;
                        str2 = NULL;
                        delka1 = strlen ( str1 );

                        switch ( operand2->type )   // kontrola typu
                        {
                            case tNumeric:
                            {
                                str2 = ( char* ) malloc ( ( delka2 + 1 ) * sizeof ( char ) );

                                if ( str2 == NULL )
                                {
                                    SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                                }

                                dabl3 = operand2->data;
                                //memset(str2, '\0', delka2);
                                // do str2 ulozeno cislo jak bz bzlo vypsano procentem g
                                sprintf ( str2, "%g", *dabl3 );
                            }
                            break;

                            case tBoolean:
                            {

                                str2 = ( char* ) malloc ( ( delka2 + 1 ) * sizeof ( char ) );

                                if ( str2 == NULL )
                                {
                                    SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                                }

                                dabl3 = operand2->data;

                                if ( *dabl3 == 1.0 )    // pokud dostaneme hodnotu TRUE
                                {
                                    str2 = "true";  // text bude "true"
                                }

                                else
                                {
                                    str2 = "false";
                                }

                            }
                            break;

                            case tNil:
                            {

                                str2 = ( char* ) malloc ( ( delka2 + 1 ) * sizeof ( char ) );

                                if ( str2 == NULL )
                                {
                                    SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                                }

                                dabl3 = operand2->data;

                                str2 = "Nil";
                            }
                            break;

                            case tString:
                            {

                                str2 = operand2->data;

                                if ( str2 == NULL ) // pokud mame prazdny string
                                {
                                    delka2 = 0; // delka je nulova
                                    str2 = "";  // vlozime "nic"
                                }

                                else   // jinak delka2 = delka stringu
                                {
                                    delka2 = strlen ( str2 );
                                }

                            }
                            break;

                            default:
                            {
                                SetError ( E_INT_TYPE, "interpret.c: ADD: Incompatible types!" );
                            }
                            break;

                        }

                        if ( ( str3 = ( char* ) malloc ( ( delka1 + delka2 + 1 ) * sizeof ( char ) ) ) == NULL )    // naalokujeme misto na vysledny string
                        {
                            SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                        }

                        memset ( str3, '\0', ( delka1 + delka2 + 1 ) );
                        strcat ( str3, str1 );  //kopirujeme texty
                        strcat ( str3, str2 );

                        free ( result->data );
                        result->data = str3;
                        result->type = tString;
                        //fprintf ( stderr, "interpret.c: '%s' + '%s' = '%s'\n", str1, str2, str3 );

                        if ( operand2->type != tString )
                        {
                            free ( str2 );  // pokud jsme nemeli typ string uvolnime str2 ("")
                        }

                    }
                    break;

                    default:
                    {
                        SetError ( E_INT_TYPE, "interpret.c: ADD: Incompatible types!" );
                    }
                    break;
                }
            }
            break;

            // RESULT OPERAND1 OPERAND2
            case I_SUB: // instrukce rozdilu
            {
                if ( ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) ) // kontrola spravnosti typu
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    dabl2 = operand1->data;
                    dabl3 = operand2->data;

                    *dabl1 = *dabl2 - *dabl3;

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tNumeric;

                    //fprintf ( stderr, "interpret.c:  %g - %g = %g\n", *dabl2, *dabl3, *dabl1 );
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: SUB: Incompatible types!" );
                }

            }
            break;

            // RESULT OPERAND1 OPERAND2
            case I_MUL: // instrukce soucinu
            {
                if ( ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) )
                {
                    // pokud mame klasicke ciselne hodnoty, provedeme klasicky soucin
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    dabl2 = operand1->data;
                    dabl3 = operand2->data;

                    *dabl1 = *dabl2  *  *dabl3;

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tNumeric;

                    //fprintf ( stderr, "interpret.c:  %g * %g = %g\n", *dabl2, *dabl3, *dabl1 );
                }

                else if ( ( operand1->type == tString ) && ( operand2->type == tNumeric ) )
                {
                    // pokud mame text a ciselnou hodnotu, provadime iteraci
                    int delka = 1;
                    str1 = operand1->data;
                    delka = strlen ( str1 );
                    dabl1 = operand2->data;

                    if ( *dabl1 < 0.0 )
                    {
                        SetError ( E_INT_TYPE, "interpret.c: MUL: Incompatible types!" );
                    }

                    long cela_cast = trunc ( *dabl1 );  // odsekneme desetinnou cast

                    str2 = malloc ( ( cela_cast * ( delka + 1 ) + 1 ) * sizeof ( char ) );

                    if ( str2  == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    if ( cela_cast == 0 )
                    {
                        str2[0] = '\0';
                    }

                    else
                    {
                        str2 = strcpy ( str2, str1 );

                        for ( long i = 1; i < cela_cast; i++ )
                        {
                            str2 = strcat ( str2, str1 );   // kopirujeme text
                        }
                    }

                    free ( result->data );
                    result->data = str2;
                    result->type = tString;
                    //fprintf(stderr, "interpret.c: '%s' * %g = '%s'\n", str1, cela_cast, str2);
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: MUL: Incompatible types!" );
                }

            }
            break;

            // RESULT OPERAND1 OPERAND2
            case I_DIV: // instrukce podilu
            {
                if ( ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) )
                {
                    dabl2 = operand1->data;
                    dabl3 = operand2->data;

                    if ( *dabl3 == 0.0 )
                    {
                        SetError ( E_INT_DIV_BY_ZERO, "interpret.c: Devided by zero!" );
                    }

                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }


                    else
                    {
                        *dabl1 = *dabl2 / *dabl3;
                        free ( result->data );
                        result->data = dabl1;
                        result->type = tNumeric;
                        //fprintf ( stderr, "interpret.c:  %g / %g = %g\n", *dabl2, *dabl3, *dabl1 );
                    }
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: DIV: Incompatible types!" );
                }
            }
            break;

            // RESULT OPERAND1 OPERAND2
            case I_POW: // instrukce mocniny
            {
                if ( ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    dabl2 = operand1->data;
                    dabl3 = operand2->data;
                    *dabl1 = pow ( *dabl2, *dabl3 );

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tNumeric;

                    // fprintf ( stderr, "interpret.c:  %g ** %g = %g\n", *dabl2, *dabl3, *dabl1 );
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: POW: Incompatible types!" );
                }
            }
            break;

            ////////////////////////////////////////////////////////////////////////////////
            // RESULT OPERAND1 OPERAND2
            case I_EQ:  // ==
            {
                if ( ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    dabl2 = operand1->data;
                    dabl3 = operand2->data;

                    if ( *dabl2 == *dabl3 ) // pokud se operadny rovnaji
                    {
                        *dabl1 = 1.0;   // do vysledku nastavime TRUE
                    }

                    else
                    {
                        *dabl1 = 0.0;   // jinak false
                    }

                    // fprintf ( stderr, "interpret.c:  %g == %g : %g\n", *dabl2, *dabl3, *dabl1 );
                }

                else if ( ( operand1->type == tString ) && ( operand2->type == tString ) )
                {
                    // pokud mame stringy
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    str2 = operand1->data;
                    str3 = operand2->data;

                    // porovname lexikograficky
                    if ( ( strcmp ( str2, str3 ) ) == 0 )
                    {
                        *dabl1 = 1.0;   // pokud se shoduji nastavime TRUE
                    }

                    else
                    {
                        *dabl1 = 0.0;   // pokud ne FALSE
                    }

                    // fprintf ( stderr, "interpret.c:  '%s' == '%s' : %g\n", str2, str3, *dabl1 );
                }

                // pokud mame nsled. kombinace typu
                else if ( ( operand1->type == tNil     ||
                            operand1->type == tNumeric ||
                            operand1->type == tString  ||
                            operand1->type == tBoolean ) &&
                          ( operand2->type == tNil     ||
                            operand2->type == tNumeric ||
                            operand2->type == tString  ||
                            operand2->type == tBoolean ) )
                {
                    *dabl1 = 0.0;   // nastavime FALSE
                    // fprintf ( stderr, "interpret.c: EQ : %g\n", *dabl1 );
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: EQ: Incompatible types!" );
                }

                free ( result->data );
                result->data = dabl1;
                result->type = tBoolean;
            }
            break;

            // RESULT OPERAND1 OPERAND2
            case I_NEQ: // !=
            {
                if ( ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    dabl2 = operand1->data;
                    dabl3 = operand2->data;

                    if ( *dabl2 == *dabl3 )
                    {
                        *dabl1 = 0.0;
                    }

                    else
                    {
                        *dabl1 = 1.0;
                    }

                    // fprintf ( stderr, "interpret.c:  %g != %g : %g\n", *dabl2, *dabl3, *dabl1 );
                }

                else if ( ( operand1->type == tString ) && ( operand2->type == tString ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    str2 = operand1->data;
                    str3 = operand2->data;

                    if ( ( strcmp ( str2, str3 ) ) == 0 )
                    {
                        *dabl1 = 0.0;
                    }

                    else
                    {
                        *dabl1 = 1.0;
                    }

                    // fprintf ( stderr, "interpret.c:  '%s' != '%s' : %g\n", str2, str3, *dabl1 );
                }

                else if ( ( operand1->type == tNil     ||
                            operand1->type == tNumeric ||
                            operand1->type == tString  ||
                            operand1->type == tBoolean ) &&

                          ( operand2->type == tNil     ||
                            operand2->type == tNumeric ||
                            operand2->type == tString  ||
                            operand2->type == tBoolean ) )
                {
                    *dabl1 = 1.0;
                    // fprintf ( stderr, "interpret.c: NEQ : %g\n", *dabl1 );
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: NEQ: Incompatible types!" );
                }

                free ( result->data );
                result->data = dabl1;
                result->type = tBoolean;
            }
            break;


            // RESULT OPERAND1 OPERAND2
            case I_GT:  // >
            {

                if ( ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    dabl2 = operand1->data;
                    dabl3 = operand2->data;

                    if ( *dabl2 > *dabl3 )
                    {
                        *dabl1 = 1.0;
                    }

                    else
                    {
                        *dabl1 = 0.0;
                    }

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tBoolean;

                    // fprintf ( stderr, "interpret.c:  %g > %g : %g\n", *dabl2, *dabl3, *dabl1 );
                }

                else if ( ( operand1->type == tString ) && ( operand2->type == tString ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    str2 = operand1->data;
                    str3 = operand2->data;

                    if ( ( strcmp ( str2, str3 ) ) > 0 )
                    {
                        *dabl1 = 1.0;
                    }

                    else
                    {
                        *dabl1 = 0.0;
                    }

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tBoolean;

                    // fprintf ( stderr, "interpret.c:  '%s' > '%s' : %g\n", str2, str3, *dabl1 );
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: GT: Incompatible types!" );
                }
            }
            break;

            // RESULT OPERAND1 OPERAND2
            case I_GTE: // >=
            {

                if ( ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    dabl2 = operand1->data;
                    dabl3 = operand2->data;

                    if ( *dabl2 >= *dabl3 )
                    {
                        *dabl1 = 1.0;
                    }

                    else
                    {
                        *dabl1 = 0.0;
                    }

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tBoolean;

                    // fprintf ( stderr, "interpret.c:  %g >= %g : %g\n", *dabl2, *dabl3, *dabl1 );
                }

                else if ( ( operand1->type == tString ) && ( operand2->type == tString ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    str2 = operand1->data;
                    str3 = operand2->data;

                    if ( ( strcmp ( str2, str3 ) ) >= 0 )
                    {
                        *dabl1 = 1.0;
                    }

                    else
                    {
                        *dabl1 = 0.0;
                    }

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tBoolean;

                    // fprintf ( stderr, "interpret.c:  '%s' >= '%s' : %g\n", str2, str3, *dabl1 );
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: GTE: Incompatible types!" );
                }
            }
            break;

            // RESULT OPERAND1 OPERAND2
            case I_LT:  // <
            {

                if ( ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    dabl2 = operand1->data;
                    dabl3 = operand2->data;

                    if ( *dabl2 < *dabl3 )
                    {
                        *dabl1 = 1.0;
                    }

                    else
                    {
                        *dabl1 = 0.0;
                    }

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tBoolean;

                    // fprintf ( stderr, "interpret.c:  %g < %g : %g\n", *dabl2, *dabl3, *dabl1 );
                }

                else if ( ( operand1->type == tString ) && ( operand2->type == tString ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    str2 = operand1->data;
                    str3 = operand2->data;

                    if ( ( strcmp ( str2, str3 ) ) < 0 )
                    {
                        *dabl1 = 1.0;
                    }

                    else
                    {
                        *dabl1 = 0.0;
                    }

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tBoolean;

                    // fprintf ( stderr, "interpret.c:  '%s' < '%s' : %g\n", str2, str3, *dabl1 );
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: LT: Incompatible types!" );
                }
            }
            break;

            // RESULT OPERAND1 OPERAND2
            case I_LTE: // <=
            {

                if ( ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    dabl2 = operand1->data;
                    dabl3 = operand2->data;

                    if ( *dabl2 <= *dabl3 )
                    {
                        *dabl1 = 1.0;
                    }

                    else
                    {
                        *dabl1 = 0.0;
                    }

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tBoolean;

                    // fprintf ( stderr, "interpret.c:  %g <= %g : %g\n", *dabl2, *dabl3, *dabl1 );
                }

                else if ( ( operand1->type == tString ) && ( operand2->type == tString ) )
                {
                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                    if ( dabl1 == NULL )
                    {
                        SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                    }

                    str2 = operand1->data;
                    str3 = operand2->data;

                    if ( ( strcmp ( str2, str3 ) ) <= 0 )
                    {
                        *dabl1 = 1.0;
                    }

                    else
                    {
                        *dabl1 = 0.0;
                    }

                    free ( result->data );
                    result->data = dabl1;
                    result->type = tBoolean;

                    // fprintf ( stderr, "interpret.c:  '%s' <= '%s' : %g\n", str2, str3, *dabl1 );
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: LTE: Incompatible types!" );
                }
            }
            break;

            // ARG NULL NULL
            case I_PRINT:   // instrukce vypisu
            {
                if ( result != NULL )
                {
                    switch ( result->type )
                    {
                        case tNumeric:  // vypis cisla v formate %g
                        {
                            dabl1 = result->data;
                            printf ( "%g", *dabl1 );
                        }
                        break;

                        case tString:   // vypis textu
                        {
                            str1 = result->data;
                            printf ( "%s", str1 );
                        }
                        break;

                        case tBoolean:
                        {
                            dabl1 = result->data;

                            if ( *dabl1 )   // pokud mame zadanou bool hodnotu 1.0
                            {
                                printf ( "true" );  // vypis true
                            }

                            else   // pokud mame zadanou bool hodnotu 0.0
                            {
                                printf ( "false" );  // vypis false
                            }
                        }
                        break;

                        case tNil:
                        {
                            printf ( "Nil" );   // vypis Nil
                        }
                        break;

                        default:
                        {
                            SetError ( E_INT_TYPE, "interpret.c: PRINT: Incompatible types!" );
                        }
                        break;
                    }
                }

                TStructToken  Result = GenerateDoubleTokenDynamic ( AllocateStrDynamic ( "@result" ), tNil, 0.0 );

                htInsertDynamic ( ActDynamicTab, Result.name, Result );
            }
            break;

            case I_INPUT:   // instrukce nacitani
            {
                char* tokenstr = NULL;
                int symbol     = 0;

                symbol = fgetc ( stdin );   // nacitame znak ze standart vstupu

                if ( symbol == '\n' )
                {
                    append ( &tokenstr, '\0' );
                }

                while ( symbol != '\n' )    // nacitame dokun nenarazime na konec radku
                {
                    append ( &tokenstr, symbol );
                    symbol = fgetc ( stdin );
                }

                TStructToken Result;
                Result.name = AllocateStrDynamic ( "@result" );
                Result.type = tString;
                Result.data = tokenstr;

                htInsertDynamic ( ActDynamicTab, Result.name , Result );
            }
            break;

            // RESULT NULL NULL
            case I_NUMERIC: // konverze stringu na cislo
            {
                switch ( result->type )
                {
                    case tNumeric:
                    {
                        dabl2 = result->data;
                        dabl1 = ( void* ) malloc ( sizeof ( double ) );

                        if ( dabl1 == NULL )
                        {
                            SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                        }

                        *dabl1 = *dabl2;

                        TStructToken res;
                        res.name = AllocateStrDynamic ( "@result" );
                        res.type = tNumeric;
                        res.data = dabl1;
                        htInsertDynamic ( ActDynamicTab, res.name , res );
                    }
                    break;

                    case tBoolean:
                    case tNil:
                    default:
                    {
                        SetError ( E_INT_NUMERIC, "interpret.c: NUMERIC: Incompatible types!" );
                    }
                    break;

                    case tString:   // pokud mame string
                    {
                        char* str = result->data;
                        char* str1 = NULL;
                        unsigned long j = 0;
                        int symbol = 0;

                        if ( strlen ( str ) == 0 )
                        {
                            SetError ( E_INT_NUMERIC, "Numerical literal errors" );
                        }

                        symbol = str[j++];

                        // prochazime znak po znaku a kontrolujeme zda je string reprezentujici double bez chyby
                        while ( isspace ( symbol ) )
                        {
                            symbol = str[j++];
                        }

                        if ( !isdigit ( symbol ) )
                        {
                            SetError ( E_INT_NUMERIC, "Numerical literal error!" );
                        }

                        while ( isdigit ( symbol ) )
                        {
                            append ( &str1, symbol );
                            symbol = str[j++];
                        }

                        if ( symbol != '.' && symbol != 'e' )
                        {
                            SetError ( E_LEX, "Numerical literal error . / e!" );
                        }

                        if ( symbol == '.' )
                        {
                            append ( &str1, symbol );

                            symbol = str[j++];

                            if ( isdigit ( symbol ) )
                            {

                                while ( isdigit ( symbol ) ) //cteni desetinne casti
                                {
                                    append ( &str1, symbol );
                                    symbol = str[j++];
                                }

                                if ( symbol != 'e' )
                                {
                                    dabl1 = ( double* ) malloc ( sizeof ( double ) );

                                    if ( dabl1 == NULL )
                                    {
                                        SetError ( E_ERR_OTHER, "interpret.c: Malloc Error!" );
                                    }

                                    *dabl1 = strtod ( str1, NULL ); //nejak poresit preteceni

                                    if ( str != NULL )
                                    {
                                        free ( str1 );
                                    }

                                }
                            }

                            else
                            {
                                SetError ( E_INT_NUMERIC, "Numerical literal error (decimal part)" );
                            }
                        }

                        if ( symbol == 'e' ) //exponencialni cast
                        {
                            append ( &str1, symbol );
                            symbol = str[j++];


                            if ( ( !isdigit ( symbol ) ) && ( symbol != '+' ) && ( symbol != '-' ) )
                            {
                                SetError ( E_INT_NUMERIC, "Numerical literal error (exponential part)" );
                            }

                            if ( symbol == '+' || symbol == '-' )
                            {
                                append ( &str1, symbol );
                                symbol = str[j++];

                                if ( !isdigit ( symbol ) )
                                {
                                    SetError ( E_INT_NUMERIC, "Numerical literal error (exponential part)" );
                                }
                            }

                            append ( &str1, symbol );
                            symbol = str[j++];


                            while ( isdigit ( symbol ) ) //cteni exponencialni casti
                            {
                                append ( &str1, symbol );
                                symbol = str[j++];

                            }

                            dabl1 = ( double* ) malloc ( sizeof ( double ) );

                            if ( dabl1 == NULL )
                            {
                                SetError ( E_ERR_OTHER, "scan: Malloc Error!!!" );
                            }

                            *dabl1 = strtod ( str1, NULL ); //nejak poresit preteceni

                            if ( str != NULL )
                            {
                                free ( str1 );
                            }
                        }

                        TStructToken res;
                        res.name = AllocateStrDynamic ( "@result" );
                        res.type = tNumeric;
                        res.data = dabl1;
                        htInsertDynamic ( ActDynamicTab, res.name , res );
                    }
                }
            }
            break;

            // RESULT NULL NULL
            case I_TYPEOF:  // instrukce typu
            {
                double tmp = 0.0;

                switch ( result->type )
                {
                    case tNil:
                    {
                        tmp = 0.0;
                    }
                    break;

                    case tBoolean:
                    {
                        tmp = 1.0;
                    }
                    break;

                    case tNumeric:
                    {
                        tmp = 3.0;
                    }
                    break;

                    case tFunction:
                    {
                        tmp = 6.0;
                    }
                    break;

                    case tString:
                    {
                        tmp = 8.0;
                    }
                    break;

                    default:
                    {
                        SetError ( E_INT_TYPE, "interpret.c: TYPEOF: Incompatible types!" );
                    }
                    break;

                }

                TStructToken Result = GenerateDoubleTokenDynamic ( AllocateStrDynamic ( "@result" ), tNumeric, tmp );
                htInsertDynamic ( ActDynamicTab, Result.name , Result );
            }
            break;

            // RESULT NULL NULL
            case I_LEN: // instrukce delky retezce
            {
                dabl1 = ( void* ) malloc ( sizeof ( double ) );

                if ( dabl1 == NULL )
                {
                    SetError ( E_ERR_OTHER, "interpret.c: Malloc error!" );
                }

                switch ( result->type )
                {
                        // pro naslednujici hodnoty implicitne nastaveno na 0.0
                    case tNil:
                    case tBoolean:
                    case tNumeric:
                    {
                        *dabl1 = 0.0;
                    }
                    break;

                    case tString:   // pro retezce delka retezce pomoci strlen
                    {
                        *dabl1 = strlen ( result->data );
                    }
                    break;

                    default:
                    {
                        free ( dabl1 );
                        SetError ( E_INT_TYPE, "interpret.c: LEN: Incompatible types!" );
                    }
                    break;
                }

                TStructToken res;
                res.name = AllocateStrDynamic ( "@result" );
                res.type = tNumeric;
                res.data = dabl1;
                htInsertDynamic ( ActDynamicTab, res.name , res );
            }
            break;

            // RESULT NULL NULL
            case I_SORT:    // instrukce razeni
            {
                if ( result->type == tString )
                {
                    int len = strlen ( result->data );

                    TStructToken Result = GenerateStringTokenDynamic ( AllocateStrDynamic ( "@result" ), tString, result->data );

                    char* str1 = Result.data;
                    StrQuickSort ( str1, 0, len - 1 );  // volani funkce pro razeni textu

                    htInsertDynamic ( ActDynamicTab, Result.name , Result );
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: SORT: Incompatible types!" );
                }

            }
            break;

            // RESULT NULL NULL
            case I_FIND:    // instrukce hledani podstringu v stringu
            {
                if ( ( result->type == tString ) && ( operand1->type == tString ) )
                {
                    double tmp = KMP ( result->data, operand1->data );
                    TStructToken Result = GenerateDoubleTokenDynamic ( AllocateStrDynamic ( "@result" ), tNumeric, tmp );
                    htInsertDynamic ( ActDynamicTab, Result.name , Result );
                }

                else
                {
                    SetError ( E_INT_TYPE, "interpret.c: FIND: Incompatible types!" );
                }
            }
            break;

            // STRING FROM TO
            case I_SUBSTR:  // instrukce podretezce
            {
                if ( ( result->type == tString ) && ( operand1->type == tNumeric ) && ( operand2->type == tNumeric ) )
                {
                    str1  = result->data;
                    dabl1 = operand1->data;
                    dabl2 = operand2->data;

                    int od_cela, do_cela;
                    int delka_textu = strlen ( str1 );

                    if ( isnan ( *dabl1 ) ) // neni zadano od
                    {
                        od_cela = 0.0;  // nastaveni na pocatek
                    }

                    else
                    {
                        od_cela = trunc ( *dabl1 );
                    }

                    if ( isnan ( *dabl2 ) ) // neni zadano do
                    {
                        do_cela = delka_textu;  // nastaveni na konec textu
                    }

                    else
                    {
                        do_cela = trunc ( *dabl2 ); // odseknuti desetinne casti
                    }

                    // naalokovani mista pro vysledny podstring
                    char* final = ( char* ) malloc ( ( delka_textu + 1 ) * sizeof ( char ) );

                    if ( ( od_cela > delka_textu ) ||
                            ( do_cela > delka_textu ) ||
                            ( od_cela < 0.0 ) ||
                            ( do_cela < 0.0 ) ||
                            ( od_cela >= do_cela ) )
                    {
                        final[0] = '\0';
                    }

                    else
                    {
                        int x = od_cela;
                        int i = 0;

                        // samotne prirazovani podstringu
                        while ( x != do_cela )
                        {
                            final[i] = str1[x];
                            x++;
                            i++;
                        }

                        final[i] = '\0';
                    }

                    TStructToken res;
                    res.name = AllocateStrDynamic ( "@result" );
                    res.type = tString;
                    res.data = final;

                    htInsertDynamic ( ActDynamicTab, res.name , res );
                }

                else
                {
                    SetError ( E_SEM_OTHER, "interpret.c: SUBSTR: Incompatible types!" );
                }
            }
            break;

            default:
            {
                SetError ( E_ERR_OTHER, "interpret.c: UNKNOWN INSTRUCTION!" );
                break;
            }
        }   //switch end

        listNext ( instrList );

    } // while end

} //inter end