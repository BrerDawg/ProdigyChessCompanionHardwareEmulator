/*
Copyright (C) 2021 BrerDawg

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

//chss.cpp
//----v1.01		25-09-2017

#include "chss.h"

extern chess_wnd *ch_wnd;

extern int sw_matrix_cpanel[ cn_outs ][ cn_ins ];
extern void cpu_boot();
void cb_bt_help( Fl_Widget *, void *);
extern bool set_pause_state_to( int state );

extern int pause_state;



//-----------------------------------------------------------------------------

jpg_cpanel::jpg_cpanel( int xx, int yy, int wid, int hei, const char *label ) : Fl_Widget( xx, yy, wid, hei, label )
{
left_button = right_button = 0;
ctrl_key = shift_key = 0;
entered = 1;

jpg_panel = 0;

butt_pressed = -1;
hov_idx = -1;
}





void jpg_cpanel::set_left_click_cb( void (*p_cb)( void*, void* ), void *args )
{
left_click_p_cb = p_cb;
left_click_cb_args = args;
}



bool jpg_cpanel::init( string ss )
{
vbutt.clear();

if( !set_image( ss ) ) return 0;

//if( jpg_panel ) delete jpg_panel;

//jpg_panel = new Fl_JPEG_Image( ss.c_str() );

//if( !jpg_panel ) return;

size( jpg_panel->w(), jpg_panel->h() );

return 1;
}





bool jpg_cpanel::set_image( string ss )
{

if( jpg_panel ) delete jpg_panel;

jpg_panel = new Fl_JPEG_Image( ss.c_str() );

if( !jpg_panel ) return 0;

size( jpg_panel->w(), jpg_panel->h() );

redraw();
return 1;
}







void jpg_cpanel::add_image_bounding_button( button_tag o )
{
vbutt.push_back( o );
}




void jpg_cpanel::draw()
{
if( jpg_panel == 0 ) return;

int xx = x();
int yy = y();

fl_color( FL_BACKGROUND_COLOR );
fl_rectf( x(), y(), w(), h() );


//jpg_panel->draw( xx, yy, jpg_panel->w(), jpg_panel->h(), 0, mh );	//draw jpg
jpg_panel->draw( xx, yy );	//draw jpg



//draw highlight rect around button
button_tag o;
if( ( hov_idx != -1 ) && ( entered ) )
	{
	o = vbutt[ hov_idx ];

	int x1, y1, x2, y2;

	double scale_tmp = 1.0;

	int text_offsx = 0;
	if( o.text_button ) { scale_tmp = 1.0;	text_offsx = 0; }				//non image based button?

	
	x1 = nearbyint( o.x1 * scale_tmp + text_offsx );
	y1 = nearbyint( o.y1 * scale_tmp );
	x2 = nearbyint( o.x2 * scale_tmp + text_offsx );
	y2 = nearbyint( o.y2 * scale_tmp );

    fl_line_style ( FL_SOLID, 1 );

	fl_color( 0, 0, 0 );
	fl_rect( x() + x1,   y() + y1,    x2 - x1,    y2 - y1 );

	fl_color( 255, 255, 255 );
	fl_rect( x() + x1 + 1,   y() + y1 + 1,    x2 - x1 - 2,    y2 - y1 - 2 );
	}

}








int jpg_cpanel::do_mouse_move( int xx, int yy )
{
string s1, st;
button_tag o;

xx -= x();
yy -= y();

if( ( xx >= 0 ) && ( xx <= jpg_panel->w() ) )					//back within calc image?
	{
	if( ( yy >= 0 ) && ( yy <= jpg_panel->h() ) )
		{
		fl_cursor( FL_CURSOR_ARROW, FL_WHITE, FL_BLACK );
		}
	}

hov_idx = -1;
for( int i = 0; i < vbutt.size(); i++ )
	{
	o = vbutt[ i ];
	
	int x1, y1, x2, y2;
	double scale = 1.0;

	int text_offsx = 0;
	if( o.text_button ) { scale = 1.0;	text_offsx = 0; }				//non image based button?

	
	x1 = nearbyint( o.x1 * scale + text_offsx );
	y1 = nearbyint( o.y1 * scale );
	x2 = nearbyint( o.x2 * scale + text_offsx );
	y2 = nearbyint( o.y2 * scale );

	if ( ( xx >= x1 ) & ( xx <= x2 ) )
		{
		if ( ( yy >= y1 ) & ( yy <= y2 ) )
			{
			fl_cursor( FL_CURSOR_ARROW, FL_WHITE, FL_BLACK );
//exit(0);
			hov_idx = i;
			Fl::focus( this );
			return i;
			}
		}
	}
return -1;
}








int jpg_cpanel::do_butt_left()
{
string s1, st;
button_tag o;


//develop_font_pos_x = lb_x;
//develop_font_pos_y = lb_y;



/*
strpf( s1, "-----------\n", "" );
st += s1;
strpf( s1, "o.sname = \"\";\n", "" );
st += s1;
strpf( s1, "o.keycode = vbutt.size();\n" );
st += s1;
strpf( s1, "o.x1 = %d;\n", lb_x );
st += s1;
strpf( s1, "o.y1 = %d;\n", lb_y );
st += s1;
strpf( s1, "o.x2 = %d;\n", rb_x );
st += s1;
strpf( s1, "o.y2 = %d;\n", rb_y );
st += s1;
st += "vbutt.push_back( o );\n";
strpf( s1, "-----------\n", "" );
st += s1;

printf( "do_butt_left() - '%s'\n", st.c_str() );
*/

if( ( hov_idx != -1 ) & ( hov_idx < vbutt.size() ) )
	{
//	Fl::focus( this );

	o = vbutt[ hov_idx ];
	butt_pressed = o.keycode;
	
	id0 = o.id0;
	id1 = o.id1;
	
	if( left_click_p_cb ) left_click_p_cb( this, left_click_cb_args );



//	process_button( o.sname );

//	strpf( s1, "butt_pressed: %03d, key: '%s'", o.keycode , o.sname.c_str() );
	
//	printf( "do_butt_left() - '%s'\n", s1.c_str() );
	}
}



/*
//do button action
void jpg_cpanel::process_button( string ss )
{
string s1, s2;



//if( iButtonPressed==-1 ) return;
}
*/






int jpg_cpanel::do_butt_right()
{
string s1, st;
button_tag o;

strpf( s1, "o.sname = \"\";\n", "" );
st += s1;
strpf( s1, "o.x1 = %d;\n", lb_x );
st += s1;
strpf( s1, "o.y1 = %d;\n", lb_y );
st += s1;
strpf( s1, "o.x2 = %d;\n", rb_x );
st += s1;
strpf( s1, "o.y2 = %d;\n", rb_y );
st += s1;
st += "vbutt.push_back( o );\n";

printf( "do_butt_right() - '%s'\n", st.c_str() );
}










static int bDonePaste = 0;		//need this to be 0 to make dragging from web browser in linux work


