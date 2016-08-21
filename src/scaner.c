/*
 * File:                 scanner.c
 * Description:          scanner.c
 * Author:               Wrona Jan       -   xwrona00
 *
 * Projekt:              IFJ-2012
 * Team 34:
 *                       Duban Michal    -   xduban01
 *                       Hanselka Vaclav -   xhanse00
 *                       Heczkova Petra  -   xheczk04
 *                       Fabry Marko     -   xfabry01
 *                       Wrona Jan       -   xwrona00
 */
char* tokenstr = NULL;

TStructToken GetNextToken ( void )
{
    TStructToken token;
    token.name = NULL;
    token.type = tError;
    token.data = NULL;

    int symbol = 0;

    int rerun = 0;
    static int returnedEol = 1;

    do
    {
        symbol = fgetc ( f_source );
        rerun = 0;

        while ( isspace ( symbol ) && symbol != '\n' )
        {
            symbol = fgetc ( f_source );
        }

        switch ( symbolClass ( symbol ) )
        {
            case IDENTIFIER:      //recognized idenfier
                append ( &tokenstr, symbol );
                symbol = fgetc ( f_source );

                while ( isalnum ( symbol ) || symbol == '_' )
                {
                    append ( &tokenstr, symbol );
                    symbol = fgetc ( f_source );
                }

                ungetc ( symbol, f_source );

                token.type = identType ( tokenstr );

                if ( token.type == tIdent )
                {
                    token.name = tokenstr;
                }

                else
                {
                    if ( tokenstr != NULL )
                    {
                        free ( tokenstr );
                    }

                    if ( token.type == tNil )
                    {
                        double* tmp = NULL;
                        tmp = ( double* ) malloc ( sizeof ( double ) );

                        if ( tmp == NULL )
                        {
                            SetError ( E_ERR_OTHER, "scan: Malloc Error!!!" );
                        }

                        *tmp = 0.0;
                        token.data = tmp;
                    }

                    else if ( token.type == tFalse )
                    {
                        double* tmp = NULL;
                        tmp = ( double* ) malloc ( sizeof ( double ) );

                        if ( tmp == NULL )
                        {
                            SetError ( E_ERR_OTHER, "scan: Malloc Error!!!" );
                        }

                        *tmp = 0.0;
                        token.data = tmp;
                        token.type = tBoolean;
                    }

                    else if ( token.type == tTrue )
                    {
                        double* tmp = NULL;
                        tmp = ( double* ) malloc ( sizeof ( double ) );

                        if ( tmp == NULL )
                        {
                            SetError ( E_ERR_OTHER, "scan: Malloc Error!!!" );
                        }

                        *tmp = 1.0;
                        token.data = tmp;
                        token.type = tBoolean;
                    }
                }

                tokenstr = NULL;
                break;

            case NUMLIT:      //recognized numerical literal
                append ( &tokenstr, symbol );

                while ( isdigit ( symbol = fgetc ( f_source ) ) ) //cteni cele casti
                {
                    append ( &tokenstr, symbol );
                }

                if ( symbol == '.' )
                {
                    append ( &tokenstr, symbol );

                    if ( isdigit ( symbol = fgetc ( f_source ) ) )
                    {
                        append ( &tokenstr, symbol );

                        while ( isdigit ( symbol = fgetc ( f_source ) ) ) //cteni desetinne casti
                        {
                            append ( &tokenstr, symbol );
                        }

                        if ( symbol != 'e' )
                        {
                            ungetc ( symbol, f_source );

                            double* tmp = NULL;
                            tmp = ( double* ) malloc ( sizeof ( double ) );

                            if ( tmp == NULL )
                            {
                                SetError ( E_ERR_OTHER, "scan: Malloc Error!!!" );
                            }

                            *tmp = strtod ( tokenstr, NULL ); //nejak poresit preteceni

                            token.data = tmp;
                            token.type = tNumeric;

                            if ( tokenstr != NULL )
                            {
                                free ( tokenstr );
                            }

                            tokenstr = NULL;
                            break;
                        }
                    }
                }

                if ( symbol == 'e' ) //exponencialni cast
                {
                    append ( &tokenstr, symbol );
                    symbol = fgetc ( f_source );

                    if ( ( !isdigit ( symbol ) ) && ( symbol != '+' ) && ( symbol != '-' ) )
                    {
                        if ( tokenstr != NULL )
                        {
                            free ( tokenstr );
                            tokenstr = NULL;
                        }

                        SetError ( E_LEX, "Numerical literal error (exponential part)" );
                    }

                    if ( symbol == '+' || symbol == '-' )
                    {
                        append ( &tokenstr, symbol );

                        if ( !isdigit ( symbol = fgetc ( f_source ) ) )
                        {
                            if ( tokenstr != NULL )
                            {
                                free ( tokenstr );
                                tokenstr = NULL;
                            }

                            SetError ( E_LEX, "Numerical literal error (exponential part)" );
                        }
                    }

                    append ( &tokenstr, symbol );

                    while ( isdigit ( symbol = fgetc ( f_source ) ) ) //cteni exponencialni casti
                    {
                        append ( &tokenstr, symbol );
                    }

                    ungetc ( symbol, f_source );

                    double* tmp = NULL;
                    tmp = ( double* ) malloc ( sizeof ( double ) );

                    if ( tmp == NULL )
                    {
                        SetError ( E_ERR_OTHER, "scan: Malloc Error!!!" );
                    }

                    *tmp = strtod ( tokenstr, NULL ); //nejak poresit preteceni

                    token.data = tmp;
                    token.type = tNumeric;

                    if ( tokenstr != NULL )
                    {
                        free ( tokenstr );
                    }

                    tokenstr = NULL;
                    break;
                }

                if ( symbol != '.' && symbol != 'e' )
                {
                    if ( tokenstr != NULL )
                    {
                        free ( tokenstr );
                        tokenstr = NULL;
                    }

                    SetError ( E_LEX, "Numerical literal error . / e" );
                }

                break;

            case STRING:      //recognized string

                if ( ( symbol = fgetc ( f_source ) ) == '"' )
                {
                    append ( &tokenstr, '\0' );
                }

                else
                {
                    while ( symbol != '"' )
                    {
                        if ( symbol == '\\' )
                        {
                            char xEscape[3] = {'\0'};

                            switch ( symbol = fgetc ( f_source ) )
                            {
                                case 'n':
                                    append ( &tokenstr, '\n' );
                                    break;

                                case 't':
                                    append ( &tokenstr, '\t' );
                                    break;

                                case '\\':
                                    append ( &tokenstr, '\\' );
                                    break;

                                case '"':
                                    append ( &tokenstr, '\"' );
                                    break;

                                case 'x':

                                    for ( int i = 0; i < 2; i++ )
                                    {
                                        symbol = fgetc ( f_source );

                                        if ( isxdigit ( symbol ) )
                                        {
                                            xEscape[i] = symbol;
                                        }

                                        else
                                        {
                                            SetError ( E_LEX, "Escape sequence error (\\x)" );
                                            break;
                                        }
                                    }

                                    if ( strcmp ( xEscape, "00" ) == 0 )
                                    {
                                        SetError ( E_LEX, "Escape sequence error (\\x)" );
                                        break;
                                    }

                                    append ( &tokenstr, strtol ( xEscape, NULL, 16 ) );
                                    break;

                                default:
                                    SetError ( E_LEX, "Escape sequence error (unknown esc seq)" );
                                    break;
                            }
                        }

                        else if ( symbol <= 0 || symbol == EOF )
                        {
                            SetError ( E_LEX, "Unterminated string" );
                            ungetc ( symbol, f_source );
                            break;
                        }

                        else
                        {
                            append ( &tokenstr, symbol );
                        }

                        symbol = fgetc ( f_source );
                    }
                }

                token.name = NULL;
                token.type = tString;
                token.data = tokenstr;
                tokenstr = NULL;;

                break;

            case '/':     //recognized slash
                symbol = fgetc ( f_source );

                if ( symbol == '/' ) //single line comment
                {
                    symbol = fgetc ( f_source );

                    while ( symbol != '\n' && symbol != EOF )
                    {
                        symbol = fgetc ( f_source );
                    }

                    ungetc ( symbol, f_source );
                    rerun = 1;
                }

                else if ( symbol == '*' ) //multiline comment
                {
                    while ( ( symbol = fgetc ( f_source ) ) )
                    {
                        if ( symbol == EOF )
                        {
                            SetError ( E_LEX, "Unterminated comment" );
                            rerun = 1;
                            ungetc ( symbol, f_source );
                            break;
                        }

                        else if ( symbol == '*' )
                        {
                            if ( ( symbol = fgetc ( f_source ) ) == '/' ) //properly terminated ml comment
                            {
                                rerun = 1;
                                break;
                            }

                            else if ( symbol == EOF )
                            {
                                SetError ( E_LEX, "Unterminated comment" );
                                rerun = 1;
                                ungetc ( symbol, f_source );
                                break;
                            }
                        }
                    }
                }

                else            //division symbol
                {
                    token.type = tDiv;
                    ungetc ( symbol, f_source );
                }

                break;

            case '+':     //plus symbol
                token.type = tAdd;
                break;

            case '-':     //minus symbol
                token.type = tSub;
                break;

            case '*':     //star symbol
                if ( ( symbol = fgetc ( f_source ) ) == '*' ) //power
                {
                    token.type = tPow;
                }

                else
                {
                    token.type = tMul;    //multiplication
                    ungetc ( symbol, f_source );
                }

                break;

            case '=':
                if ( ( symbol = fgetc ( f_source ) ) == '=' ) //equality
                {
                    token.type = tEq;
                }

                else
                {
                    token.type = tAssign; //assignment
                    ungetc ( symbol, f_source );
                }

                break;

            case '<':
                if ( ( symbol = fgetc ( f_source ) ) == '=' ) //less or equal then
                {
                    token.type = tLtEq;
                }

                else
                {
                    token.type = tLt; //less then
                    ungetc ( symbol, f_source );
                }

                break;

            case '>':
                if ( ( symbol = fgetc ( f_source ) ) == '=' ) //greater or equal then
                {
                    token.type = tGtEq;
                }

                else
                {
                    token.type = tGt; //greater then
                    ungetc ( symbol, f_source );
                }

                break;

            case '!':
                if ( ( symbol = fgetc ( f_source ) ) == '=' ) //not equal
                {
                    token.type = tNEq;
                }

                else
                {
                    SetError ( E_LEX, "Numerical literal error at \"!\" missing = " );
                    ungetc ( symbol, f_source );
                }

                break;

            case '(':
                token.type = tL_Br; //round left bracket
                break;

            case ')':
                token.type = tR_Br; //round right bracket
                break;

            case '[':
                token.type = tL_Sq_Br;  //square left bracket
                break;

            case ']':
                token.type = tR_Sq_Br;  //square right bracket
                break;

            case ',':
                token.type = tComma;    //comma
                break;

            case ':':     //colon
                token.type = tColon;
                break;

            case '\n':        //end of the line
                if ( returnedEol == 1 )
                {
                    while ( ( symbol = fgetc ( f_source ) ) == '\n' ); //removing additional EOLs

                    ungetc ( symbol, f_source );
                    rerun = 1;
                }

                else
                {
                    token.type = tEol;

                    while ( ( symbol = fgetc ( f_source ) ) == '\n' ); //removing additional EOLs

                    ungetc ( symbol, f_source );
                }

                break;

            case EOF:
                token.type = tEof;
                break;

            default:          //unrecognized symbol
                token.type = tError;
                SetError ( E_LEX, "Unknown input symbol" );
                break;
        }               //case

    }
    while ( rerun == 1 );     //do while

    if ( token.name != NULL )
    {
        RegPtr ( token.name );
    }

    if ( token.data != NULL )
    {
        RegPtr ( token.data );
    }


    if ( token.type == tEol )
    {
        returnedEol = 1;
    }

    else
    {
        returnedEol = 0;
    }

    return token;
}               //GetNextToken


