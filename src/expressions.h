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

static void PushStack ( int i , TStructToken New, TStructToken** Stack );
static void PushDollar ( int i, TStructToken** Stack );
static void PushMark ( int i, TStructToken* *Stack );

static TStructToken PopStack ( int* i, TStructToken** Stack );
static int TerminalStack ( int i, TStructToken* Stack );
static int GetIndex ( TToken type );

TStructToken* expression();