int jpg_cpanel::handle( int e )
{
bool need_redraw = 0;
bool dont_pass_on = 0;


#ifdef compile_for_windows
bDonePaste = 1;					//does not seem to be required for Windows, so nullify by setting to 1
#endif

if ( e == FL_PASTE )	//needed below code because on drag release the first FL_PASTE call does not have valid text as yet,
	{					//possibly because of a delay in X windows, so have used Fl:paste(..) to send another paste event and 

//	if( wdg_hasfocus == this )
		{
		string s = Fl::event_text();
		int len = Fl::event_length();

		printf( "paste: '%s'\n", s.c_str() );
		if( len )								//anything dropped/pasted?
			{
//			paste_str_num( s );

//			oneshot = 1;
//			Fl::add_timeout( 0.1, cb_calc_timer1 );		//update controls, post queued messages
			need_redraw = 1;
			dont_pass_on = 1;	
			}
		}
	}


//if ( e & FL_SHORTCUT )	
//	{
//	printf("shortcut: %d\n", 0 );
//	}

if ( e == FL_ENTER )
	{
	entered = 1;
		Fl::focus( this );
//	printf("FL_ENTER\n");
	dont_pass_on = 1;
	}

if ( e == FL_LEAVE )
	{
	entered = 0;
//	printf("FL_LEAVE\n");
	dont_pass_on = 1;
	}


if ( e == FL_FOCUS )
	{
//	need_redraw = 1;

	dont_pass_on = 1;
	}

if (e == FL_DND_DRAG)
	{
	printf("\nDrag\n");
	dont_pass_on = 1;
	}

if (e == FL_DND_ENTER)
	{
	printf("\nDrag Enter\n");
	dont_pass_on = 1;
	}

if (e == FL_DND_RELEASE)
	{
	printf("\nDrag Release\n");
	dont_pass_on = 1;
	}

//printf("jpg_cpanel() - e: %d\n", e);

if ( e & FL_MOVE )
	{
//printf("mywnd:: FL_MOVE\n");

	mousex = Fl::event_x();
	mousey = Fl::event_y();

	int ibutt = do_mouse_move( mousex, mousey );
	
	if( ibutt >= 0 )
		{
		}

	need_redraw = 1;
	dont_pass_on = 1;
	}


if ( e == FL_PUSH )
	{
	if( Fl::event_button() == 1 )
		{
		int xx = Fl::event_x();
		int yy = Fl::event_y();

printf("xx: %d %d\n", xx - x(), yy - y() );

//		if( ( xx >= 0 ) && ( xx <= vjpg_cx[ image_idx ] ) )
//			{
//			if( ( yy >= menu_hei ) && ( yy <= vjpg_cy[ image_idx ] ) )
			
		if( ( hov_idx != -1 ) & ( hov_idx < vbutt.size() ) )
				{
				left_button = 1;

				lb_x = Fl::event_x();
				lb_y = Fl::event_y();

				do_butt_left();

				need_redraw = 1;
				dont_pass_on = 1;

				Fl::focus( this );
	//			take_focus();
				}
//			}
		}


	if( Fl::event_button() == 2 )					//middle wheel button?
		{

//		Fl::paste( *calc_wnd, 0 );					//issue a selection paste

		need_redraw = 1;
		dont_pass_on = 1;
		}




	if( Fl::event_button() == 3 )
		{
		right_button = 1;

		rb_x = Fl::event_x();
		rb_y = Fl::event_y();

		do_butt_right();

		need_redraw = 1;
		dont_pass_on = 1;
		}
	}




if ( e == FL_RELEASE )
	{
	
	if(Fl::event_button()==1)
		{
		left_button = 0;
		
		do_butt_left();

		need_redraw = 1;
		}

	if( Fl::event_button() == 3 )
		{
		right_button = 0;
		need_redraw = 1;
		}

	dont_pass_on = 1;
	}


//if ( ( e == FL_KEYDOWN ) || ( e == FL_SHORTCUT ) )					//key pressed?
if ( e == FL_KEYDOWN )													//key pressed?
	{
	int key = Fl::event_key();
	
	if( ( key == FL_Control_L ) || (  key == FL_Control_R ) ) ctrl_key = 1;
	if( ( key == FL_Shift_L ) || (  key == FL_Shift_R ) ) shift_key = 1;


	need_redraw = 1;
	dont_pass_on = 1;
	}


if ( e == FL_KEYUP )												//key release?
	{
	int key = Fl::event_key();
	
	if( ( key == FL_Control_L ) || ( key == FL_Control_R ) ) ctrl_key = 0;
	if( ( key == FL_Shift_L ) || (  key == FL_Shift_R ) ) shift_key = 0;

	hov_idx = -1;
	need_redraw = 1;
	dont_pass_on = 1;
	}


Fl_Widget *wdg_hasfocus = this;
if ( ( e == FL_MOUSEWHEEL ) & ( wdg_hasfocus == this ) )
	{
	mousewheel = Fl::event_dy();
	
	if( !ctrl_key )
		{
		}
	else{
		if( ( hov_idx != -1 ) & ( hov_idx < ( vbutt.size() - 1 ) ) )		//change FIX via wheel?
			{
			string s1;
			button_tag o;

			}
		}

	need_redraw = 1;
	dont_pass_on = 0;
	}
	
if ( need_redraw ) redraw();

if( dont_pass_on ) return 1;

return Fl_Widget::handle(e);
}




//-----------------------------------------------------------------------------






st_piece_val piece_val[] = 
{
0.0,                //pt_none
1.0,                //pt_pawn
3.2,                //pt_knight
3.2,                //pt_bishop
5.1,                //pt_rook
8.8,                //pt_queen
0.0,                //pt_king
};


void cb_coord_led( Fl_Widget *w, void* v )
{

GCLed *o=( GCLed*)w;

//int ii = o->GetColIndex();
//o->ChangeCol( !ii );
}






//toggle image while clicked
void cb_cpnl( void* w, void* v )
{
jpg_cpanel *wdj = (jpg_cpanel*)w;

button_tag o = wdj->vbutt[ wdj->hov_idx ];


//printf( "cb_cpnl() - '%s', hov_idx: %d, id0: %d, id1: %d, pressed: %d\n", o.sname.c_str(), wdj->hov_idx, o.id0, o.id1, wdj->left_button );

string s1;
if( wdj->left_button ) s1 = "prodigy_key.jpg";
else s1 = "350px-Prodigy.jpg";


wdj->set_image( s1 );
wdj->parent()->redraw();					//redraw parent incase image (and wdj) is smaller
}





void cb_cpnl_butt( void* w, void* v )
{
jpg_cpanel *wdj = (jpg_cpanel*)w;

button_tag o = wdj->vbutt[ wdj->hov_idx ];


printf( "cb_cpnl_butt() - '%s', hov_idx: %d, id0: %d, id1: %d, pressed: %d\n", o.sname.c_str(), wdj->hov_idx, o.id0, o.id1, wdj->left_button );

sw_matrix_cpanel[ o.id0 ][ o.id1 ] = wdj->left_button;

if( ( o.id0 == 8 ) && ( o.id1 == 3 )  ) ch_wnd->deselect_all();				//CE pressed?
}







void cb_popup_chess( Fl_Widget *w, void* v )
{
int which = (intptr_t)v;

int crdx, crdy;
 
if( !ch_wnd->find_coord( ch_wnd->mousex, ch_wnd->mousey, crdx, crdy ) ) return;
if( which == 0 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_pawn ].val;
	ch_wnd->place_piece( ch_wnd->vwht, pt_pawn, crdx, crdy, val, 0 );
	}

if( which == 1 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_rook ].val;
	ch_wnd->place_piece( ch_wnd->vwht, pt_rook, crdx, crdy, val, 0 );
	}

if( which == 2 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_knight ].val;
	ch_wnd->place_piece( ch_wnd->vwht, pt_knight, crdx, crdy, val, 0 );
	}

if( which == 3 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_bishop ].val;
	ch_wnd->place_piece( ch_wnd->vwht, pt_bishop, crdx, crdy, val, 0 );
	}

if( which == 4 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_queen ].val;
	ch_wnd->place_piece( ch_wnd->vwht, pt_queen, crdx, crdy, val, 0 );
	}

if( which == 5 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_king ].val;
	ch_wnd->place_piece( ch_wnd->vwht, pt_king, crdx, crdy, val, 0 );
	}


if( which == 6 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_pawn ].val;
	ch_wnd->place_piece( ch_wnd->vblk, pt_pawn, crdx, crdy, val, 0 );
	}

if( which == 7 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_rook ].val;
	ch_wnd->place_piece( ch_wnd->vblk, pt_rook, crdx, crdy, val, 0 );
	}

if( which == 8 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_knight ].val;
	ch_wnd->place_piece( ch_wnd->vblk, pt_knight, crdx, crdy, val, 0 );
	}

if( which == 9 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_bishop ].val;
	ch_wnd->place_piece( ch_wnd->vblk, pt_bishop, crdx, crdy, val, 0 );
	}

if( which == 10 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_queen ].val;
	ch_wnd->place_piece( ch_wnd->vblk, pt_queen, crdx, crdy, val, 0 );
	}

if( which == 11 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	
	int val = piece_val[ pt_king ].val;
	ch_wnd->place_piece( ch_wnd->vblk, pt_king, crdx, crdy, val, 0 );
	}
	
if( which == 12 )
	{
	int idx;
	ch_wnd->delete_piece( crdx, crdy, idx );
	}
}



