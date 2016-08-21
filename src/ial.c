/*
* File:                 ial.c
* Description:          Implementation of ADT Hash table.
*                       Quick sort and Knuth-Moris-Pratt algorithm.
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


////////////////////////////////////////////////////////////////////////////////
// Hash Table - author Marko Fabry

/*
* Rozptylova funkce
*/
unsigned long hashCode ( tKey key )
{
    unsigned long retval = 1;
    unsigned long keylen = strlen ( key );

    for ( unsigned long i = 0; i < keylen; i++ )
    {
        retval += key[i];
    }

    return ( retval % HTSIZE );
}

/*
* Inicializace HT
*/
void htInit ( tHTable* ptrht )
{
    if ( ptrht == NULL )
    {
        fprintf ( stderr, "ial.c - htInit : Invalid Table Pointer\n" );
        exit ( 99 );
    }

    for ( int i = 0; i < HTSIZE; i++ )
    {
        ( *ptrht ) [i] = NULL;
    }
}

/*
* Vyhledavani v HT
*/
tHTItem* htSearch ( tHTable* ptrht, tKey key )
{
    if ( ptrht == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTPrintTable ial.c - htSearch : Invalid Table Pointer" );
    }

    tHTItem* tmp;
    unsigned long hash = hashCode ( key );
    tmp = ( *ptrht ) [hash];


    while ( ( tmp != NULL ) && strcmp ( tmp->key, key ) != 0 )
    {
        tmp = tmp->ptrnext;
    }

    return tmp;
}


/*
* Vlozeni do HT (POZOR! funkce je upravena aby vracela data vloženého prvku (zefektivneni))
* Static - funkce neuvolnuje prepsana data (uvolnuji se jinde)
*/
tData* htInsertStatic ( tHTable* ptrht, tKey key, tData data )
{
    if ( ptrht == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTPrintTable ial.c - htInsertStatic : Invalid Table Pointer" );
    }

    if ( key != data.name )
    {
        free ( data.name );
        data.name = key;
    }

    tHTItem* tmp = htSearch ( ptrht, key );

    if ( tmp == NULL )
    {
        tHTItem* tmp1 = NULL;
        unsigned long hash = hashCode ( key );

        tmp = ( tHTItem* ) malloc ( sizeof ( tHTItem ) );

        if ( tmp == NULL )
        {
            SetError ( E_ERR_OTHER, "ial.c: malloc!" );
        }

        tmp1           = ( *ptrht ) [hash];
        ( *ptrht ) [hash] = tmp;

        tmp->key       = key;
        tmp->ptrnext   = tmp1;
        tmp->data      = data;

        return & ( tmp->data );
    }

    else
    {
        tmp->data = data;

        return & ( tmp->data );
    }
}

/*
* Vlozeni do HT (POZOR! funkce je upravena aby vracela data vloženého prvku (zefektivneni))
* Dynamic - funkce uvolnuje i prepisovana data
*/
tData* htInsertDynamic ( tHTable* ptrht, tKey key, tData data )
{
    if ( ptrht == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTPrintTable ial.c - htInsertDynamic : Invalid Table Pointer" );
    }

    if ( key != data.name )
    {
        free ( data.name );
        data.name = key;
    }

    tHTItem* tmp = htSearch ( ptrht, key );

    if ( tmp == NULL )
    {
        // neni v tabulke, pridaj na zaciatok
        tHTItem* tmp1 = NULL;
        unsigned long hash = hashCode ( key );

        tmp = ( tHTItem* ) malloc ( sizeof ( tHTItem ) );

        if ( tmp == NULL )
        {
            SetError ( E_ERR_OTHER, "ial.c: malloc!" );
        }

        tmp1           = ( *ptrht ) [hash];
        ( *ptrht ) [hash] = tmp;

        tmp->key       = key;
        tmp->ptrnext   = tmp1;
        tmp->data      = data;

        return & ( tmp->data );
    }

    else
    {
        // je v tabulke, aktualizuj
        if ( tmp->key != key )
        {
            free ( tmp->key );
            tmp->key = key;
        }

        if ( tmp->data.data != data.data )
        {
            free ( tmp->data.data );
            tmp->data.data = data.data;
        }

        tmp->data.name = data.name;
        tmp->data.type = data.type;

        return & ( tmp->data );
    }
}

