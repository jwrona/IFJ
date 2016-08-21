/*
* ilist.c castecne prevzat z jednoducheho interpretu uverejneneho na strankach predmetu
* File:                 ilist.c
* Description:          Instrukcni paska (linearni seznam)
*
* Projekt:              IFJ-2012
* Team 34:
*                       Duban Michal    -   xduban01
*                       Hanselka Vaclav -   xhanse00
*                       Heczkova Petra  -   xheczk04
*                       Fabry Marko     -   xfabry01
*                       Wrona Jan       -   xwrona00
*/

void listInit ( tListOfInstr* L )
// funkce inicializuje seznam instrukci
{
    L->first  = NULL;
    L->active = NULL;
}

void listFree ( tListOfInstr* L )
// funkce dealokuje seznam instrukci
{
    //fprintf(stderr, "------------ FREE INSTRUCTION LIST ------------\n");
    tListItem Puk;

    while ( L->first != NULL )
    {
        Puk = L->first;
        L->first = L->first->ptr;

        //fprintf(stderr, "instr: %p type: %s\n", Puk, INSTSIGN[Puk->Instruction.instType]);
        free ( Puk );

        Puk = NULL;
    }

    L->active = NULL;
}

void listInsertFirst ( tListOfInstr* L, tInstr I )
// vlozi novou instrukci na zacatek seznamu
{
    tListItem newItem;

    if ( ( newItem = malloc ( sizeof ( struct listItem ) ) ) == NULL )
    {
        SetError ( E_ERR_OTHER, "ilist.c: chyba mallocu!" );
    }

    else
    {
        newItem->ptr = L->first;
        newItem->Instruction = I;
        L->first = newItem;
    }
}

void* listPostInsert ( tListOfInstr* L, tInstr I )
// vlozi novou instrukci za aktivni prvek seznamu
{
    tListItem newItem;

    if ( L->active != NULL )
    {
        if ( ( newItem = malloc ( sizeof ( struct listItem ) ) ) == NULL )
        {
            SetError ( E_ERR_OTHER, "ilist.c: chyba mallocu!" );
            return NULL;
        }

        else
        {
            newItem->Instruction = I;
            newItem->ptr = L->active->ptr;
            L->active->ptr = newItem;
            return newItem;
        }
    }

    else
    {
        SetError ( E_ERR_OTHER, "ilist.c: zadna aktivni instrukce!" );
        return NULL;
    }
}

void listFirst ( tListOfInstr* L )
// zaktivuje prvni instrukci
{
    L->active = L->first;
}

void listNext ( tListOfInstr* L )
// aktivni instrukci se stane nasledujici instrukce
{
    if ( L->active != NULL )
    {
        L->active = L->active->ptr;
    }

    else
    {
        SetError ( E_ERR_OTHER, "ilist.c: zadna aktivni instrukce!" );
    }
}

void listGoto ( tListOfInstr* L, void* gotoInstr )
// nastavime aktivni instrukci podle zadaneho ukazatele
{
    L->active = ( tListItem ) gotoInstr;
}

tInstr* listGetData ( tListOfInstr* L )
// vrati aktivni instrukci
{
    if ( L->active == NULL )
    {
        SetError ( E_ERR_OTHER, "ilist.c: zadna aktivni instrukce!" );
        return NULL;
    }

    else
    {
        return & ( L->active->Instruction );
    }
}