//-------------------------------------------------------------
Fl_Menu_Item menu_popup_chess[] =
{
//	{ "&Help", 0, 0, 0, FL_SUBMENU },
		{ "place white pawn", 0, 	(Fl_Callback *)cb_popup_chess,(void*)0},
		{ "place white rook", 0, 	(Fl_Callback *)cb_popup_chess,(void*)1},
		{ "place white knight", 0	, 		(Fl_Callback *)cb_popup_chess,(void*)2},
		{ "place white bishop", 0	, 		(Fl_Callback *)cb_popup_chess,(void*)3},
		{ "place white queen", 0	, 		(Fl_Callback *)cb_popup_chess,(void*)4},
		{ "place white king", 0	, 		(Fl_Callback *)cb_popup_chess,(void*)5, FL_MENU_DIVIDER },
		{ "place black pawn", 0	, 		(Fl_Callback *)cb_popup_chess,(void*)6},
		{ "place black rook", 0, 	(Fl_Callback *)cb_popup_chess,(void*)7},
		{ "place black knight", 0	, 		(Fl_Callback *)cb_popup_chess,(void*)8},
		{ "place black bishop", 0	, 		(Fl_Callback *)cb_popup_chess,(void*)9},
		{ "place black queen", 0	, 		(Fl_Callback *)cb_popup_chess,(void*)10},
		{ "place black king", 0	, 		(Fl_Callback *)cb_popup_chess,(void*)11, FL_MENU_DIVIDER },
		{ "delete piece", 0	, 		(Fl_Callback *)cb_popup_chess,(void*)12 },
		
		{ 0 },
};
//-------------------------------------------------------------



//-------------------------------------------------------------------------------
void cb_usec( Fl_Widget* w, void* v )
{
Fl_Int_Input *o = (Fl_Int_Input*) w;

o->value();
int ii;
sscanf( o->value(), "%d", &ii );

if( ii < 0 ) ii = 0;
if( ii > 100000 ) ii = 100000;

printf( "cb_usec: %d\n", ii );

string s1;
strpf( s1, "%d", ii );
o->value( s1.c_str() );

ch_wnd->usec = ii;
}





void cb_bt_reboot( Fl_Widget* w, void* v )
{
string s1;


strpf( s1, "Reboot Prodigy?" );

int ret = fl_choice( s1.c_str(),"Cancel","Reboot", 0 );
if( ret == 1 )
	{
	cpu_boot();
	}

}





void cb_bt_pause( Fl_Widget* w, void* v )
{
if( pause_state == 0 ) set_pause_state_to( 2 );				//toggle pause
else set_pause_state_to( 0 );
}




chess_wnd::chess_wnd( int xx, int yy, int wid, int hei, const char *label ) : Fl_Double_Window( xx, yy, wid ,hei, label )
{
left_button = 0;
left_pressed = 0;
tick_cnt = 0;
flash = 0;




col_bkgd = fl_color();

show_piece_move_lines = 1;

usec = 500;
paused = 0;


//----
jpg_cpanel *cpnl;

cpnl = new jpg_cpanel( wid - 360, 30, 100, 100, "" );
string s1 = "350px-Prodigy.jpg";

cpnl->init( s1 );



cpnl->set_left_click_cb( cb_cpnl, (void*)0 );				//will toggle image

button_tag o;

o.text_button = 0;

o.sname = "toggle";
strcpy( o.sztooltip, "");
o.keycode = cpnl->vbutt.size();
o.id0 = 0;							//sw matrix
o.id1 = 0;
o.x1 = 0;
o.y1 = 0;
o.x2 = cpnl->w();
o.y2 = cpnl->h();
cpnl->add_image_bounding_button( o );

//----







//----
jpg_cpanel *cpnl_butt;

cpnl_butt = new jpg_cpanel( wid - 295, hei - 320, 100, 100, "" );
s1 = "200px-Destiny-Tastatur.jpg";
cpnl_butt->init( s1 );

cpnl_butt->set_left_click_cb( cb_cpnl_butt, (void*)0 );



o.text_button = 0;					//these are imaged based buttons

o.sname = "restore";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 8;							//sw matrix
o.id1 = 2;
o.x1 = 10;
o.y1 = 75;
o.x2 = 55;
o.y2 = 106;
cpnl_butt->add_image_bounding_button( o );

o.sname = "hint";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 9;							//sw matrix
o.id1 = 2;
o.x1 = 55;
o.y1 = 76;
o.x2 = 101;
o.y2 = 106;
cpnl_butt->add_image_bounding_button( o );

o.sname = "audio";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 9;							//sw matrix
o.id1 = 3;
o.x1 = 100;
o.y1 = 76;
o.x2 = 147;
o.y2 = 106;
cpnl_butt->add_image_bounding_button( o );

o.sname = "ce";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 8;							//sw matrix
o.id1 = 3;
o.x1 = 146;
o.y1 = 76;
o.x2 = 192;
o.y2 = 106;
cpnl_butt->add_image_bounding_button( o );



o.sname = "g7";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 8;							//sw matrix
o.id1 = 1;
o.x1 = 10;
o.y1 = 107;
o.x2 = 55;
o.y2 = 138;
cpnl_butt->add_image_bounding_button( o );

o.sname = "h8";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 9;							//sw matrix
o.id1 = 1;
o.x1 = 55;
o.y1 = 107;
o.x2 = 101;
o.y2 = 138;
cpnl_butt->add_image_bounding_button( o );

o.sname = "time";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 9;							//sw matrix
o.id1 = 4;
o.x1 = 100;
o.y1 = 106;
o.x2 = 148;
o.y2 = 138;
cpnl_butt->add_image_bounding_button( o );


o.sname = "level";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 8;							//sw matrix
o.id1 = 4;
o.x1 = 146;
o.y1 = 106;
o.x2 = 193;
o.y2 = 137;
cpnl_butt->add_image_bounding_button( o );



o.sname = "d4";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 8;							//sw matrix
o.id1 = 0;
o.x1 = 10;
o.y1 = 136;
o.x2 = 55;
o.y2 = 168;
cpnl_butt->add_image_bounding_button( o );

o.sname = "e5";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 9;							//sw matrix
o.id1 = 0;
o.x1 = 55;
o.y1 = 136;
o.x2 = 101;
o.y2 = 168;
cpnl_butt->add_image_bounding_button( o );

o.sname = "f6";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 9;							//sw matrix
o.id1 = 5;
o.x1 = 100;
o.y1 = 136;
o.x2 = 148;
o.y2 = 168;
cpnl_butt->add_image_bounding_button( o );

o.sname = "chbrd";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 8;							//sw matrix
o.id1 = 5;
o.x1 = 147;
o.y1 = 136;
o.x2 = 195;
o.y2 = 168;
cpnl_butt->add_image_bounding_button( o );




o.sname = "a1";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 8;							//sw matrix
o.id1 = 9;
o.x1 = 10;
o.y1 = 169;
o.x2 = 55;
o.y2 = 199;
cpnl_butt->add_image_bounding_button( o );

o.sname = "b2";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 9;							//sw matrix
o.id1 = 9;
o.x1 = 55;
o.y1 = 169;
o.x2 = 102;
o.y2 = 199;
cpnl_butt->add_image_bounding_button( o );

o.sname = "c3";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 9;							//sw matrix
o.id1 = 6;
o.x1 = 101;
o.y1 = 168;
o.x2 = 149;
o.y2 = 199;
cpnl_butt->add_image_bounding_button( o );

o.sname = "verify";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 8;							//sw matrix
o.id1 = 6;
o.x1 = 148;
o.y1 = 167;
o.x2 = 196;
o.y2 = 199;
cpnl_butt->add_image_bounding_button( o );




o.sname = "go";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 8;							//sw matrix
o.id1 = 8;
o.x1 = 10;
o.y1 = 199;
o.x2 = 56;
o.y2 = 231;
cpnl_butt->add_image_bounding_button( o );

o.sname = "black";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 9;							//sw matrix
o.id1 = 8;
o.x1 = 55;
o.y1 = 199;
o.x2 = 103;
o.y2 = 231;
cpnl_butt->add_image_bounding_button( o );

o.sname = "white";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 9;							//sw matrix
o.id1 = 7;
o.x1 = 102;
o.y1 = 199;
o.x2 = 150;
o.y2 = 231;
cpnl_butt->add_image_bounding_button( o );

o.sname = "enter";
strcpy( o.sztooltip, "");
o.keycode = cpnl_butt->vbutt.size();
o.id0 = 8;							//sw matrix
o.id1 = 7;
o.x1 = 149;
o.y1 = 199;
o.x2 = 197;
o.y2 = 231;
cpnl_butt->add_image_bounding_button( o );

//----




flm_menu_popup_chess = new Fl_Menu_Button( 0, 0, wid, hei, "Change Piece" );
flm_menu_popup_chess->textsize( 12 );
flm_menu_popup_chess->type( Fl_Menu_Button::POPUP3 );
flm_menu_popup_chess->menu( menu_popup_chess );
flm_menu_popup_chess->type( 0x8 );								//disable mouse activation by setting any val above these ornings:  Fl_Menu_Button::POPUP1 ---> Fl_Menu_Button::POPUP3



//static ulli zxc;

//instance = -1;


//thrd1 = 0;

//proc_mode = pm_white_mve;
//look_ahead = 5;

mouse_coord_x = -1;
mouse_coord_y = -1;

//sel_crdx = sel_crdy = -1;

//menu bar
//menu_chess = new Fl_Menu_Bar( 0, 0, wid, 25 );
//menu_chess->textsize(12);
//menu_chess->copy( menuchess, this );
//menu_hei = menu_chess->h();


//fi_move_input = new My_Input_Enter( w() - 160, 30, 150, 20 );


checker_x = 70;
checker_y = checker_x;
checker_off_x = 30;
checker_off_y = 40;

piece_off_y = 5;

scale = 1.5;


brd_wid = checker_x * 8;
brd_hei = checker_y * 8;




wdg_seg0 = new seg7_wdg( 770-37, 300+015, 26, 35, "seg0");
wdg_seg1 = new seg7_wdg( 796-37, 300+015, 26, 35, "seg1");
wdg_seg2 = new seg7_wdg( 822-37, 300+015, 26, 35, "seg2");
wdg_seg3 = new seg7_wdg( 848-37, 300+015, 26, 35, "seg3");

wdg_seg0->inverted = 0;
wdg_seg0->diode_a = 1;
wdg_seg0->diode_b = 1;
wdg_seg0->diode_c = 1;
wdg_seg0->diode_d = 1;
wdg_seg0->diode_e = 1;
wdg_seg0->diode_f = 1;
wdg_seg0->diode_g = 1;
wdg_seg0->diode_dpl = 1;
wdg_seg0->diode_dpr = 1;
wdg_seg0->redraw();



int offx = checker_off_x - 10;
int offy = checker_off_y - 10;

//x leds
for( int x = 0; x < 8; x++ )
	{
	GCLed *led1;

	int px = offx + checker_x/2 + 7 + x * checker_x;
	int py = offy + 8 * checker_y + 30;
	
	led1 = new GCLed( px, py, 14, 14, "" );
	led1->tooltip( "put a tooltip" );

	led1->align( FL_ALIGN_RIGHT );

	led1->led_style = cn_gcled_style_square;
	led1->led_style = cn_gcled_style_round;

	led1->SetColIndex(0, 120, 0, 0);
	led1->SetColIndex(1, 255,40, 40);
	led1->ChangeCol( 0 );
	led1->callback( cb_coord_led, (void*)0 );
	
	coord_ledx[x] = led1;
	}

//y leds
for( int y = 0; y < 8; y++ )
	{
	GCLed *led1;

	int px = checker_off_x - 25;
	int py = offy + checker_y/2 + 4 + y * checker_y;
	
	led1 = new GCLed( px, py, 14, 14, "" );
	led1->tooltip( "put a tooltip" );

	led1->align( FL_ALIGN_RIGHT );

	led1->led_style = cn_gcled_style_square;
	led1->led_style = cn_gcled_style_round;

	led1->SetColIndex(0, 120, 0, 0);
	led1->SetColIndex(1, 255,40, 40);
	led1->ChangeCol( 0 );
	led1->callback( cb_coord_led, (void*)0 );
	
	coord_ledy[y] = led1;
	}




Fl_Button *bt_pause = new Fl_Button( wid - 580, hei - 20 -3, 65, 20, "pause" );
bt_pause->labelsize(10);
bt_pause->tooltip( "toggle pause state" );
bt_pause->callback( cb_bt_pause, (void*)0 );


bx_status = new Fl_Box( wid - 500, hei - 20 -3, 65, 20, "paused" );


fi_usec = new Fl_Int_Input( wid - 300, hei - 23, 80, 18, "usec delay:" );
fi_usec->textsize(10);
fi_usec->tooltip("set a delay period in uS and hit enter (0->100000), affects emulation speed,\nlower vals give faster emulation but hog your PC's processor,\ntry 1000 (1mS),\nyou can check Prodigy's time function to see how fast a second changes on its led display (after a game has begun)" );
fi_usec->callback( cb_usec, (void*)0 );
fi_usec->when( FL_WHEN_ENTER_KEY );

strpf( s1, "%d", usec );
fi_usec->value( s1.c_str() );



Fl_Button *bt_reboot = new Fl_Button( wid - 210, hei - 20 -3, 65, 20, "reboot..." );
bt_reboot->labelsize(10);
bt_reboot->tooltip( "repower Prodigy" );
bt_reboot->callback( cb_bt_reboot, (void*)0 );


Fl_Button *bt_help = new Fl_Button( wid - 50, hei - 20 -3, 45, 20, "help..." );
bt_help->labelsize(10);
bt_help->tooltip( "will open the 'help.txt' file in your favorite editor..." );
bt_help->callback( cb_bt_help, (void*)0 );

init();
}







