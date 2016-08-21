/**
 * File:                 expressions.c
 * Description:          modul pracujici s vyrazy, volan z parseru
 * Author:               Heczkova Petra  -   xheczk04
 *
 * Projekt:              IFJ-2012
 * Team 34:
 *                       Duban Michal    -   xduban01
 *                       Hanselka Vaclav -   xhanse00
 *                       Heczkova Petra  -   xheczk04
 *                       Fabry Marko     -   xfabry01
 *                       Wrona Jan       -   xwrona00
 */


// velikost precedencni tabulky
#define MaxTab 12

// velikost zasobniku
#define MaxStack 16

// konstanta pri praci s indexy tokenu, s 0-5 pracuje parser a
// ve vyrazu nejsou povolene, napr tWhile
#define ExpTokenIndex 6

// konstanta pri nahlizeni do zasobniku
#define BackConst 2

// magicka konstanta pro stesti
#define MagicNumer 42

// zasobnik
TStructToken* Stack = NULL;

/**
 * Vycet moznych hodnot v precedencni tabulce
 */
typedef enum TTable
{
    tG,   // <
    tL,   // >
    tE,   // =
    tN,   // nedefinovane -> error
    tEND, // Eol, v tabluce $ -> spravne ukonceni
    tS,   // operace stringu id [ n : n ]
    tF,   // funkce id ( )
} TTable;


/**
 * Precencni tabulka, v radcich jsou indexy terminalu ulozenych nejblize
 * vrcholu zasobniku, v sloupcich indexy aktualniho tokenu.
 * tF detekujeme funkci, indexy [5][3]
 * tS detekujeme praci se stringem, indexy [10][12]
 * tEND spravne ukonceni vyrazu, [12][12]
 */
static TTable PrecTable[MaxTab][MaxTab] =
{
    // znak == zastupuje == a !=
    // znak <> zastupuje <, >, <= a >=

    //  0   1   2   3   4   5   6   7   8   9  10  11
    // +-  */  **   (   )   i  ==  <>   [   ]   :   $
    {  tG, tL, tL, tL, tG, tL, tG, tG, tL, tG, tN, tG   }, // 0  +-
    {  tG, tG, tL, tL, tG, tL, tG, tG, tL, tG, tN, tG   }, // 1  */
    {  tG, tG, tG, tL, tG, tL, tG, tG, tL, tG, tN, tG   }, // 2  **
    {  tL, tL, tL, tL, tE, tL, tL, tL, tN, tN, tN, tN   }, // 3  (
    {  tG, tG, tG, tN, tG, tN, tG, tG, tN, tN, tN, tG   }, // 4  )
    {  tG, tG, tG, tF, tG, tN, tG, tG, tG, tG, tG, tG   }, // 5  i
    {  tL, tL, tL, tL, tG, tL, tG, tL, tL, tG, tN, tG   }, // 6  ==
    {  tL, tL, tL, tL, tG, tL, tG, tG, tL, tG, tN, tG   }, // 7  <>
    {  tL, tL, tL, tN, tN, tL, tL, tL, tL, tG, tE, tN   }, // 8  [
    {  tG, tG, tG, tN, tN, tN, tG, tG, tN, tG, tN, tS   }, // 9  ]
    {  tN, tN, tN, tN, tN, tL, tN, tN, tN, tE, tN, tN   }, // 10 :
    {  tL, tL, tL, tL, tN, tL, tL, tL, tL, tG, tN, tEND }, // 11 $
};

int StackSize = MaxStack;  // velikost zasobniku, pri realokaci se zdvojnasobi

/**
 * Vkladani na zasobnik
 * @i oznacuje misto kam se vlozi novy token @New
 */
static void PushStack ( int i, TStructToken New, TStructToken** Stack )
{
    TStructToken* temp = NULL;

    if ( i % StackSize == 0 )
    {
        //StackSize <<= 1; // bitovy posun doleva => StackSize * 2

        temp = realloc ( *Stack, sizeof ( TStructToken ) * ( StackSize + 1 + i ) );

        if ( temp == NULL )
        {
            // free ( *Stack );
            SetError ( E_ERR_OTHER, "exp: Realloc error!" );
        }

        *Stack = temp;

    }

    ( *Stack ) [i] = New;
    return;
}

/**
 * Vkladani znaku $ na zasobnik
 * zavola funkci PushStack
 * @i oznacuje misto kam se vlozi novy token s $
 */
