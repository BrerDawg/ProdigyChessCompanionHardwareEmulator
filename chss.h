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

//chss.h v1.01

#ifndef chss_h
#define chss_h



#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <string>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <math.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Int_Input.H>

#include "globals.h"
#include "GCProfile.h"
#include "seg7.h"
#include "GCLed.h"

using namespace std;



#define cn_max_mve_cnt 100
#define max_pieces_per_side 16

struct colref_chss
{
int r, g, b;
};

static colref_chss col_ch_bkgd =   { 64,    64,    64 	};
static colref_chss col_ch_yel = 	{ 255,   255,   0 	};
static colref_chss col_ch_red =	{ 255,   0,     0	};
static colref_chss col_ch_mag =	{ 255,   0,     255	};
static colref_chss col_ch_wht =	{ 255,   255,   255	};








static int vec_none[ 1 ][ 2 ]=
{
0,0
};

static int vec_pawn[25][2]=
{
12,0,		//number of lines
-10,-15,
10,-15,
-10,-15,
-3,-7,
10,-15,
3,-7,
-3,-7,
-3,0,
3,-7,
3,0,
-3,0,
-5,3,
3,0,
5,3,
-5,3,
-5,7,
5,3,
5,7,
-5,7,
-3,10,
5,7,
3,10,
-3,10,
3,10,

};




static int vec_bishop[25][2]=
{
11,0,		//number of lines
-10,-15,
10,-15,
-10,-15,
-3,-7,
10,-15,
3,-7,
-3,-7,
-3,5,
3,-7,
3,5,
-3,5,
-5,8,
3,5,
5,9,
-5,8,
-2,14,
-2,14,
0,10,
0,10,
3,18,
3,18,
5,8
};




static int vec_rook[37][2]=
{
18,0,		//number of lines
-10,-15,
10,-15,
-10,-15,
-5,-10,
10,-15,
5,-10,
-5,-10,
-5,5,
5,-10,
5,5,
-5,5,
-8,10,
5,5,
8,10,
-8,10,
-8,15,
8,10,
8,15,
-8,15,
-5,15,
8,15,
5,15,
-5,15,
-5,10,
5,15,
5,10,
-5,10,
-2,10,
5,10,
2,10,
-2,10,
-2,15,
2,10,
2,15,
-2,15,
2,15,
};









static int vec_knight[37][2]=
{
14,0,		//number of lines
-10,-15,
10,-15,
-10,-15,
-3,-5,
-3,-5,
-7,0,
-7,0,
-7,10,
-7,10,
-5,14,
-5,14,
5,19,
5,19,
5,15,
5,15,
10,10,
10,10,
15,0,
15,0,
15,-5,
15,-5,
10,-5,
10,-5,
5,0,
5,0,
5,-10,
5,-10,
10,-15,
};



static int vec_queen[37][2]=
{
15,0,		//number of lines
-10,-15,
10,-15,
-10,-15,
-5,-10,
10,-15,
5,-10,
-5,-10,
-5,10,
5,-10,
5,10,
-5,10,
-9,15,
5,10,
9,15,
-9,15,
-9,20,
9,15,
9,20,
-9,20,
-5,15,
9,20,
5,15,
-5,15,
-3,20,
5,15,
3,20,
-3,20,
0,15,
3,20,
-1,15,
};









static int vec_king[37][2]=
{
15,0,		//number of lines
-10,-15,
10,-15,
-10,-15,
-5,-10,
10,-15,
5,-10,
-5,-10,
-5,7,
5,-10,
5,7,
-5,7,
-10,7,
5,7,
10,7,
-10,7,
-5,7,
-10,7,
-10,15,
10,7,
10,15,
-10,15,
-5,15,
10,15,
5,15,
-5,15,
-5,20,
5,15,
5,20,
-5,20,
5,20,

};





enum en_piece_type
{
pt_none,
pt_pawn,
pt_bishop,
pt_rook,
pt_knight,
pt_queen,
pt_king
};



struct st_piece_val
{
double val;
};




//would only be 16 pieces in length for lowest dimension
struct st_piece
{
int type;
double val;
int x, y;
bool moved;                                 //has piece been moved from its starting pos, usefull for detecting if pawn allowed to move by 2
bool sel;
};






struct brd_tag
{
int pce_typ;
bool plyr;

} static brd[ 64 ];





//-----------------------------------------------------------------------------

struct button_tag
{
string sname;
char sztooltip[255];
int keycode;
int x1, y1, x2, y2;
bool text_button;				//set if button is a non image based, it wont have its dimensions scaled
int id0, id1;					//user defined ids, eg: for button matrix identification
};