chess_wnd::~chess_wnd( )
{
}






void chess_wnd::tick()
{
tick_cnt++;

if( !(tick_cnt%1) ) flash = !flash;

if ( paused )
	{
	if( flash ) bx_status->label("");
	else  bx_status->label("-Paused-");
	}
else{
	bx_status->label("-Running-");
	}
redraw();
}



bool chess_wnd::select_piece( int crdx, int crdy, bool sel )
{
if( ( crdx < 0 ) || (  crdx > 7 ) ) return 0;
if( ( crdy < 0 ) || (  crdy > 7 ) ) return 0;


//st_piece *plyr;
bool is_white;
int idx = find_piece_at_xy( crdx, crdy, is_white );

if( idx >= 0 )
    {
	if( is_white )
		{
		vwht[ idx ].sel = sel;
		}
 	else{
		vblk[ idx ].sel = sel;
		}
	return 1;
    }

return 0;
}



bool chess_wnd::toggle_select_piece( int crdx, int crdy )
{
if( ( crdx < 0 ) || (  crdx > 7 ) ) return 0;
if( ( crdy < 0 ) || (  crdy > 7 ) ) return 0;


bool is_white;

int idx = find_piece_at_xy( crdx, crdy, is_white );

if( idx >= 0 )
    {
	if( is_white )
		{
		vwht[ idx ].sel = !vwht[ idx ].sel;
		}
	else{
		vblk[ idx ].sel = !vblk[ idx ].sel;
		}
    return 1;
    }

return 0;
}



bool chess_wnd::deselect_all()
{
for( int i = 0; i < vwht.size(); i++ )
	{
	vwht[i].sel = 0;
	}

for( int i = 0; i < vblk.size(); i++ )
	{
	vblk[i].sel = 0;
	}
/*
for( int i = 0; i < piece_cnt0; i++ )
    {
    int col = 0;
    int type;
    int x;
    int y;
    double val;
	bool sel;

	int crdx = plyr0[ i ].x;
	int crdy = plyr0[ i ].y;

	select_piece( crdx, crdy, 0 );	       
//	redraw();
//printf("piece %d, type= %d, x= %d, y= %d\n", i, type, x, y );
    }

for( int i = 0; i < piece_cnt1; i++ )
    {
    int col = 1;
    int type;
    int x;
    int y;
    double val;

	int crdx = plyr1[ i ].x;
	int crdy = plyr1[ i ].y;

	select_piece( crdx, crdy, 0 );	       
//	redraw();
    }
*/
}