////////////////////////////////////////////////////////////////////////////////
void append ( char** string, int symbol )
{
    static size_t count = 0;
    const int BLOCKSIZE = 16; //musi byt vetsi nez 1
    char* tmp = NULL;
    tmp = *string;

    if ( *string == NULL )
    {
        count = 0;
    }

    if ( count % BLOCKSIZE == 0 )
    {
        //zvetseni velikosti bufferu podle poctu nactenych znaku
        tmp = realloc ( *string, ( count + BLOCKSIZE + 1 ) * sizeof ( char ) );

        if ( tmp == NULL )      //realloc vraci null
        {
            if ( *string != NULL )
            {
                free ( *string );
            }

            SetError ( E_ERR_OTHER, "scan: Realloc ERROR!!!" );
        }

        tmp[count] = '\0';
    }

    tmp[count] = symbol;
    count++;
    tmp[count] = '\0';


    *string = tmp;
}


int symbolClass ( int symbol )
{
    if ( isalpha ( symbol ) || symbol == '_' )
    {
        return IDENTIFIER;
    }

    else if ( isdigit ( symbol ) )
    {
        return NUMLIT;
    }

    else if ( symbol == '"' )
    {
        return STRING;
    }

    return symbol;
}

int identType ( char* tokenstr )
{
    for ( int i = 0; i <= tEnd; i++ )
    {
        if ( strcmp ( tokenstr, TOKENSIGN[i] ) == 0 )
        {
            return i;
        }
    }

    for ( int i = tNil; i <= tFalse; i++ )
    {
        if ( strcmp ( tokenstr, TOKENSIGN[i] ) == 0 )
        {
            return i;
        }
    }

    for ( int i = 0; i < RESERVED_LEN; i++ )
    {
        if ( strcmp ( tokenstr, RESERVED[i] ) == 0 )
        {
            return tReserved;
        }
    }

    return tIdent;
}