/*
* Funkce vraci hodnotu dat, urcenych klicem, v pripade nenalezeni vraci NULL
*/
tData* htRead ( tHTable* ptrht, tKey key )
{
    if ( ptrht == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTPrintTable ial.c - htRead : Invalid Table Pointer" );
    }

    tHTItem* tmp  = NULL;
    tmp = htSearch ( ptrht, key );

    if ( tmp != NULL )
    {
        return & ( tmp->data );
    }

    else
    {
        return NULL;
    }
}

/*
* Funkce smaze prvek urceny klicem
*/
void htDelete ( tHTable* ptrht, tKey key )
{
    if ( ptrht == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTPrintTable ial.c - htDelete : Invalid Table Pointer" );
    }

    tHTItem* tmp_prev = NULL;       // predchodca mazaneho prvku
    tHTItem* tmp      = NULL;       // mazany prvok

    unsigned long hash = hashCode ( key );

    tmp = ( *ptrht ) [hash];

    while ( ( tmp != NULL ) && ( strcmp ( tmp->key, key ) != 0 ) )
    {
        tmp_prev = tmp;
        tmp = tmp->ptrnext;
    }

    if ( tmp == NULL )
    {
        // kde nic neni, ani...
        return;
    }

    // tmp tu je, ideme ho zmazat
    if ( tmp_prev == NULL )
    {
        // mazany je uplne prvy, nema predchodcu
        // tabulka dostane odkaz na naslednika mazaneho prvku
        ( *ptrht ) [hash] = tmp->ptrnext;
    }

    else
    {
        // predchodca dostane odkaz na naslednika mazaneho prvku
        tmp_prev->ptrnext = tmp->ptrnext;
    }

    // a maz!
    free ( tmp );
}


/*
* Zruseni struktury HT a uvolneni alokovane pameti
* Static - funkce neuvolnuje vlastni data (uvolnuji se jinde)
*/
void htClearAllStatic ( tHTable* ptrht )
{
    if ( ptrht == NULL )
    {
        fprintf ( stderr, "ial.c - htClearAllStatic : Invalid Table Pointer\n" );
        exit ( 99 );
    }

    tHTItem* act = NULL;
    tHTItem* tmp = NULL;

    //fprintf(stderr, "----------- FREE STATIC HASH TABLE -----------\n");

    for ( unsigned i = 0; i < HTSIZE; i++ )
    {
        act = ( *ptrht ) [i];

        while ( act != NULL )
        {
            // fprintf(stderr, "token: %10p  type: %14s  data: %10p name: %10p\n",
            //         act, TOKENSIGN[act->data.type], act->data.data, act->key);

            tmp = act;
            act = act->ptrnext;

            free ( tmp );
        }

        ( *ptrht ) [i] = NULL;
        act = NULL;
        tmp = NULL;
    }

}

/*
* Zruseni struktury HT a uvolneni alokovane pameti
* Dynamic - funkce uvolnuje i vlastni data
*/
void htClearAllDynamic ( tHTable* ptrht )
{
    if ( ptrht == NULL )
    {
        fprintf ( stderr, "ial.c - htClearAllDynamic : Invalid Table Pointer\n" );
        exit ( 99 );
    }

    tHTItem* act = NULL;
    tHTItem* tmp = NULL;

    //fprintf(stderr, "----------- FREE DYNAMIC HASH TABLE -----------\n");

    for ( unsigned i = 0; i < HTSIZE; i++ )
    {
        act = ( *ptrht ) [i];

        while ( act != NULL )
        {
            // fprintf ( stderr, "token: %10p  type: %14s  data: %10p name: %10p\n",
            //           act, TOKENSIGN[act->data.type], act->data.data, act->key );

            tmp = act;
            act = act->ptrnext;

            free ( tmp->key );
            tmp->key = NULL;
            free ( tmp->data.data );
            tmp->data.data = NULL;

            free ( tmp );
        }

        ( *ptrht ) [i] = NULL;
        act = NULL;
        tmp = NULL;
    }

}