static void PushDollar ( int i, TStructToken** Stack )
{
    TStructToken Dollar;
    Dollar.type = tEol;
    Dollar.data = NULL;
    PushStack ( i , Dollar, Stack );
    return;
}

/**
 * Vkladani znaku < na zasobnik
 * zavola funkci PushStack, znak < se vklada jako pomocny znak,
 * urcuje se podle nej, ktere pravidlo se ma pouzit,
 * vola se ve funkci expression()
 * @i oznacuje misto kam se vlozi novy token s $
 */
static void PushMark ( int i, TStructToken** Stack )
{
    TStructToken Mark;
    Mark.type = tMark ;
    Mark.data = NULL;
    PushStack ( i , Mark, Stack );
    return;
}

/**
 * Vybrani ze zasobniku
 * @i oznacuje vrchol zasobniku (tzn. volne misto, kam by se zapisovalo),
 * musime @i dekrementovat, abychom ziskali misto odkud se token vybira,
 * nahradime ho tokenem typu tError, vznika zde novy vrchol zasobniku
 */
static TStructToken PopStack ( int* i, TStructToken** Stack )
{
    TStructToken Error;

    if ( -- ( *i ) > -1 )
    {
        Error.type = tError;
        Error.data = 0;

        TStructToken New = ( *Stack ) [*i];
        ( *Stack ) [*i] = Error;
        return New;
    }

    if ( *Stack != NULL )
    {
        // free ( *Stack );
        SetError ( E_SYN, "exp: Syntax Error (pop)!" );
    }

    return Error;
}

/**
 * Vraci terminal na zasobniku nejblize vrcholu
 * @i oznacuje vrchol zasobniku
 * posouvame se polem od konce (dekrementujeme j) a hledame nejblizsi terminal
 * (tzn. znamenko, Eol, konstantu, promennou, ale ne uz zpracovany vyraz)
 */
static int TerminalStack ( int i, TStructToken* Stack )
{
    int j = i - 1;

    while ( j >= 0 )
    {
        if ( Stack[j].type < tIdent || Stack[j].type > tColon )
        {
            j--;
        }

        else
        {
            return j;
        }
    }

    // chyba by nemela nastat, zasobnik obsahuje vzdy alespon $ na zacatku
    if ( Stack != NULL )
    {
        // free ( Stack );
        SetError ( E_ERR_OTHER, "exp: TerminalStack error!" );
    }

    return 0;
}

/**
 * Zjisti index daneho tokenu, ktery se dale pouzije v precedencni tabulce,
 * zavisla na poradi tokenu v ifj12.h
 * ExpTokenIndex je 6, s tokeny 0-5 pracuje parser
 * napr. tNumeric -> [20-6]=[14], na 14. miste v IndexTable je cislo 5
 * v precedencni tabulce bude mit index 5
 */
static int GetIndex ( TToken type )
{
    // pokud se zmeni poradi tokenu zmenit!
    int IndexTable[] =
    {
        5,                  // identifikator
        0, 1, 0, 1, 2,      //  +  *  -  /  **
        6, 6, 7, 7, 7, 7,   // ==  !=  >  <  >=  <=
        3, 4,               // (  )
        5, 5, 5, 5,         // double string bool nil //true false
        11,                 // Eol  //Eof
        8, 9, 10,            // [  ]  ,  :
    };

    return  IndexTable[type - ExpTokenIndex];
}

/**
 * Vraci pomocny znak < na zasobniku nejblize vrcholu
 * funguje podobne jako funkce TerminalStack
 */
static int FindMark ( int i, TStructToken* Stack )
{
    int j = i - 1;

    while ( j >= 0 )
    {
        switch ( Stack[j].type )
        {
            case tMark :
                return j;

            default:
                j--;
        }
    }

    // if ( Stack != NULL )
    // {
    //     free ( Stack );
    // }

    SetError ( E_SYN, "exp: Syntax Error (no mark)!" );
    return 0;

}


