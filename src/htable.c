/*
* File:                 htable.c
* Description:          Implementation of ADT Hash table.
*
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
* Rozptylova funkce
*/
unsigned long FuncHThashCode ( tKey key )
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
void FuncHTInit ( tFuncHTable* ptrht )
{
    if ( ptrht == NULL )
    {
        fprintf ( stderr, "neplatny ukazatel na tabulku\n" );
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
tFuncHTItem* FuncHTSearch ( tFuncHTable* ptrht, tFuncKey key )
{
    if ( ptrht == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTSearch neplatny ukazatel na tabulku" );
    }

    tFuncHTItem* tmp;
    int hash = FuncHThashCode ( key );
    tmp = ( *ptrht ) [hash];


    while ( ( tmp != NULL ) && strcmp ( tmp->key, key ) != 0 )
    {
        tmp = tmp->ptrnext;
    }

    return tmp;
}

/*
* Vlozeni do HT ( funkce neuvolnuje prepsana data (uvolnuji se jinde) )
*/
tFuncData* FuncHTInsert ( tFuncHTable* ptrht, tFuncKey key, tFuncData data )
{
    if ( ptrht == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTInsert neplatny ukazatel na tabulku" );
    }

    tFuncHTItem* tmp  = NULL;
    tmp = FuncHTSearch ( ptrht, key );

    if ( tmp == NULL )
    {
        // neni v tabulke, pridaj na zaciatok
        tFuncHTItem* tmp1 = NULL;
        int hash = FuncHThashCode ( key );

        tmp = ( tFuncHTItem* ) malloc ( sizeof ( tFuncHTItem ) );

        if ( tmp == NULL )
        {
            exit ( 99 );
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

        tmp->data = data;
        return & ( tmp->data );
    }
}

/*
* Funkce vraci hodnotu dat, urcenych klicem, v pripade nenalezeni vraci NULL
*/
tFuncData* FuncHTRead ( tFuncHTable* ptrht, tFuncKey key )
{
    if ( ptrht == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTRead neplatny ukazatel na tabulku" );
    }

    tFuncHTItem* tmp  = NULL;
    tmp = FuncHTSearch ( ptrht, key );

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
void FuncHTDelete ( tFuncHTable* ptrht, tFuncKey key )
{
    if ( ptrht == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTRead neplatny ukazatel na tabulku" );
    }

    tFuncHTItem* tmp_prev = NULL;       // predchodca mazaneho prvku
    tFuncHTItem* tmp      = NULL;       // mazany prvok

    int hash = FuncHThashCode ( key );

    tmp = ( *ptrht ) [hash];

    while ( ( tmp != NULL ) && ( tmp->key != key ) )
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
*/
void FuncHTClearAll ( tFuncHTable* ptrht )
{
    if ( ptrht == NULL )
    {
        fprintf ( stderr, "neplatny ukazatel na tabulku\n" );
        exit ( 99 );
    }

    tFuncHTItem* act = NULL;
    tFuncHTItem* tmp = NULL;

    for ( int i = 0; i < HTSIZE; i++ )
    {
        act = ( *ptrht ) [i];

        while ( act != NULL )
        {
            tmp = act;
            act = act->ptrnext;

            free ( tmp->data.fargv );

            if ( tmp->data.ftable != NULL )
            {
                htClearAllStatic ( tmp->data.ftable );
                free ( tmp->data.ftable );
            }

            free ( tmp );

        }

        ( *ptrht ) [i] = NULL;
    }
}

/*
* Funkce kontroluje zda byli vsechny funkce definovane,
* a zda v se v jejich definicich neopakují argumenty.
*/
void FuncHTCheck ( tFuncHTable* ptrht )
{
    if ( ptrht == NULL )
    {
        SetError ( E_ERR_OTHER, "FuncHTPrintTable neplatny ukazatel na tabulku" );
    }

    for ( int i = 0; i < HTSIZE; i++ )
    {
        tFuncHTItem* ptr = ( *ptrht ) [i];

        while ( ptr != NULL )
        {
            if ( !ptr->data.valid )
            {
                SetError ( E_SEM_UNDEF_FUNC, "err: NEDEFINOVANA FUNKCE!" );
            }

            for ( unsigned long q = 1; q < ptr->data.fargc; q++ )
            {
                if ( ( FuncHTSearch ( ptrht, ptr->data.fargv[q].name ) ) != NULL )
                {
                    SetError ( E_SEM_OTHER, "Argument cannot have name like defined function" );
                }
            }

            ptr = ptr->ptrnext;
        }
    }
}

