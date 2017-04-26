/*
 * rofi
 *
 * MIT/X11 License
 * Copyright © 2013-2017 Qball Cow <qball@gmpclient.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <assert.h>
#include <locale.h>
#include <glib.h>
#include <stdio.h>
#include <helper.h>
#include <string.h>
#include <xcb/xcb_ewmh.h>
#include "xcb-internal.h"
#include "rofi.h"
#include "settings.h"
#include "theme.h"
#include "widgets/widget-internal.h"

static int       test = 0;
struct xcb_stuff *xcb;

#define TASSERT( a )        {                            \
        assert ( a );                                    \
        printf ( "Test %i passed (%s)\n", ++test, # a ); \
}
#define TASSERTE( a, b )    {                                                            \
        if ( ( a ) == ( b ) ) {                                                          \
            printf ( "Test %i passed (%s == %s) (%u == %u)\n", ++test, # a, # b, a, b ); \
        }else {                                                                          \
            printf ( "Test %i failed (%s == %s) (%u != %u)\n", ++test, # a, # b, a, b ); \
            abort ( );                                                                   \
        }                                                                                \
}
gboolean error = FALSE;
void rofi_add_error_message ( GString *msg )
{
    fprintf ( stdout, "%s\n", msg->str );
    error = TRUE;
    g_string_free ( msg, TRUE );
}

int rofi_view_error_dialog ( const char *msg, G_GNUC_UNUSED int markup )
{
    fputs ( msg, stderr );
    return TRUE;
}

int show_error_message ( const char *msg, int markup )
{
    rofi_view_error_dialog ( msg, markup );
    return 0;
}
void rofi_view_get_current_monitor ( int *width, int *height )
{
    if ( width ) {
        *width = 1920;
    }
    if ( height ) {
        *height = 1080;
    }
}
double textbox_get_estimated_char_height ( void )
{
    return 16.0;
}
xcb_screen_t          *xcb_screen;
xcb_ewmh_connection_t xcb_ewmh;
int                   xcb_screen_nbr;
#include <x11-helper.h>

int main ( int argc, char ** argv )
{
    cmd_set_arguments ( argc, argv );

    if ( setlocale ( LC_ALL, "" ) == NULL ) {
        fprintf ( stderr, "Failed to set locale.\n" );
        return EXIT_FAILURE;
    }
    // EMPTY string.
    rofi_theme_parse_string ( "");
    TASSERT ( rofi_theme != NULL );
    rofi_theme_free ( rofi_theme );
    rofi_theme = NULL;
    // EMPTY global.
    rofi_theme_parse_string ( " *{} ");
    TASSERT ( rofi_theme != NULL );
    rofi_theme_free ( rofi_theme );
    rofi_theme = NULL;
    // EMPTY section.
    rofi_theme_parse_string ( " #{} ");
    TASSERT ( rofi_theme != NULL );
    rofi_theme_free ( rofi_theme );
    rofi_theme = NULL;

    rofi_theme_parse_string ( " Blaat ");
    TASSERT ( rofi_theme != NULL );
    TASSERT ( error == TRUE );
    rofi_theme_free ( rofi_theme );
    rofi_theme = NULL;
    error = FALSE;
    // C++ style comments with nesting.
    rofi_theme_parse_string ( "// Random comments // /*test */");
    TASSERT ( rofi_theme != NULL );
    rofi_theme_free ( rofi_theme );
    rofi_theme = NULL;
    rofi_theme_parse_string ( "/* test /* aap */ */");
    TASSERT ( rofi_theme != NULL );
    rofi_theme_free ( rofi_theme );
    rofi_theme = NULL;
    // C++ comments multiple lines.
    rofi_theme_parse_string ( "// Random comments\n// /*test */");
    TASSERT ( rofi_theme != NULL );
    rofi_theme_free ( rofi_theme );
    rofi_theme = NULL;
    rofi_theme_parse_string ( "/* test \n*\n*/* aap */ */");
    TASSERT ( rofi_theme != NULL );
    rofi_theme_free ( rofi_theme );
    rofi_theme = NULL;


    widget wid;
    wid.name = "blaat";
    wid.state = NULL;
    /** Boolean property */
    rofi_theme_parse_string ( "*{ test: true; test2:/* inline */false; }");
    TASSERT ( rofi_theme_get_boolean ( &wid, "test", FALSE) == TRUE );
    TASSERT ( rofi_theme_get_boolean ( &wid, "test2", TRUE) == FALSE );
    TASSERT ( rofi_theme != NULL );
    rofi_theme_free ( rofi_theme );
    rofi_theme = NULL;

    /** reference. */
    error = 0;
    rofi_theme_parse_string ( "* { test: true; test2:/* inline */false;} *{ a:@test; b:@test2;}");
    TASSERT ( error == 0 );
    TASSERT ( rofi_theme_get_boolean ( &wid, "test", FALSE) == TRUE );
    TASSERT ( rofi_theme_get_boolean ( &wid, "b", TRUE) == FALSE );
    TASSERT ( rofi_theme != NULL );
    rofi_theme_free ( rofi_theme );
    rofi_theme = NULL;
    {
        error = 0;
        rofi_theme_parse_string ( "* { test: 10em;}");
        TASSERT ( error == 0 );
        Distance d = (Distance){ 1, PW_PX, SOLID};
        Padding pi = (Padding){d,d,d,d};
        Padding p = rofi_theme_get_padding ( &wid, "test", pi);
        TASSERT (  p.left.distance == 10 );
        TASSERT (  p.left.type == PW_EM );
        TASSERT ( rofi_theme != NULL );
        rofi_theme_free ( rofi_theme );
        rofi_theme = NULL;
    }

    {
        rofi_theme = NULL;
        error = 0;
        rofi_theme_parse_string ( "* { test: 10px;}");
        TASSERT ( error == 0 );
        Distance d = (Distance){ 1, PW_PX, SOLID};
        Padding pi = (Padding){d,d,d,d};
        Padding p = rofi_theme_get_padding ( &wid, "test", pi);
        TASSERT (  p.left.distance == 10 );
        TASSERT (  p.left.type == PW_PX );
        TASSERT ( rofi_theme != NULL );
        rofi_theme_free ( rofi_theme );
        rofi_theme = NULL;
    }
    {
        rofi_theme = NULL;
        error = 0;
        rofi_theme_parse_string ( "* { test: 10%;}");
        TASSERT ( error == 0 );
        Distance d = (Distance){ 1, PW_PX, SOLID};
        Padding pi = (Padding){d,d,d,d};
        Padding p = rofi_theme_get_padding ( &wid, "test", pi);
        TASSERT (  p.left.distance == 10 );
        TASSERT (  p.left.type == PW_PERCENT );
        TASSERT ( rofi_theme != NULL );
        rofi_theme_free ( rofi_theme );
        rofi_theme = NULL;
    }
    {
        rofi_theme = NULL;
        error = 0;
        rofi_theme_parse_string ( "* { sol: 10% solid; dash: 10% dash;}");
        TASSERT ( error == 0 );
        Distance d = (Distance){ 1, PW_PX, DASH};
        Padding pi = (Padding){d,d,d,d};
        Padding p = rofi_theme_get_padding ( &wid, "sol", pi);
        TASSERT (  p.left.distance == 10 );
        TASSERT (  p.left.type == PW_PERCENT );
        TASSERT (  p.left.style == SOLID );
        p = rofi_theme_get_padding ( &wid, "dash", pi);
        TASSERT (  p.left.distance == 10 );
        TASSERT (  p.left.type == PW_PERCENT );
        TASSERT (  p.left.style == DASH );
        TASSERT ( rofi_theme != NULL );
        rofi_theme_free ( rofi_theme );
        rofi_theme = NULL;
    }
    {
        rofi_theme = NULL;
        error = 0;
        rofi_theme_parse_string ( "* { sol: 10px solid; dash: 14px dash;}");
        TASSERT ( error == 0 );
        Distance d = (Distance){ 1, PW_PX, DASH};
        Padding pi = (Padding){d,d,d,d};
        Padding p = rofi_theme_get_padding ( &wid, "sol", pi);
        TASSERT (  p.left.distance == 10 );
        TASSERT (  p.left.type == PW_PX);
        TASSERT (  p.left.style == SOLID );
        p = rofi_theme_get_padding ( &wid, "dash", pi);
        TASSERT (  p.left.distance == 14 );
        TASSERT (  p.left.type == PW_PX);
        TASSERT (  p.left.style == DASH );
        TASSERT ( rofi_theme != NULL );
        rofi_theme_free ( rofi_theme );
        rofi_theme = NULL;
    }
    {
        rofi_theme = NULL;
        error = 0;
        rofi_theme_parse_string ( "* { sol: 1.3em solid; dash: 1.5em dash;}");
        TASSERT ( error == 0 );
        Distance d = (Distance){ 1, PW_PX, DASH};
        Padding pi = (Padding){d,d,d,d};
        Padding p = rofi_theme_get_padding ( &wid, "sol", pi);
        TASSERT (  p.left.distance == 1.3 );
        TASSERT (  p.left.type == PW_EM );
        TASSERT (  p.left.style == SOLID );
        p = rofi_theme_get_padding ( &wid, "dash", pi);
        TASSERT (  p.left.distance == 1.5 );
        TASSERT (  p.left.type == PW_EM );
        TASSERT (  p.left.style == DASH );
        TASSERT ( rofi_theme != NULL );
        rofi_theme_free ( rofi_theme );
        rofi_theme = NULL;
    }

    {
        // Test newline and link.
        rofi_theme = NULL;
        error = 0;
        rofi_theme_parse_string ( "* { test: 10%;}\n* { blaat: @test;}");
        TASSERT ( error == 0 );
        Distance d = (Distance){ 1, PW_PX, SOLID};
        Padding pi = (Padding){d,d,d,d};
        Padding p = rofi_theme_get_padding ( &wid, "test", pi);
        TASSERT (  p.left.distance == 10 );
        TASSERT (  p.left.type == PW_PERCENT );
        Padding p2 = rofi_theme_get_padding ( &wid, "blaat", pi);
        TASSERT (  p2.left.distance == p.left.distance );
        TASSERT (  p2.left.type == p.left.type );
        TASSERT ( rofi_theme != NULL );
        rofi_theme_free ( rofi_theme );
        rofi_theme = NULL;
    }
    {
        rofi_theme = NULL;
        error = 0;
        rofi_theme_parse_string ( "\r\n\n\n\r\n* { center: center; east: east; west: west; south: south; north:north;}" );
        TASSERT ( error == 0 );
        TASSERT ( rofi_theme != NULL );
        TASSERT ( rofi_theme_get_position ( &wid, "center", WL_SOUTH) == WL_CENTER );
        TASSERT ( rofi_theme_get_position ( &wid, "south", WL_EAST) == WL_SOUTH);
        TASSERT ( rofi_theme_get_position ( &wid, "east", WL_WEST) == WL_EAST);
        TASSERT ( rofi_theme_get_position ( &wid, "west", WL_NORTH) == WL_WEST);
        TASSERT ( rofi_theme_get_position ( &wid, "north", WL_CENTER) == WL_NORTH);
        rofi_theme_parse_string ( "* { southwest: southwest; southeast: southeast; northwest: northwest; northeast:northeast;}" );
        TASSERT ( rofi_theme_get_position ( &wid, "southwest", WL_EAST) == WL_SOUTH_WEST);
        TASSERT ( rofi_theme_get_position ( &wid, "southeast", WL_WEST) == WL_SOUTH_EAST);
        TASSERT ( rofi_theme_get_position ( &wid, "northwest", WL_NORTH) == WL_NORTH_WEST);
        TASSERT ( rofi_theme_get_position ( &wid, "northeast", WL_CENTER) == WL_NORTH_EAST);
        rofi_theme_free ( rofi_theme );
        rofi_theme = NULL;
    }
    {
        rofi_theme = NULL;
        rofi_theme_parse_file ("/dev/null");
        TASSERT ( error == 0 );
        TASSERT ( rofi_theme != NULL );
        rofi_theme_free ( rofi_theme );
    }
    {
        rofi_theme = NULL;
        rofi_theme_parse_file ("not-existing-file.rasi");
        TASSERT ( error == TRUE );
        TASSERT ( rofi_theme == NULL );
        error = 0;
    }
    {
        rofi_theme = NULL;
        rofi_theme_parse_string ( "@import \"/dev/null\"" );
        TASSERT ( error == 0 );
        TASSERT ( rofi_theme != NULL );
        rofi_theme_free ( rofi_theme );


    }
}
