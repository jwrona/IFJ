/*
* File:                 stack.c
* Description:          Implementation of ADT Stack.
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
* Stack initialization
*/
void ST_Init ( TStack* Stack )
{
    Stack->Top = NULL;
}

/*
* Push item on top of stack (allocate memory for it)
*/
void ST_Push ( TStack* Stack, tStackData data )
{
    TStackItem* tmp = NULL;
    tmp = ( TStackItem* ) malloc ( sizeof ( TStackItem ) );

    if ( tmp == NULL )
    {
        SetError ( E_ERR_OTHER, "stack.c - ST_Push: malloc error!" );
    }

    tmp->data = data;
    tmp->next = Stack->Top;
    Stack->Top = tmp;
}

/*
* Pop top item from stack, return it, free allocated memory
*/
tStackData ST_Pop ( TStack* Stack )
{
    TStackItem* tmp = NULL;
    tStackData data;

    if ( Stack->Top != NULL )
    {
        data = Stack->Top->data;
        tmp = Stack->Top;
        Stack->Top = Stack->Top->next;

        // free the data part
        htClearAllDynamic ( tmp->data.table );
        free ( tmp->data.table );

        free ( tmp );
        tmp = NULL;

        return data;
    }

    else
    {
        SetError ( E_ERR_OTHER, "stack.c - ST_Pop: nothing to pop!" );
        return data;
    }
}

/*
* Delete stack (free memory)
*/
void ST_Dispose ( TStack* Stack )
{
    while ( Stack->Top != NULL )
    {
        ST_Pop ( Stack );
    }
}