//returns index of first found selected piece, else -1
int chess_wnd::find_selected( int &crdx, int &crdy, bool &is_plyr0 )
{

for( int i = 0; i < vwht.size(); i++ )
	{
	if( vwht[i].sel )
		{
		is_plyr0 = 1;
		crdx = vwht[i].x;
 		crdy = vwht[i].y;
		return i;
		}
	}

for( int i = 0; i < vblk.size(); i++ )
	{
	if( vblk[i].sel )
		{
		is_plyr0 = 0;
		crdx = vblk[i].x;
 		crdy = vblk[i].y;
		return i;
		}
	}

return -1;

/*
is_plyr0 = 1;

for ( int i = 0; i < piece_cnt0; i++ )
    {
    if( plyr0[ i ].sel )
        {
		crdx = plyr0[ i ].x;
 		crdy = plyr0[ i ].y;
        *plyr = plyr0;
        return i;                   //piece found
        }
    }


is_plyr0 = 0;
for ( int i = 0; i < piece_cnt1; i++ )
    {
    if( plyr1[ i ].sel )
        {
		crdx = plyr1[ i ].x;
		crdy = plyr1[ i ].y;
		*plyr = plyr1;
		return i;                   //piece found
        
        }
    }

return -1;
*/
}







bool chess_wnd::delete_piece( int crdx, int crdy, int &idx )
{
if( ( crdx < 0 ) || (  crdx > 7 ) ) return 0;
if( ( crdy < 0 ) || (  crdy > 7 ) ) return 0;


bool is_white;
idx = find_piece_at_xy( crdx, crdy, is_white );

if( idx > -1 )
	{
	if( is_white )
		{
		vwht.erase( vwht.begin() + idx );
//		vwht[ idx ].type = pt_none;
//		vwht[ idx ].sel = 0;
		}
 	else{
		vblk.erase( vblk.begin() + idx );
//		vblk[ idx ].type = pt_none;
//		vblk[ idx ].sel = 0;
		}
	return 1;
	}

return 0;
}






//crdx = 0: is led A1
//crdx = 7: is led H1

void chess_wnd::set_led_x( int crdx, bool on )
{
if( ( crdx < 0 ) || (  crdx > 7 ) ) return;

coord_ledx[crdx]->ChangeCol( on );
}




//crdy = 0: is led A1
//crdy = 7: is led A8
void chess_wnd::set_led_y( int crdy, bool on )
{
if( ( crdy < 0 ) || (  crdy > 7 ) ) return;

coord_ledy[crdy]->ChangeCol( on );
}





void chess_wnd::init()
{
string s1;

//sel_crdx = sel_crdy = -1;

new_game();

//move_piece(0, 0, 3, 3 );
}



void chess_wnd::new_game()
{
double val;

//for( int i = 0; i < 64; i++ )
//	{
//	brd[ i ].pce_typ = pt_none;
//	}

//timer_mve_cnt = 0;

vwht.clear();
vblk.clear();

bool plyr;
bottom_plyr = 1;

if( bottom_plyr ) plyr = 1;
else plyr = 0;
col_plyr0 = 0;                                      //player 0 is black
col_plyr1 = !col_plyr0;                             //player 1 is opossite col to player 0



dir_plyr0 = -1;                     //player 0 moves downward
dir_plyr1 = -dir_plyr0;             //player 1 moves opossite to player 0

comp_comp = 0;						//computer vs computer
comp_mve = 0;						//computer moves next


piece_cnt0 = 16;
piece_cnt1 = 16;

int sel = 0;

//bottom player
val = piece_val[ pt_rook ].val;
place_piece( vwht, pt_rook, 0, 0, val, sel );

val = piece_val[ pt_knight ].val;
place_piece( vwht, pt_knight, 1, 0, val, sel );

val = piece_val[ pt_bishop ].val;
place_piece( vwht, pt_bishop, 2, 0, val, sel );

val = piece_val[ pt_queen ].val;
place_piece( vwht, pt_queen, 3, 0, val, sel );

val = piece_val[ pt_king ].val;
place_piece( vwht, pt_king, 4, 0, val, sel );

val = piece_val[ pt_bishop ].val;
place_piece( vwht, pt_bishop, 5, 0, val, sel );

val = piece_val[ pt_knight ].val;
place_piece( vwht, pt_knight, 6, 0, val, sel );

val = piece_val[ pt_rook ].val;
place_piece( vwht, pt_rook, 7, 0, val, sel );

for( int i = 0; i < 8; i ++ )
    {
    val = piece_val[ pt_pawn ].val;
	place_piece( vwht, pt_pawn,i, 1, val, sel );
   }



//top player
plyr = !plyr;

val = piece_val[ pt_rook ].val;
place_piece( vblk, pt_rook, 0, 7, val, sel );

val = piece_val[ pt_knight ].val;
place_piece( vblk, pt_knight, 1, 7, val, sel );

val = piece_val[ pt_bishop ].val;
place_piece( vblk, pt_bishop, 2, 7, val, sel );

val = piece_val[ pt_queen ].val;
place_piece( vblk, pt_queen, 3, 7, val, sel );

val = piece_val[ pt_king ].val;
place_piece( vblk, pt_king, 4, 7, val, sel );

val = piece_val[ pt_bishop ].val;
place_piece( vblk, pt_bishop, 5, 7, val, sel );

val = piece_val[ pt_knight ].val;
place_piece( vblk, pt_knight, 6, 7, val, sel );

val = piece_val[ pt_rook ].val;
place_piece( vblk, pt_rook, 7, 7, val, sel );

for( int i = 0; i < 8; i ++ )
    {
    val = piece_val[ pt_pawn ].val;
	place_piece( vblk, pt_pawn, i, 6, val, sel );
    }

//set pawns as not yet moved
//for( int i = 0; i < max_pieces_per_side; i ++ )
//    {
//    if( plyr0[ i ].type == pt_pawn )
//        {
//        plyr0[ i ].moved = 0;
//       }
//    }


//set pawns as not yet moved
//for( int i = 0; i < max_pieces_per_side; i ++ )
//    {
//    if( plyr1[ i ].type == pt_pawn )
//        {
//        plyr1[ i ].moved = 0;
//        }
//    }


//memcpy( draw_plyr0, plyr0, sizeof( plyr0 ) ); 
//memcpy( draw_plyr1, plyr1, sizeof( plyr1 ) ); 


//move_piece( 0, 1, 0, 2 );

//move_piece( 0, 6, 1, 2 );

/*

//set bottom player pieces
brd[ 0 ].pce_typ = pt_rook;
brd[ 0 ].col = col;

brd[ 1 ].pce_typ = pt_knight;
brd[ 1 ].col = col;

brd[ 2 ].pce_typ = pt_bishop;
brd[ 2 ].col = col;

brd[ 3 ].pce_typ = pt_queen;
brd[ 3 ].col = col;

brd[ 4 ].pce_typ = pt_king;
brd[ 4 ].col = col;

brd[ 5 ].pce_typ = pt_bishop;
brd[ 5 ].col = col;

brd[ 6 ].pce_typ = pt_knight;
brd[ 6 ].col = col;

brd[ 7 ].pce_typ = pt_rook;
brd[ 7 ].col = col;

for( int i = 8; i < 16; i++ )
	{
	brd[ i ].pce_typ = pt_pawn;
	brd[ i ].col = col;
	}

if( white_bottom ) col = 0;
else col = 1;


//set top player pieces
for( int i = 48; i < 56; i++ )
	{
	brd[ i ].pce_typ = pt_pawn;
	brd[ i ].col = col;
	}

brd[ 56 ].pce_typ = pt_rook;
brd[ 56 ].col = col;

brd[ 57 ].pce_typ = pt_knight;
brd[ 57 ].col = col;

brd[ 58 ].pce_typ = pt_bishop;
brd[ 58 ].col = col;

brd[ 59 ].pce_typ = pt_queen;
brd[ 59 ].col = col;

brd[ 60 ].pce_typ = pt_king;
brd[ 60 ].col = col;

brd[ 61 ].pce_typ = pt_bishop;
brd[ 61 ].col = col;

brd[ 62 ].pce_typ = pt_knight;
brd[ 62 ].col = col;

brd[ 63 ].pce_typ = pt_rook;
brd[ 63 ].col = col;

*/
//set_piece( pt_pawn, 3, 4 );
}