class jpg_cpanel : public Fl_Widget
{
private:
int butt_pressed;
int mousewheel;
bool ctrl_key, shift_key;
void (*left_click_p_cb)( void *obj_in, void *args_in ); 
void *cb_obj;
void *left_click_cb_args;


bool entered;
int mousex, mousey;
int lb_x, lb_y;
int rb_x, rb_y;

public:
Fl_JPEG_Image *jpg_panel;
bool left_button, right_button;
vector<button_tag>vbutt;
int hov_idx;
int id0, id1;

public:
jpg_cpanel( int xx, int yy, int wid, int hei, const char *label );
bool init( string ss );
bool set_image( string ss );
void add_image_bounding_button( button_tag o );
void set_left_click_cb( void (*p_cb)( void*, void* ), void *args );


private:
void draw();
int handle( int e );
int do_mouse_move( int xx, int yy );
int do_butt_left();
int do_butt_right();
//void process_button( string ss );

};

//-----------------------------------------------------------------------------







class chess_wnd : public Fl_Double_Window
{
private:											//private var
Fl_Menu_Button *flm_menu_popup_chess;
int dir_plyr0;
int dir_plyr1;
bool bottom_plyr;                               //which player is at bottom
bool col_plyr0;                                 //player colour
bool col_plyr1;                                 //player colour

bool left_button;
int instance;
Fl_Color col_bkgd;

int menu_hei;
int checker_x;
int checker_y;
int checker_off_x;
int checker_off_y;
int piece_off_y;
float scale;

bool comp_comp;
bool comp_mve;

int tick_cnt;
bool flash;


//st_piece plyr_mve0[ cn_max_mve_cnt ][ max_pieces_per_side ];          //holds player pieces
//st_piece plyr_mve1[ cn_max_mve_cnt ][ max_pieces_per_side ];          //holds player pieces


st_piece draw_plyr0[ max_pieces_per_side ];                          	//hold player pieces used for drawing brd
st_piece draw_plyr1[ max_pieces_per_side ]; 


//int sel_crdx, sel_crdy;

public:
Fl_Menu_Bar *menu;
Fl_Int_Input *fi_usec;
int brd_wid, brd_hei;
vector<st_piece> vwht, vblk;

int piece_cnt0;
int piece_cnt1;
st_piece plyr0[ max_pieces_per_side ];          //holds player pieces
st_piece plyr1[ max_pieces_per_side ];          //holds player pieces

seg7_wdg *wdg_seg0;
seg7_wdg *wdg_seg1;
seg7_wdg *wdg_seg2;
seg7_wdg *wdg_seg3;


bool left_pressed;
GCLed* coord_ledx[8];
GCLed* coord_ledy[8];
int mousex, mousey;
int mouse_coord_x;
int mouse_coord_y;
bool show_piece_move_lines;						//draw 2 lines to show where the piece needs to be placed by user, uses the lit leds as coords
int usec;
Fl_Box *bx_status;
bool paused;

private:

void setcolref( colref_chss col );

void draw();
void draw_board();
void draw_grid();
void draw_highlight_checker( int xx, int yy, int r, int g, int b );
void draw_highlight_checkers();
void draw_sqr_coords( );
void draw_mouse_coords();
void draw_piece( bool plyr, int type, int x, int y );
void draw_pieces();
void draw_pieces2();
int handle( int e );

void init();
int brd_idx( int coordx, int coordy );
//bool set_piece( ulli bb, int coordx, int coordy );

bool move_piece( int coordx1, int coordy1, int coordx2, int coordy2 );
//bool mve_gen( int cur_x, int cur_y, int new_x, int new_y );
//bool chk_valid_pieces( int &x, int &y  );
//flts player_valuation( bool plyr );
//bool get_piece_via_idx( bool plyr, int idx, int &type, int &x, int &y, flts &val );
bool get_draw_piece_via_idx( bool col, int idx, int &type, int &x, int &y, double &val, bool &sel );
//bool move_gen( bool plyr, int idx );
//int checker_status( int x, int y );
int find_piece_at_xy( int x, int y, bool &is_white );


public:											//public var
chess_wnd( int xx, int yy, int wid, int hei, const char *label );
~chess_wnd();
//void set_instance_id( int id );
void new_game();
bool find_coord( int x, int y, int &coordx, int &coordy );
//bool place_piece( bool plyr, int type, int x, int y, double val, bool sel, int idx );
bool place_piece( vector<st_piece> &oo, int type, int x, int y, double val, bool sel );

bool user_move_via_text( string s1 );
//void game_to_str( string &ss );
//void str_to_game( string ss );
//bool open_game( string fname );
//void test1();
//void test2();
void set_led_x( int crdx, bool on );
void set_led_y( int crdy, bool on );
void tick();
bool select_piece( int coordx1, int coordy1, bool sel );
bool toggle_select_piece( int crdx, int crdy );
bool deselect_all();
int find_selected( int &crdx, int &crdy, bool &is_plyr0 );
bool delete_piece( int crdx, int crdy, int &idx );
};



#endif