/*
* Kopirovani tabulky
*/
tHTable* htCopyTable ( tHTable* dest, tHTable* source )
{
    if ( source == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTPrintTable ial.c - htCopyTable : Invalid Table Pointer (source)" );
    }

    if ( dest == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTPrintTable ial.c - htCopyTable : Invalid Table Pointer (dest)" );
    }

    TStructToken PtrToken;
    PtrToken.name = NULL;
    PtrToken.type = tError;
    PtrToken.data = NULL;

    for ( int i = 0; i < HTSIZE; i++ )
    {

        tHTItem* ptr    = ( *source ) [i];

        while ( ptr != NULL )
        {

            PtrToken.name = malloc ( strlen ( ptr->key ) * sizeof ( char ) + 1 );

            if ( PtrToken.name == NULL )

            {
                SetError ( E_ERR_OTHER, "ial.c: htCopyTable Malloc" );
            }

            PtrToken.name = strcpy ( PtrToken.name, ptr->key );


            PtrToken.type = ptr->data.type;

            switch ( PtrToken.type )
            {
                case tBoolean:
                case tNumeric:
                case tNil:
                {
                    PtrToken.data = malloc ( sizeof ( double ) );

                    if ( PtrToken.data == NULL )
                    {
                        SetError ( E_ERR_OTHER, "ial.c: htCopyTable Malloc" );
                    }

                    if ( ptr->data.data != NULL )
                    {
                        PtrToken.data = memcpy ( PtrToken.data, ptr->data.data, sizeof ( double ) );
                    }
                }
                break;

                case tString:
                {

                    PtrToken.data = malloc ( strlen ( ptr->data.data ) * sizeof ( char ) + 1 );

                    if ( PtrToken.data == NULL )
                    {
                        SetError ( E_ERR_OTHER, "ial.c: htCopyTable Malloc" );
                    }

                    PtrToken.data = strcpy ( PtrToken.data, ptr->data.data );
                }
                break;

                case tLab:
                case tError:
                case tIdent:
                case tFunction:
                default:
                {
                    PtrToken.data = NULL;
                }
                break;
            }

            htInsertStatic ( dest, PtrToken.name, PtrToken );
            ptr = ptr->ptrnext;
        }
    }

    return dest;
}

// Hash Table - END
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
// Quick Sort - author Vaclav Hanselka


// Swapne chary na pozicich a,b v stringu string
void Swap ( char* string, int a, int b )
{
    char tmp;
    tmp = string[a];
    string[a] = string[b];
    string[b] = tmp;
}

// Sortne string vzostupne
void StrQuickSort ( char* string , int a, int b )
{

    char pivot = string[ ( a + b ) / 2];
    int i = a, j = b;

    while ( i <= j )
    {
        while ( string[i] < pivot )
        {
            i++;
        }

        while ( string[j] > pivot )
        {
            j--;
        }

        if ( i <= j )
        {
            Swap ( string, i, j );
            i++; j--;
        }
    }

    if ( a < j )
    {
        StrQuickSort ( string, a, j );
    }

    if ( i < b )
    {
        StrQuickSort ( string, i, b );
    }
}

// Quick Sort - END
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Knuth-Morris-Pratt algorithm - author Vaclav Hanselka


double KMP ( char* retezec, char* hledany )
{
    unsigned long len_R = 0;
    unsigned long len_H = 0;
    unsigned long R = 0;
    unsigned long H = 0;

    len_R = strlen ( retezec );
    len_H = strlen ( hledany );

    if ( len_H == 0 )
    {
        return 0.0;
    }

    bool nasel = false;

    // printf("delky: %d %d\n",len_R,len_H);

    while ( R <= len_R && nasel == false )
    {
        if ( retezec[R] == hledany[H] )
        {
            H++;
        }

        else
        {
            if ( H )
            {
                R--;
            }

            H = 0;
        }

        if ( H == len_H )
        {
            nasel = true;
            //printf("nasel!!!\n");
        }

        R++;
    }

    if ( nasel )
    {
        return ( double ) ( R - len_H );
    }

    else
    {
        return -1.0;
    }
}
// Knuth-Morris-Pratt algorithm - END
////////////////////////////////////////////////////////////////////////////////