/*
//given user a text coord move, convert to x1, y1 x2, y2 coords
bool chess_wnd::user_move_via_text( string s_in )
{
string s1;
char c0, c1;

printf("user: %s\n", s_in.c_str() );

mystr m1 = s_in;


m1.FindReplace( s1, " ", "", 0 );


if( s1.length() < 4 ) return 0;
c0 = s1[ 0 ];
c1 = s1[ 1 ];

c0 = c0 & 0xdf;

if( ( c0 < 'A' ) || ( c0 > 'H' ) ) return 0;
if( ( c1 < '1' ) || ( c1 > '8' ) ) return 0;

int coordx1 = c0 - 0x41;
int coordy1 = c1 - 0x31;


c0 = s1[ 2 ];
c1 = s1[ 3 ];

c0 = c0 & 0xdf;

if( ( c0 < 'A' ) || ( c0 > 'H' ) ) return 0;
if( ( c1 < '0' ) || ( c1 > '8' ) ) return 0;

int coordx2 = c0 - 0x41;
int coordy2 = c1 - 0x31;

printf( "xy: %d, %d   %d, %d\n", coordx1, coordy1, coordx2, coordy2 );

return move_piece( coordx1, coordy1, coordx2, coordy2 );

}
*/




//return an index of brd using coords
int chess_wnd::brd_idx( int coordx, int coordy )
{
if( ( coordx < 0 ) || ( coordx > 7 ) ) return -1;
if( ( coordy < 0 ) || ( coordy > 7 ) ) return -1;

int ret = coordx + coordy * 8;
}





bool chess_wnd::place_piece( vector<st_piece> &vv, int type, int x, int y, double val, bool sel )
{
st_piece o;

o.type = type;
o.x = x;
o.y = y;
o.val = val;
o.sel = sel;

vv.push_back( o );
}



/*
bool chess_wnd::place_piece( bool plyr, int type, int x, int y, double val, bool sel, int idx )
{
int ii;

st_piece *pp;
int *cnt;

if( plyr )
    {
    pp = plyr1;
    cnt = &piece_cnt1;
    ii = piece_cnt1;
    }
else{
    pp = plyr0;
    cnt = &piece_cnt0;
    ii = piece_cnt0;
    }


//if( ii >= max_pieces_per_side ) return 0;


pp[ idx ].type = type;
pp[ idx ].x = x;
pp[ idx ].y = y;
pp[ idx ].val = val;

pp[ idx ].sel = sel;

//(*cnt)++;
//else piece_cnt0++;

return 1;
}
*/





bool chess_wnd::get_draw_piece_via_idx( bool col, int idx, int &type, int &x, int &y, double &val, bool &sel )
{

if( idx >= max_pieces_per_side ) return 0;

st_piece *pp;
if( col == 0 ) pp = draw_plyr0;
else pp = draw_plyr1;

type = pp[ idx ].type;
x = pp[ idx ].x;
y = pp[ idx ].y;
val = pp[ idx ].val;
sel = pp[ idx ].sel;

return 1;
}









//given a screen pos (x,y),  calculate which checker coord the pos points to
//returns 1 if valid coord found, else 0
//coordx, coordy will hold the found checker coord, else they a set to -1
bool chess_wnd::find_coord( int x, int y, int &coordx, int &coordy )
{
int ll, tt;

coordx = -1;
coordy = -1;


tt = 0;
for( int yy = 0; yy < 8; yy++ )
	{
	ll = 0;
	for( int xx = 0; xx < 8; xx++ )
		{
		if( ( y > tt + checker_off_y ) && ( y < tt + checker_off_y + checker_y ) )
			{
			if( ( x > ll + checker_off_x ) && ( x < ll + checker_off_x + checker_x ) )
				{
				coordx = xx;
				coordy = 7 - yy;
				return 1;
				}
			}
		ll += checker_x;		
		}
	tt += checker_y;
	}

return 0;
}






//given user a text coord move, convert to x1, y1 x2, y2 coords
bool chess_wnd::user_move_via_text( string s_in )
{
string s1;
char c0, c1;

printf("user: %s\n", s_in.c_str() );

mystr m1 = s_in;


m1.FindReplace( s1, " ", "", 0 );


if( s1.length() < 4 ) return 0;
c0 = s1[ 0 ];
c1 = s1[ 1 ];

c0 = c0 & 0xdf;

if( ( c0 < 'A' ) || ( c0 > 'H' ) ) return 0;
if( ( c1 < '1' ) || ( c1 > '8' ) ) return 0;

int coordx1 = c0 - 0x41;
int coordy1 = c1 - 0x31;


c0 = s1[ 2 ];
c1 = s1[ 3 ];

c0 = c0 & 0xdf;

if( ( c0 < 'A' ) || ( c0 > 'H' ) ) return 0;
if( ( c1 < '0' ) || ( c1 > '8' ) ) return 0;

int coordx2 = c0 - 0x41;
int coordy2 = c1 - 0x31;

printf( "xy: %d, %d   %d, %d\n", coordx1, coordy1, coordx2, coordy2 );

return move_piece( coordx1, coordy1, coordx2, coordy2 );
}









int chess_wnd::find_piece_at_xy( int x, int y, bool &is_white )
{

for( int i = 0; i < vwht.size(); i++ )
	{
	if( vwht[ i ].type != pt_none )
		{
		if( vwht[ i ].x == x )
			{
			if( vwht[ i ].y == y )
				{
				is_white = 1;
				return i;                   //piece found
				}
			}
		}
	}

for( int i = 0; i < vblk.size(); i++ )
	{
	if( vblk[ i ].type != pt_none )
		{
		if( vblk[ i ].x == x )
			{
			if( vblk[ i ].y == y )
				{
				is_white = 0;
				return i;                   //piece found
				}
			}
		}
	}
return -1;

/*
is_plyr0 = 1;
for ( int i = 0; i < piece_cnt0; i++ )
    {
	if( plyr0[ i ].type != pt_none )
		{
		if( plyr0[ i ].x == x )
			{
			if( plyr0[ i ].y == y )
				{
				*plyr = plyr0;
				return i;                   //piece found
				}
			}
        }
    }


is_plyr0 = 0;
for ( int i = 0; i < piece_cnt1; i++ )
    {
	if( plyr1[ i ].type != pt_none )
		{
		if( plyr1[ i ].x == x )
			{
			if( plyr1[ i ].y == y )
				{
				*plyr = plyr1;
				return i;                   //piece found
				}
			}
		}
    }

return -1;
*/
}






bool chess_wnd::move_piece( int coordx1, int coordy1, int coordx2, int coordy2 )
{
if( ( coordx1 < 0 ) || (  coordx1 > 7 ) ) return 0;
if( ( coordy1 < 0 ) || (  coordy1 > 7 ) ) return 0;

if( ( coordx2 < 0 ) || (  coordx2 > 7 ) ) return 0;
if( ( coordy2 < 0 ) || (  coordy2 > 7 ) ) return 0;

bool is_white;
int idx = find_piece_at_xy( coordx1, coordy1, is_white );

if( idx >= 0 )
    {
	if( is_white )
		{
		vwht[ idx ].x = coordx2;
		vwht[ idx ].y = coordy2;
		vwht[ idx ].moved = 1;
		}
 	else{
		vblk[ idx ].x = coordx2;
		vblk[ idx ].y = coordy2;
		vblk[ idx ].moved = 1;
		}
	return 1;
    }

return 0;
}





void chess_wnd::setcolref( colref_chss col )
{
fl_color( col.r , col.g , col.b );
}








void chess_wnd::draw_board()
{
int ll,tt;
bool white = 1;

tt = 0;

for( int j = 0; j < 8; j++ )
	{
	ll = 0;
	white = !white;
	for( int i = 0; i < 8; i++ )
		{

		if( white ) fl_color( 119 , 162 , 109 );
		else  fl_color( 200 , 194 , 110 );

		fl_rectf( ll + checker_off_x , tt + checker_off_y , checker_x , checker_y );


		white = !white;
		ll += checker_x;
		}
	tt += checker_y;
	}

}