TStructToken* expression()
{
    int i = 0;

    int TI;             // TI - index terminalu na zasobniku
    int AI = 99;        // AI - index tokenu, ktery se ma na zasobnik vlozit
    int ExpLen = 0;     // ExpLen - delka vyrazu

    // pomocne tokeny a ukazatele
    TStructToken Act = token;
    TStructToken Term;

    TStructToken Dest;
    TStructToken Source1;
    TStructToken Source2;

    TStructToken* PtrDest    = NULL;
    TStructToken* PtrSource1 = NULL;
    TStructToken* PtrSource2 = NULL;

    TStructToken Oper;
    TStructToken Temp;

    Source1.name = NULL;
    Source1.type = tWhile;
    Source1.data = NULL;

    Source2 = Source1;
    Dest    = Source1;
    Temp = Source1;

    // pomocna promenna, pokud true tak pracujeme s tokenem Act,
    // ktery jeste nebyl vlozen do zasobniku,
    // nesmi se zavolat funkce GetNextToken
    bool ActPushed = true;

    if ( Act.type == tEol )
    {
        SetError ( E_SYN, "exp: Syntax Error - empty expression!" );
    }

    // zjistime index aktualniho tokenu
    AI = GetIndex ( Act.type );

    // alokace zasobniku
    Stack = ( TStructToken* ) malloc ( sizeof ( TStructToken ) * MaxStack );

    if ( Stack == NULL )
    {
        SetError ( E_ERR_OTHER, "exp: Malloc error!" );
    }

    //vlozeni dolaru
    PushDollar ( i++, &Stack );

    // hlavni cyklus zpracovani vyrazu
    do
    {
        // zjistime jaky je terminal nejblize vrcholu a jeho index
        Term = Stack[TerminalStack ( i, Stack )];
        TI = GetIndex ( Term.type );

        // zjistime jaky je aktualni token a jeho index
        if ( ActPushed == false )
        {
            token = GetNextToken();
            Act = token;
            //AI  = GetIndex ( Act.type );

        }

        //fprintf(stderr, "exp: ACT %s %d\n",TOKENSIGN[Act.type],AI);

        // pokud dostaneme index, ktery nepatri do vyrazu
        if ( Act.type < tIdent || Act.type > tColon )
        {
            // if ( Stack != NULL )
            // {
            //     free ( Stack );
            // }

            SetError ( E_SYN, "exp:  Wrong token!" );
        }

        AI  = GetIndex ( Act.type );

        // rozhodujeme podle indexu v precedencni tabulce
        switch ( PrecTable[TI][AI] )
        {
            case tF:
            {
                //fprintf(stderr, "exp:  id(E) -> E\n");
                Dest = PopStack ( &i, &Stack ); // identifikator funkce
                PopStack ( &i, &Stack );      // pomocny znak <
                Source1 = PopStack ( &i, &Stack );      // pomocny znak <

                free ( Stack );
                Stack = NULL;

                if ( Dest.type != tIdent )
                {
                    SetError ( E_SYN, "Calling for something that is not function" );
                }

                if ( Source1.type != tEol )
                {
                    SetError ( E_SYN, "Function call in expression" );
                }

                // odkazujeme se dale do parseru
                PtrDest = handleFunctionCall ( Dest );
                return PtrDest;
            }
            break;

            case tS:
            {
                // fprintf(stderr, "exp:  id[n:n] -> E\n");
                // id[n:n] || id[n:] || id[:n] || id[:]
                PopStack ( &i, &Stack ); // prava ]
                Source2 = PopStack ( &i, &Stack );

                // ocekavame ze Source2 bude Numeric nebo " : "
                if ( Source2.type == tColon )
                {
                    // je to " : "
                    // chybi 2. parametr, nahradime ho
                    Source2 = GenerateDoubleToken ( GenerateTmpVarName(), tNumeric, 0.0 / 0.0 );
                    PtrSource2 = htInsertStatic ( ActStaticTab, Source2.name, Source2 );
                }

                else if ( Source2.type == tExpress )
                {
                    // je to cislo
                    PopStack ( &i, &Stack ); // vybereme jeste i znak " : "
                }

                else
                {
                    // je to neco jineho
                    // free ( Stack );
                    SetError ( E_SYN, "exp: Syntax String Error!" );
                }

                // ocekavame ze Source1 bude Numeric nebo " [ "
                Source1 = PopStack ( &i, &Stack );

                if ( Source1.type == tL_Sq_Br )
                {
                    // je to " [ "
                    // chybi 1. parametr, nahradime ho
                    Source1 = GenerateDoubleToken ( GenerateTmpVarName(), tNumeric, 0.0 / 0.0 );
                    PtrSource1 = htInsertStatic ( ActStaticTab, Source1.name, Source1 );
                }

                else if ( Source1.type == tExpress )
                {
                    // je to cislo
                    PopStack ( &i, &Stack ); // vybereme jeste i znak " [ "
                }

                else
                {
                    // je to neco jineho
                    // free ( Stack );
                    SetError ( E_SYN, "exp: Syntax String Error!" );
                }

                PopStack ( &i, &Stack ); // pomocny znak <
                Dest = PopStack ( &i, &Stack ); // identifikator funkce

                if ( Dest.type != tExpress )
                {
                    // je to neco jineho
                    // free ( Stack );
                    SetError ( E_SYN, "exp: Syntax String Error!" );
                }


                PtrSource1 = htRead ( ActStaticTab, Source1.name );
                PtrSource2 = htRead ( ActStaticTab, Source2.name );
                PtrDest = htRead ( ActStaticTab, Dest.name );

                if ( ( PtrSource1->type != tNumeric ) && ( PtrSource1->type != tIdent ) && ( PtrDest->type != tNil ) )
                {
                    // free ( Stack );
                    SetError ( E_SEM_OTHER, "exp: Semantic String Error (Source1)!" );
                }

                if ( ( PtrSource2->type != tNumeric ) && ( PtrSource2->type != tIdent ) && ( PtrDest->type != tNil ) )
                {
                    // free ( Stack );
                    SetError ( E_SEM_OTHER, "exp: Semantic String Error (Source2)!" );
                }

                if ( ( PtrDest->type != tString ) && ( PtrDest->type != tIdent ) && ( PtrDest->type != tNil ) )
                {
                    // free ( Stack );
                    SetError ( E_SEM_OTHER, "exp: Semantic String Error (Dest)!" );
                }


                I = MakeInstruction ( I_SUBSTR, PtrDest, PtrSource1, PtrSource2 );
                listPostInsert ( &L, I );
                listNext ( &L );

                if ( Stack != NULL )
                {
                    free ( Stack );
                    Stack = NULL;
                }

                TStructToken  Result    = GenerateDoubleToken ( AllocateStr ( "@result" ), tNil, 0.0 );
                PtrDest = htInsertStatic ( ActStaticTab, Result.name, Result );

                return PtrDest;

            }
            break;

            case tE:
            {
                PushStack ( i++, Act, &Stack );
                ActPushed = false;
            }
            break;

            case tL:
            {
                // podobne jako pri tE, navic pridame pomocny znak <
                PushMark ( i++, &Stack );
                PushStack ( i++, Act, &Stack );
                ActPushed = false;
            }
            break;

            case tG:
            {
                // ExpLen vyjadruje delku zpracovavaneho vyrazu
                ExpLen  = i - FindMark ( i, Stack );

                Source2 = PopStack ( &i, &Stack );
                //printf("L %d\n", ExpLen);

                switch ( ExpLen )
                {
                        // obsahuje pravidla s nejvyssi prioritou
                        // i -> E
                    case 2:
                    {
                        if ( Source2.type != tIdent && ! ( tNumeric <= Source2.type && Source2.type <= tNil ) )
                        {
                            // free ( Stack );
                            SetError ( E_SYN, "exp: Syntax Error (wrong Source2)!" );
                        }

                        Dest = Source2;

                        PtrSource2 = InsertTerm ( Source2 );
                        PtrDest = PtrSource2;

                        //fprintf ( stderr, "exp:  i -> E\n" );

                        // zde bude vysledek, vlozime ho do zasobniku
                        Dest.name = PtrDest->name;
                        Dest.type = tExpress;

                        PopStack ( &i, &Stack );
                        PushStack ( i++, Dest, &Stack );
                        ActPushed = true;

                    }
                    break;


                    case 3:
                    {
                        // obsahuje pravidla E -> E op E
                        // tzn. aritmericke a relacni
                        Oper = PopStack ( &i, &Stack );  // operator
                        Temp = PopStack ( &i, &Stack );  // pomocny znak <
                        Source1 = PopStack ( &i, &Stack ); // cislo

                        if ( Source1.type != tExpress )
                        {
                            // free ( Stack );
                            SetError ( E_SYN, "exp: Syntax Error (wrong Source1)!" );
                        }

                        if ( Source2.type != tExpress )
                        {
                            // free ( Stack );
                            SetError ( E_SYN, "exp: Syntax Error (wrong Source2)!" );
                        }


                        // vytvorim miesto v statickej tabulke
                        // pre vysledok aritmetickej operacie
                        Dest.name = GenerateTmpVarName();
                        Dest.data = NULL;
                        Dest.type = tIdent;

                        PtrDest    = htInsertStatic ( ActStaticTab, Dest.name, Dest );

                        PtrSource1 = htRead ( ActStaticTab, Source1.name );
                        PtrSource2 = htRead ( ActStaticTab, Source2.name );

                        // vybiram pravidlo podle typu operatoru
                        switch ( Oper.type )
                        {
                            case tAdd:
                            {
                                //E -> E + E
                                I = MakeInstruction ( I_ADD, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            case tSub:
                            {
                                //E -> E - E
                                I = MakeInstruction ( I_SUB, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            case tMul:
                            {
                                //E -> E * E
                                I = MakeInstruction ( I_MUL, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            case tDiv:
                            {
                                //E -> E / E
                                I = MakeInstruction ( I_DIV, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            case tPow:
                            {
                                //E -> E ** E
                                I = MakeInstruction ( I_POW, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            case tEq:
                            {
                                //E -> E == E
                                I = MakeInstruction ( I_EQ, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            case tNEq:
                            {
                                //E -> E != E
                                I = MakeInstruction ( I_NEQ, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            case tGtEq:
                            {
                                //E -> E >= E
                                I = MakeInstruction ( I_GTE, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            case tLtEq:
                            {
                                //E -> E <= E
                                I = MakeInstruction ( I_LTE, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            case tGt:
                            {
                                //E -> E > E
                                I = MakeInstruction ( I_GT, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            case tLt:
                            {
                                //E -> E < E
                                I = MakeInstruction ( I_LT, PtrDest, PtrSource1, PtrSource2 );
                            }
                            break;

                            default:
                            {
                                // if ( Stack != NULL )
                                // {
                                //     free ( Stack );
                                // }

                                SetError ( E_SYN, "exp:  Syntax Error (unknown operator)!" );
                            }
                            break;
                        }

                        //fprintf ( stderr, "exp:  E -> E %s E\n", TOKENSIGN[Oper.type] );

                        // vlozim do seznamu instrukci
                        listPostInsert ( &L, I );
                        listNext ( &L );

                        // vysledek vlozim do zasobniku
                        Dest.type = tExpress;
                        PushStack ( i++, Dest, &Stack );
                        ActPushed = true;
                    }
                    break;

                    case 4:
                    {
                        //fprintf ( stderr, "exp:  E -> (E)\n" );
                        Dest    = PopStack ( &i, &Stack );  // E
                        Source1 = PopStack ( &i, &Stack );  // (
                        PopStack ( &i, &Stack );            // <

                        if ( Source1.type != tL_Br  || Dest.type != tExpress || Source2.type != tR_Br )
                        {
                            //free ( Stack );
                            SetError ( E_SYN, "exp: Syntax Error ()" );
                        }

                        PushStack ( i++, Dest, &Stack ); // vratim bez zavorek

                        // pokud je za zavorkou znamenko pridam pomocny znak <
                        if ( Act.type >= tAdd && Act.type <= tLtEq )
                        {
                            PushMark ( i++, &Stack );
                            PushStack ( i++, Act, &Stack );
                            ActPushed = false;
                        }

                        // pokud za zavorkou je Eol
                        if ( Act.type == tEol )
                        {
                            ActPushed = true;
                        }

                        if ( Act.type == tR_Br )
                        {

                            ActPushed = true;
                        }
                    }
                    break;

                    default:
                    {
                        // if ( Stack != NULL )
                        // {
                        //     free ( Stack );
                        // }

                        SetError ( E_SYN, "exp: Syntax Error (wrong expression lengh)!" );
                    }
                    break;
                }

            }

            break;

            case tEND:
            {
                // vse probehlo v poradku
                if ( Stack != NULL )
                {
                    free ( Stack );
                    Stack = NULL;
                }

                return PtrDest;
            }
            break;

            case tN:
            {
                // if ( Stack != NULL )
                // {
                //     free ( Stack );
                // }

                SetError ( E_SYN, "exp: Syntax Error (undefined in PrecTable)!" );
            }
            break;
        }

        // int Z; // pomocny vypis zasobniku po kazdem kroku
        // fprintf ( stderr, "---------- EXPR STACK ----------\n" );

        // for ( Z = 0; Z < i; Z++ )
        // {
        //     fprintf ( stderr, "stack %2d: %s \n", Z, TOKENSIGN[Stack[Z].type] );
        // }

    }

    while ( MagicNumer );
}