void chess_wnd::draw_grid()
{
int ll,tt;


fl_color( 0 , 0 , 0 );

ll = 0;
for( int i = 0; i < 9; i++ )
	{
	fl_line( ll + checker_off_x, checker_off_y, ll + checker_off_x, checker_off_y + 8 * checker_y );
	fl_line( ll + checker_off_x + 1, checker_off_y, ll + checker_off_x + 1, checker_off_y + 8 * checker_y );
	fl_line( ll + checker_off_x + 2, checker_off_y, ll + checker_off_x + 2, checker_off_y + 8 * checker_y );

	ll += checker_x;
	}

tt = 0;
for( int i = 0; i < 9; i++ )
	{
	fl_line(checker_off_x, tt + checker_off_y, checker_off_x + 8 * checker_x, tt + checker_off_y );
	fl_line(checker_off_x, tt + checker_off_y + 1, checker_off_x + 8 * checker_x, tt + checker_off_y + 1 );
	fl_line(checker_off_x, tt + checker_off_y + 2, checker_off_x + 8 * checker_x, tt + checker_off_y + 2 );

	tt += checker_x;
	}

}









void chess_wnd::draw_highlight_checker( int xx, int yy, int r, int g, int b )
{
int ll, tt;

tt = 0;

int x = xx * checker_x;
int y = ( 7 - yy ) * checker_y;

fl_color( r , g , b );

//fl_rectf( x + checker_off_x , y + checker_off_y , checker_x , checker_y );

return;

/*
for( int y = 0; y < 8; y++ )
	{
	ll = 0;
	for( int x = 0; x < 8; x++ )
		{
		if( y == yy  )
			{
			if( x == xx  )
				{
				fl_color( r , g , b );

				fl_rectf( ll + checker_off_x , tt + checker_off_y , checker_x , checker_y );
				}
			}
		ll += checker_x;		
		}
	tt += checker_y;
	}
*/
}











void chess_wnd::draw_highlight_checkers()
{
/*
for( int i = 0; i < poss_mve_cnt; i++ )
    {
    int x = poss_mve_x[ i ];
    int y = poss_mve_y[ i ];
    draw_highlight_checker( x, y, 255, 128, 128 );

    }
*/
}











void chess_wnd::draw_sqr_coords()
{
string s1;

char ch = '8';

int iF = fl_font();
int iS = fl_size();

fl_font( 4, 11 );

fl_color( 0 , 0 , 0 );

int x;
int y;
for( int i = 0; i < 8; i++ )		//draw vert chars
	{
	x = 22;
	y = checker_off_y + checker_y / 2 + checker_y * i + 5;
	strpf( s1, "%c", ch );
	fl_draw( s1.c_str(), x, y );
	
	x = checker_off_x + checker_x * 8 + 5 ;
	fl_draw( s1.c_str(), x, y );
	ch -= 1;
	}

ch = 'a';
for( int i = 0; i < 8; i++ )		//draw horiz chars
	{
	y = checker_off_y + 8 * checker_y + 13;
	x = checker_off_x + checker_x / 2 + checker_x * i ;
	strpf( s1, "%c", ch );
	fl_draw( s1.c_str(), x, y );
	
	y = checker_off_y - 4;
	fl_draw( s1.c_str(), x, y );
	ch += 1;
	}

fl_font( iF, iS );
}






void chess_wnd::draw_mouse_coords()
{
char cx = 'a' + mouse_coord_x;
char cy = '1' + mouse_coord_y;

string s1;

strpf( s1, "%c%c", cx, cy );

int y = checker_off_y;
int x = checker_off_x + 8 * checker_x + 70;

fl_color( 0 , 0 , 0 );
fl_draw( s1.c_str(), x, y );

}








//draw piece on board
void chess_wnd::draw_piece( bool plyr, int type, int x, int y )
{
int outlines = 7;
float x1, y1, x2, y2;
double scale_new = scale;
bool col;

int *p;

y = 7 - y;              //invert for screen plot

int yy = y * checker_y + checker_off_y + piece_off_y + checker_y / 2;

int xx = x * checker_x + checker_x / 2;

for( int s = 0; s <= outlines; s++ )
	{
    if( type == pt_none ) p = (int*)vec_none;
    if( type == pt_pawn ) p = (int*)vec_pawn;
    if( type == pt_bishop ) p = (int*)vec_bishop;
    if( type == pt_rook ) p = (int*)vec_rook;
    if( type == pt_knight ) p = (int*)vec_knight;
    if( type == pt_queen ) p = (int*)vec_queen;
    if( type == pt_king ) p = (int*)vec_king;

    int count = p[ 0 ];

    fl_line_style ( FL_SOLID, 2 );


    if( plyr == 0 ) col = col_plyr0;
    else col = col_plyr1;

    if( col ) fl_color( 255 , 252 , 212 );
    else fl_color( 0 , 0 , 0 );



    for ( int k = 2; k < count * 4; k += 4 )
        {
        x1 = (double)p[ k ] * scale_new;
        y1 = (double)p[ k + 1 ] * scale_new;

        x2 = (double)p[ k + 2 ] * scale_new;
        y2 = (double)p[ k + 3 ] * scale_new;

        fl_line( xx + checker_off_x + nearbyint( x1 ), yy - nearbyint( y1 ), xx + checker_off_x + nearbyint( x2 ), yy - nearbyint( y2 ) );
        }
 		
	scale_new = scale_new * 0.95;
	}

fl_line_style ( FL_SOLID, 1 );
}





void chess_wnd::draw_pieces()
{

for( int i = 0; i < vwht.size(); i++ )
	{
	int col = 0;
   
 	bool drw = 1;	
	if( vwht[i].sel && (!flash) ) drw = 0;
		
    if( drw ) draw_piece( 1, vwht[i].type, vwht[i].x, vwht[i].y );
	}


for( int i = 0; i < vblk.size(); i++ )
	{
	int col = 0;
   
 	bool drw = 1;	
	if( vblk[i].sel && (!flash) ) drw = 0;
		
    if( drw ) draw_piece( 0, vblk[i].type, vblk[i].x, vblk[i].y );
	}
}




void chess_wnd::draw_pieces2()
{
//draw_pieces2();
//return;
/*

for( int i = 0; i < piece_cnt0; i++ )
    {
    int col = 0;
    int type;
    int x;
    int y;
    double val;
	bool sel;
    if( get_draw_piece_via_idx( col, i, type, x, y, val, sel ) )
        {
		bool drw = 1;
		if( sel && (!flash) ) drw = 0;
		
        if( drw ) draw_piece( col, type, x, y );
        }

//printf("piece %d, type= %d, x= %d, y= %d\n", i, type, x, y );
    }





for( int i = 0; i < piece_cnt1; i++ )
    {
    int col = 1;
    int type;
    int x;
    int y;
    double val;

	bool sel;
    if( get_draw_piece_via_idx( col, i, type, x, y, val, sel ) )
        {
		bool drw = 1;
		if( sel && (!flash) ) drw = 0;
		
        if( drw ) draw_piece( col, type, x, y );
        }

//printf("piece %d, type= %d, x= %d, y= %d\n", i, type, x, y );
    }


return;
*/
}











/*

void chess_wnd::draw_pieces()
{
float x1, y1, x2, y2;

int *p;


int x;

double scale_new = scale;

bool white = 1;

int outlines = 7;

for( int s = 0; s <= outlines; s++ )
	{
	int y = checker_off_y + piece_off_y + checker_y / 2;
	
	for( int j = 7; j >= 0; j-- )
		{
		
		x = checker_x / 2;

		for( int i = 0; i < 8; i++ )
			{

			int pp = j * 8 + i;

			int piece_type = brd[ pp ].pce_typ;
			white = brd[ pp ].col;

			if( piece_type == pt_none ) p = (int*)vec_none;
			if( piece_type == pt_pawn ) p = (int*)vec_pawn;
			if( piece_type == pt_bishop ) p = (int*)vec_bishop;
			if( piece_type == pt_rook ) p = (int*)vec_rook;
			if( piece_type == pt_knight ) p = (int*)vec_knight;
			if( piece_type == pt_queen ) p = (int*)vec_queen;
			if( piece_type == pt_king ) p = (int*)vec_king;

			int count = p[ 0 ];

			fl_line_style ( FL_SOLID, 2 );


			if( white ) fl_color( 255 , 252 , 212 );
			else fl_color( 0 , 0 , 0 );

			if( s == outlines )						//outline?
				{
//				scale_new = scale;
//				if( white ) fl_color( 0 , 0 , 0 );
//				else fl_color( 255 , 252 , 212 );
				}
			else{
				}


			for ( int k = 2; k < count * 4; k += 4 )
				{
				x1 = (double)p[ k ] * scale_new;
				y1 = (double)p[ k + 1 ] * scale_new;

				x2 = (double)p[ k + 2 ] * scale_new;
				y2 = (double)p[ k + 3 ] * scale_new;

				fl_line( x + checker_off_x + nearbyint( x1 ), y - nearbyint( y1 ), x + checker_off_x + nearbyint( x2 ), y - nearbyint( y2 ) );
				}
			x += checker_x;
			}
		
		y += checker_y;
		}
	scale_new = scale_new * 0.95;
	}

fl_line_style ( FL_SOLID, 1 );
}

*/





void chess_wnd::draw()
{
int rght,bot;
string s1;


Fl_Double_Window::draw();


//printf("draw()\n");


memcpy( draw_plyr0, plyr0, sizeof( plyr0 ) ); 
memcpy( draw_plyr1, plyr1, sizeof( plyr1 ) ); 


//clear wnd

fl_color( FL_BACKGROUND_COLOR );

//setcolref( col_ch_bkgd );
//fl_rectf( 0 , menu_hei , w() - 170 , h() - menu_hei );

fl_rectf( checker_off_x, checker_off_y, brd_wid, brd_hei );

draw_board();
draw_highlight_checkers();

//draw_highlight_checker( 0, 0, 255, 128, 128 );
//draw_highlight_checker( 7, 7, 128, 128, 255 );

//show mouse checker
if( ( mouse_coord_x != -1 ) && ( mouse_coord_y != -1 ) )
	{
	draw_highlight_checker( mouse_coord_x, mouse_coord_y, 64, 128, 64 );
	draw_mouse_coords();
	}

draw_grid();


//draw 2 lines to show where the piece needs to be placed by user, uses lit leds as coords
if( show_piece_move_lines )
	{
	int onx = -1;
	int ony = -1;
	
	for( int i = 0; i < 8; i++ )
		{
		if( coord_ledx[i]->GetColIndex() ) onx = i;
		if( coord_ledy[i]->GetColIndex() ) ony = i;
		}
	
	if( ( onx > -1 ) && ( ony > -1) )
		{
		int crdx = onx;
		int crdy = ony;
		
		int x1 = checker_off_x;
		int y1 = checker_off_y + crdy * checker_y + checker_y / 2;

		int x2 = checker_off_x + 8 * checker_x;
		int y2 = y1;
		
		fl_color( 255 , 150 , 150 );
		fl_line_style ( FL_DASH, 1 );
		
		fl_line( x1, y1, x2, y2 );						//horiz


		x1 = checker_off_x + crdx * checker_x + checker_x / 2;
		y1 = checker_off_y;

		x2 = x1;
		y2 = checker_off_y + 8 * checker_y;

		fl_line( x1, y1, x2, y2 );						//vert
		
		
		//draw a cross over the req chess square
		x1 = checker_off_x + crdx * checker_x;
		y1 = checker_off_y + crdy * checker_y + checker_y / 2;
		
		x2 = checker_off_x + ( crdx + 1 ) * checker_x;
		y2 = y1;

		fl_color( 255 , 0 , 0 );
		fl_line_style ( FL_SOLID, 2 );
		fl_line( x1, y1, x2, y2 );						//horiz


		x1 = checker_off_x + crdx * checker_x + checker_x / 2;
		y1 = checker_off_y + crdy * checker_y;

		x2 = x1;
		y2 = checker_off_y + ( crdy + 1 ) * checker_y;

		fl_line( x1, y1, x2, y2 );						//vert
		}

	}

fl_line_style ( FL_SOLID, 1 );

draw_pieces();
draw_sqr_coords();



setcolref( col_ch_yel );

}





int chess_wnd::handle( int e )
{
bool need_redraw = 0;
bool dont_pass_on = 0;

int bDonePaste = 1;			//seems that fltk 1.3 needs this to be '1' like windows does

#ifdef compile_for_windows 
bDonePaste = 1;					//does not seem to be required for Windows, so nullify by setting to 1
#endif

if ( e == FL_PASTE )	//needed below code because on drag release the first FL_PASTE call does not have valid text as yet,
	{					//possibly because of a delay in X windows, so have used Fl:paste(..) to send another paste event and 
	if( bDonePaste == 0)	//this seems to work ok (does not seem to happen in Windows)
		{
		Fl::paste( *this, 0 );					//passing second var as 0 uses currently selected text, (not prev cut text)
//		printf("\nDropped1\n" );
		bDonePaste = 1;
		}
	else{
		bDonePaste = 0;
		string s = Fl::event_text();
		int len = Fl::event_length();
		printf("\nDropped Len=%d, Str=%s\n", len, s.c_str() );
		if( len )								//anything dropped/pasted?
			{
//			dropped_str = s;
			need_redraw = 1;
			}
		}
//	return 1;
	}

//printf("chess_wnd() - e: %d\n", e);

if (e == FL_DND_DRAG)
	{
	printf("\nDrag\n");
	return 1;
	}

if (e == FL_DND_ENTER)
	{
	printf("\nDrag Enter\n");
	return 1;
	}

if (e == FL_DND_RELEASE)
	{
	printf("\nDrag Release\n");
	return 1;
	}

if ( e == FL_MOVE )
	{
	mousex = Fl::event_x();
	mousey = Fl::event_y();
	
	dont_pass_on = 0;
	}

if ( e == FL_PUSH )
	{
	if(Fl::event_button()==1)
		{
		left_button = 1;
		left_pressed = 0;
		
		if( find_coord( Fl::event_x(), Fl::event_y(), mouse_coord_x, mouse_coord_y ) )
			{
			left_pressed = 1;
//			printf("mouse_coord_x: %d, %d\n", mouse_coord_x, mouse_coord_y);
			

			bool is_white;
			
			int sel_crdx, sel_crdy;
			if( find_selected( sel_crdx, sel_crdy, is_white ) > -1 )
				{
				if( ( sel_crdx == mouse_coord_x ) && ( sel_crdy == mouse_coord_y ) )	//clicked on a sel piece?
					{
//					exit(0);
//				move_piece( int coordx1, int coordy1, int coordx2, int coordy2 )
					deselect_all();
					}
				else{
					int idx;
					delete_piece( mouse_coord_x, mouse_coord_y, idx );						//taking a piece?
					move_piece( sel_crdx, sel_crdy, mouse_coord_x, mouse_coord_y );
					deselect_all();
//					toggle_select_piece( mouse_coord_x, mouse_coord_y );
					}
				}
			else{
				toggle_select_piece( mouse_coord_x, mouse_coord_y );
				}
//			delete_piece( mouse_coord_x, mouse_coord_y );

			need_redraw = 1;
			dont_pass_on = 1;
			}
		}

	if(Fl::event_button()==3)
		{
		if( find_coord( Fl::event_x(), Fl::event_y(), mouse_coord_x, mouse_coord_y ) )
			{
			flm_menu_popup_chess->popup();
			}
		need_redraw = 1;
		dont_pass_on = 1;
		}
	}
	

if ( e == FL_RELEASE )
	{
	left_pressed = 0;
	if(Fl::event_button()==1 )
		{
		left_button = 0;

		need_redraw = 1;
		dont_pass_on = 1;
		}
	}


if ( ( e == FL_KEYDOWN ) || ( e == FL_SHORTCUT ) )					//key pressed?
	{
	int key = Fl::event_key();
//	if( ( key == FL_Control_L ) || (  key == FL_Control_R ) ) ctrl_key = 1;
	
	need_redraw = 1;
	}


if ( e == FL_KEYUP )												//key release?
	{
	int key = Fl::event_key();
	
//	if( ( key == FL_Control_L ) || ( key == FL_Control_R ) ) ctrl_key = 0;

	need_redraw = 1;
	}


if ( e == FL_MOUSEWHEEL )
	{
//	mousewheel = Fl::event_dy();
//deselect_all();
//	exit(0);
	
	need_redraw = 1;
	}
	
if ( need_redraw ) redraw();

if( dont_pass_on ) return 1;

return Fl_Double_Window::handle(e);
}


//-----------------------------------------------------------------------------

