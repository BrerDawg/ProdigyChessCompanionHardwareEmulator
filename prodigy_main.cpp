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

//prodigy_main.cpp

//v1.01    20-aug-2017			//
//v1.02    01-nov-2017			// more debug console features
//v1.03    30-oct-2018         	// !!! MAKE SURE you call 'mos6502::init_opcodes()', straight after calling its constructor,
								// connected chess brd and added bording leds, added prodigy control panel using: 'jpg_cpanel'
								// improved thrd handling, improved debug address filtering


//mingw needs library -lmsvcp60 for mcrtomb type calls:
//from Makefile.win that dev-c++ uses, these params were used to compile and link:
//LIBS =  -L"C:/Dev-Cpp/lib" -lmsvcp60 -lfltk_images -lfltk_jpeg -mwindows -lfltk -lole32 -luuid -lcomctl32 -lwsock32 -lm 
//INCS =  -I"C:/Dev-Cpp/include" 
//CXXINCS =  -I"C:/Dev-Cpp/lib/gcc/mingw32/3.4.2/include"  -I"C:/Dev-Cpp/include/c++/3.4.2/backward"  -I"C:/Dev-Cpp/include/c++/3.4.2/mingw32"  -I"C:/Dev-Cpp/include/c++/3.4.2"  -I"C:/Dev-Cpp/include" 
//CXXFLAGS = $(CXXINCS)  
//CFLAGS = $(INCS) -DWIN32 -mms-bitfields 

//#define compile_for_windows	//!!!!!!!!!!! uncomment for a dos/windows compile

//use -mconsole in make's LIBS shown above for windows to use the dos console, rather a attaching a console











//ACI Prodigy Destiny Chess Portable Game

//blog about writing mame driver
//https://frakaday.blogspot.com.au/2016/11/the-prodigy-chess-computer-part-iv.html

//German Prodigy site with images
//https://www.schach-computer.info/wiki/index.php?title=Chafitz_Destiny

//aci prodigy destiny chess operational manual
//http://alain.zanchetta.free.fr/docs/AppliedConcepts/ProdigyUS.pdf
//http://www.google.com.au/url?sa=t&rct=j&q=&esrc=s&source=web&cd=7&cad=rja&uact=8&ved=0ahUKEwi4n-j656_WAhXDopQKHYzoBPgQFghSMAY&url=http%3A%2F%2Falain.zanchetta.free.fr%2Fdocs%2FAppliedConcepts%2FProdigyUS.pdf&usg=AFQjCNFMXqbSGwDqJcD_HDB0C9qkwDrZ3g

// Prodigy mame rom
//http://www.planetemu.net/rom/mame-roms/prodigy-1

//6502 opcodes
//https://sites.google.com/site/6502asembly/6502-instruction-set/sty

//6502 emulator c code
//https://github.com/gianlucag/mos6502

//6502 Disassembler
//http://www.e-tradition.net/bytes/6502/disassembler.html

//How to create a hex dump of file containing only the hex characters without spaces in bash?
//https://stackoverflow.com/questions/2614764/how-to-create-a-hex-dump-of-file-containing-only-the-hex-characters-without-spac


//ACI Prodigy Destiny Chess Portable Game Hardware

//mc6502 cpu and mc6522 via running at 2 MHz
//Note: below are only assumptions, and may have errors as they were deduced over some time....

//ROM at: 0x6000-0x7fff, R2912/8KB (image also at: 0xe000-0xffff for boot vector: 0xfffc[0x60b7] and IRQ vector: 0xfffe[0x79f8] )
//RAM at: 0x0000-0x07ff, M58725P/2KB


//
// addr: 2000       ORB   b7-b0 VIA 4 outputs  PB3-PB0 SN74145 [bcd to decimal:0-9]           bcd: 0    1    2    3    4    5    6    7
//                              chess piece pressure pad 'alpha' values (as in lowest row white ): A1,  B1,  C1,  D1,  E1,  F1,  G1,  H1

// addr: 2001       ORA   b7-b0 VIA 8 inputs from chess piece pressure matrix pads, gleaned from pcb photographs.
//                                                                               ORA bit: PA0  PA1  PA2  PA3  PA4  PA5  PA6  PA7
//                  ORA   chess piece pressure pad 'num' values (as in left rook column): A1,  A2,  A3,  A4,  A5,  A6,  A7,  A8
	


//PB3-PB0 are bits 3,2,1,0 (via SN74145) to select 0 of 5 common anodes for the 4x 7 seg displays and 16x chess piece coord leds, bordering the brd ( x: alpha and y: numerics)
//CB1, CB2 are sclk/sdata (via SN74164) to drive segment cathodes of 7 seg displays (segment wiring VIA/SN74164/SN75491 is segA->segG, VIA ShiftReg 0x200a bits 0->7, positive logic, 1 = led on )

//200a is ShiftReg,  writing:  0x38 is 7seg char: 'L', 0x3f is 7seg char: '0'
								

// ---6522 VIA init sequence---
// addr_wr: 2003, 00 - DDRA - Data Direction A, pins PA0-PA7 programmed as inputs(8)
// addr_wr: 2002, 8f - DDRB - Data Direction B, pins PB4-PB6 programmed as inputs(3), pins PB7, PB3-PB0 as outputs(5)
// addr_wr: 200c, e0 - PCR  - Peripheral Control Register, CB2 out held high(serial out), CB1 irq on falling edg, CA2 irq neg edg, CA1 ints on falling edg
// addr_wr: 200b, 58 - ACR  - Auxilary Control Register, Timer1 free run mode, PB7 dis, Timer1 interval, SR shifts out under sys clock, PA and PB latches disabled
//                            ACR = 76543210
//                                  01011000

// addr_wr: 2009, 58 - T2C-H - Timer 2 High Order Counter value
// addr_wr: 2004, ff - T1C-L - Timer 1 Low Order Latches
// addr_wr: 200e, a0 - IER   - Interrupt Enable Register, Timer 2 Interrupts enabled
//                             IER = 76543210
//                                   10100000


//--------------------Trig IRQ------------------------
// 6522 VIA IRQ sequence (may be incorrect)


// addr_rd: 2008, 00	T2L-L		(possibly clears irq)
// addr_wr: 2008, 8d	T2L-L
// addr_wr: 2009, 20	T2C-H
// addr_wr: 200a, 00	shift reg	(seven seg?)
// addr_wr: 2000, 09 	ORB
// addr_rd: 2001, 00	ORA
// addr_rd: 2000, 09 	ORB
// addr_wr: 2000, 08 	ORB
// addr_rd: 2001, 00	ORA
// addr_rd: 2000, 08 	ORB
// addr_wr: 2000, 07 	ORB 
// addr_rd: 2001, 00	ORA 
// addr_wr: 200a, 00	shift reg
// addr_wr: 2000, 05 	ORB



//7 seg led details, led pattern bytes are written to VIA 0x200a shift register, this feeds SN74164/SN75491 led drivers, positive logic, 1 means led on
//
//                a  (0x01)
//              ____
//    (0x20) f /   / b   (0x02)
//            /-g-/ 
// dpl *   e /___/ c   * dpr
//             d
//           (0x08)

// c is 0x04
// e is 0x10
// g is 0x40
//dpl, dpr are not used - from what I can tell



//piezo speaker connected to VIA PB7, there may be other beeps also
//addr_wr: 200b<-58 - VIA PB7 disabled - from timer1
//addr_wr: 200b<-d8 - VIA PB7 enabled - driven by timer1 sqr wave

//addr_wr: 2005<-04 - VIA timer1 sound pitch low  (piece position changed by user beep sound)
//addr_wr: 2005<-02 - VIA timer1 sound pitch high (Prodigy finished thinking beep)


#include "prodigy_main.h"


//asm("int3");						//usefull to trigger debugger






//global objs
dble_wnd *wndMain;
gclog *logr;						//don't use 'log' it causes immediate crash			
vector<string>vlog;
gcthrd *thrd_cpu = 0;

csl_wdg *wdg_csl = 0;
csl_wdg *wdg_csl2 = 0;
csl_wdg *wdg_csl3 = 0;
seg7_wdg *wdg_seg0 = 0;
seg7_wdg *wdg_seg1 = 0;
seg7_wdg *wdg_seg2 = 0;
seg7_wdg *wdg_seg3 = 0;
chess_wnd *ch_wnd = 0;
mtx_tag mtx;								//structure used for mutex access to vars



PrefWnd* pref_wnd=0;
PrefWnd* pref_wnd2=0;
PrefWnd* font_pref_wnd=0;
Fl_Text_Buffer *tb_csl = 0;
Fl_Text_Editor *te_csl = 0;
Fl_Input *fi_gcpipe_app;
Fl_Input *fi_gcpipe_out;
Fl_Input *fi_gcpipe_quit;

//menu
Fl_Menu_Bar* meMain;


//global vars
string csIniFilename;
int iBorderWidth;
int iBorderHeight;
int gi0,gi1,gi2,gi3,gi4;
double gd0,gd1,gd2,gd3,gd4;
string app_path;						//path this app resides on
string dir_seperator="\\";				//assume dos\windows folder directory seperator
string sglobal;
string sg_col1, sg_col2;
int gi;
int mode;
int use_mutex;
int font_num = cnFontEditor;
int font_size = cnSizeEditor;
//static Fl_Font extra_font;
unsigned long long int ns_tim_start1;	//a ns timer start val
double perf_period;						//used for windows timings in timing_start( ), timing_stop( )
string fav_editor;


int iAppExistDontRun = 1;	//if set, app probably exist and user doesn't want to run another
							//set it to assume we don't want to run another incase user closes
							//wnd without pressing either 'Run Anyway' or 'Don't Run, Exit' buttons


#ifndef compile_for_windows
Display *gDisp;
#endif


string sdebug;							//holds all cslpf output 
bool my_verbose = 1;


bool bfirst_run = 1;
int vt100_count = 0;

string skey;

int timer_div = 2;
int pause_state = 2;					//1 is a req to pause (gui thrd), 2 is actual paused confirmed by 'thrd_cpu_cb()'
int run_step_size = 1;
int run_step_size_set = 0;

bool need_update = 0;
int bp_addr = -1;
bool bp_set = 0;
bool single_step = 0;

string slast_save_game;

//function prototypes
void LoadSettings(string csIniFilename);
void SaveSettings(string csIniFilename);
int CheckInstanceExists(string csAppName);
void open_editor( string fname );
void thrd_cpu_start();
void thrd_cpu_stop();
void cpu_boot();
int cpu_run( unsigned int steps );
int RunShell(string sin);
bool set_pause_state_to( int state );
bool go_into_pause_state();
void beep( int pitch, int count, int length );





//callbacks
void cslpf( const char *fmt,... );
void cb_wndmain(Fl_Widget*, void* v);
void cb_btAbout(Fl_Widget *, void *);
void cb_btOpen(Fl_Widget *, void *);
void cb_btSave(Fl_Widget *, void *);
void cb_btQuit(Fl_Widget *, void *);
void cb_btTest(Fl_Widget *, void *);
void cb_timer1(void *);
void cb_pref(Fl_Widget*w, void* v);
void cb_pref2(Fl_Widget*w, void* v);
void cb_font_pref(Fl_Widget*w, void* v);
void cb_open_folder(Fl_Widget *, void *);
void cb_open_file(Fl_Widget *, void *);
void cb_save_file(Fl_Widget *, void *);
void log_callback( string s );

void cb_user1( void *o, int row, int ctrl );
void cb_user2( void *o, int row, int ctrl );
void cb_user3( void *o, int row, int ctrl );
void cb_bt_help( Fl_Widget *, void *);


enum
{
enNone,
enStop,
enRun
};



struct sThreadTag
{
int iKillThread;
int iThreadFinished;
};


sThreadTag sThrd1;


//linux code
#ifndef compile_for_windows
void* Thread1 (void* pParams);
pthread_t thread1_id;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
#endif

//windows code
#ifdef compile_for_windows
DWORD WINAPI DosThread(void* lpData);
HANDLE h_mutex1; 
PROCESS_INFORMATION processInformation;
#endif



//--------------------- Main Menu --------------------------
Fl_Menu_Item menuitems[] =
{
	{ "&File",              0, 0, 0, FL_SUBMENU },
		{ "&LoadGame..", FL_CTRL + 'o'	, (Fl_Callback *)cb_open_file, 0 },
		{ "&SaveGame..", FL_CTRL + 's'	, (Fl_Callback *)cb_save_file, 0, FL_MENU_DIVIDER },
		{ "E&xit", FL_CTRL + 'q'	, (Fl_Callback *)cb_btQuit, 0 },
		{ 0 },

	{ "&Help", 0, 0, 0, FL_SUBMENU },
		{ "&About", 0				, (Fl_Callback *)cb_btAbout, 0 },
		{ "&Show help.txt...", 0				, (Fl_Callback *)cb_bt_help, 0 },
		{ 0 },


	{ 0 }
};
//------------------need_redraw----------------------------------------




//----------------------------------------------------------

csl_wdg::csl_wdg( int x,int y,int w, int h,const char *label ) : Fl_Widget( x, y, w, h, label )
{
scroll = 0;

col_bkgd.r = 64;
col_bkgd.g = 64;
col_bkgd.b = 64;

col_text1.r = 200;
col_text1.g = 200;
col_text1.b = 0;

curs_flash_count = 0;
curs_flash = 0;
char_cursor = '_';
save_cursx = save_cursy = 0;

fonttype = 4;
fontsize = 14;
line_offs = 14;

chars_horiz = 80;
chars_vert = 26;
cursx = cursy = 0;

keypressed = 0;

buf = 0;
set_char_dimensions( chars_horiz, chars_vert );

have_focus = 0;
show_have_focus = 1;
}




void csl_wdg::add_hscrollbar()
{
if( scroll == 0 ) 
	{
	scroll = new Fl_Scrollbar( x() + w(), y(), 12, h(), "");
	scroll->minimum( 0 );
	scroll->maximum( 100 );
	}
}


csl_wdg::~csl_wdg()
{
if ( buf != 0 ) delete buf;
}







bool csl_wdg::set_char_buf( int char_x, int char_y, char cc )
{
if( char_x < 0 ) return 0;
if( char_y < 0 ) return 0;
if( char_x >= chars_horiz ) return 0;
if( char_y >= chars_vert ) return 0;

buf[ char_y * chars_horiz + char_x ] = cc;

return 1;
}








bool csl_wdg::get_char_buf( int char_x, int char_y, char &cc )
{
if( char_x < 0 ) return 0;
if( char_y < 0 ) return 0;
if( char_x >= chars_horiz ) return 0;
if( char_y >= chars_vert ) return 0;

cc = buf[ char_y * chars_horiz + char_x ];

return 1;
}







void csl_wdg::set_char_dimensions( int char_x, int char_y )
{
if ( buf != 0 ) delete buf;


if( char_x < 1 ) char_x = 0;
if( char_y < 1 ) char_y = 0;

if( char_x > 2047 ) char_x = 2047;
if( char_y > 2047 ) char_y = 2047;

chars_horiz = char_x;
chars_vert = char_y;

buf = new int [ chars_horiz * chars_vert ];

clear();
}











void csl_wdg::setcolref( colref col )
{
fl_color( col.r , col.g , col.b );
}






void csl_wdg::force_cursor()
{
set_char_buf( cursx, cursy, char_cursor );
}





void csl_wdg::save_cursor()
{
save_cursx = cursx;
save_cursy = cursy;
}





void csl_wdg::restore_cursor()
{
cursx = save_cursx;
cursy = save_cursy;
}



void csl_wdg::tick( int ratio )
{
curs_flash_count++;
if( curs_flash_count >= ratio )
	{
	curs_flash = !curs_flash;
	curs_flash_count = 0;
	}

if( curs_flash )
	{
	set_char_buf( cursx, cursy, char_cursor );
	}
else{
	set_char_buf( cursx, cursy, ' ' );
	}

redraw();
}









void csl_wdg::clear()
{
for( int yy = 0; yy < chars_vert; yy++ )
	{
	for( int xx = 0; xx < chars_horiz; xx++ )
		{
		set_char_buf( xx, yy, ' ' );
		}
	}

cursx = cursy = 0 ;

redraw();
}







void csl_wdg::clear_line()
{
for( int xx = 0; xx < chars_horiz; xx++ )
	{
	set_char_buf( xx, cursy, ' ' );
	}


cursx = 0 ;

redraw();
}





void csl_wdg::backspace()
{
set_char_buf( cursx, cursy, ' ' );

cursx--;
if( cursx < 0 ) cursx++;
set_char_buf( cursx, cursy, char_cursor );

//if( cursy >= chars_vert )  cursy--;

force_cursor();
redraw();
}





bool csl_wdg::scroll_up()
{

for( int yy = 0; yy < chars_vert; yy++ )
	{
	for( int xx = 0; xx < chars_horiz; xx++ )
		{
		char cc;
		if( yy < chars_vert ) get_char_buf( xx, yy + 1, cc );
		else cc = ' ';
		set_char_buf( xx, yy, cc );
		}
	}

return 1;
}




void csl_wdg::add( char cc )
{
string s1;

s1 = cc;

add( s1 );

//cursx++;
//if( cursx >= chars_horiz ) cursx--;

force_cursor();

redraw();
}







void csl_wdg::add( string ss )
{
for( int i = 0; i < ss.length(); i++ )
	{
	set_char_buf( cursx, cursy, ' ');				//clear possible cursor incase currently visible
	if( ss[ i ] == '\n' )
		{
		cursy++;
		if( cursy >= chars_vert )
			{
			scroll_up();
			cursx = 0;
			cursy--;
			}
		else{
			cursx = 0;
			}
		}
	else{
		set_char_buf( cursx, cursy, ss[ i ] );
		cursx++;
		if( cursx >= chars_horiz ) { cursx--; }
//		if( cursy >= chars_vert ) cursy--;
		}
	}


redraw();
}













void csl_wdg::move_cursor( int char_x, int char_y )
{
if( char_x < 1 ) char_x = 0;
if( char_y < 1 ) char_y = 0;

if( char_x > 2047 ) char_x = 2047;
if( char_y > 2047 ) char_y = 2047;

cursx = char_x;
cursy = char_y;
}






void csl_wdg::draw()
{

//Fl_Widget::draw();
//return;

int rght,bot;
string s1;
mystr m1;

int xx = x();
int yy = y();


int iF = fl_font();
int iS = fl_size();

fl_font( fonttype, fontsize );



//clear wnd
setcolref( col_bkgd );
fl_rectf( xx , yy , w() , h() );


if( show_have_focus )
	{
	fl_color( 200, 80, 80 );
	fl_line_style( FL_SOLID, 2 );
	if( have_focus ) fl_rect( xx + 2 , yy + 2 , w() - 3 , h() - 3 );
	fl_line_style( FL_SOLID, 0 );
	}


setcolref( col_text1 );




sbuf = "";

for( int yy = 0; yy < chars_vert; yy++ )
	{
	for( int xx = 0; xx < chars_horiz; xx++ )
		{
		char cc;
		get_char_buf( xx, yy, cc );
		sbuf += cc;
		}
	sbuf += '\n';
	}




m1 = sbuf;
vector<string> vstr;
m1.LoadVectorStrings( vstr, '\n' );

int line_offs = 14;

int yyy = 14;

int i = 0;
if( vstr.size() >= chars_vert ) i = vstr.size() - chars_vert;		//skip first line(s) if need be

for( ; i < vstr.size(); i++ )
	{
	strpf( s1, "%s", vstr[ i ].c_str() );
	fl_draw( s1.c_str(), xx + 5, yy + yyy );

	yyy += line_offs;
	}


//setcolref( col_mag );
//fl_line( 50, 70, 80 , h() );

fl_font( iF, iS );
}






bool csl_wdg::kbhit()
{
if( keypressed )
	{
	keypressed = 0;
	return 1;
	}

return 0;
}






int csl_wdg::handle( int e )
{
bool need_redraw = 0;
bool dont_pass_on = 0;


if ( e == FL_PASTE )	//needed below code because on drag release the first FL_PASTE call does not have valid text as yet,
	{					//possibly because of a delay in X windows, so have used Fl:paste(..) to send another paste event and 
	string s = Fl::event_text();
	int len = Fl::event_length();
	printf("\nDropped Len=%d, Str=%s\n", len, s.c_str() );
	if( len )								//anything dropped/pasted?
		{
		need_redraw = 1;
		}
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


if ( e == FL_FOCUS )
	{
	have_focus = 1;
	need_redraw = 1;
	dont_pass_on = 1;
	}


if ( e == FL_UNFOCUS )
	{
	have_focus = 0;
	need_redraw = 1;
	dont_pass_on = 1;
	}


if ( e == FL_PUSH )
	{
	take_focus();
	if( Fl::event_button() == 1 )
		{
//		left_button = 1;
		need_redraw = 1;
		}
	dont_pass_on = 1;
	}

if ( e == FL_RELEASE )
	{
	
	if( Fl::event_button() == 1 )
		{
//		left_button = 0;
		need_redraw = 1;
		}
	dont_pass_on = 1;
	}


if ( ( e == FL_KEYDOWN ) || ( e == FL_SHORTCUT ) )					//key pressed?
	{
	int key = Fl::event_key();
	
	keycode = key;
	keypressed = 1;

//	if( ( key == FL_Control_L ) || (  key == FL_Control_R ) ) ctrl_key = 1;
	
	need_redraw = 1;
	dont_pass_on = 1;
	}


if ( e == FL_KEYUP )												//key release?
	{
	int key = Fl::event_key();
	
//	if( ( key == FL_Control_L ) || ( key == FL_Control_R ) ) ctrl_key = 0;

	need_redraw = 1;
	dont_pass_on = 1;
	}


if ( e == FL_MOUSEWHEEL )
	{
//	mousewheel = Fl::event_dy();
	
	need_redraw = 1;
	dont_pass_on = 1;
	}
	
if ( need_redraw ) redraw();

if( dont_pass_on ) return 1;

return Fl_Widget::handle(e);
}



//----------------------------------------------------------







//----------------------------------------------------------
// v1.01    09-nov-2014
// v1.02    14-nov-2014     //added stripping of \r\n option in: various functions

My_Input_Choice::My_Input_Choice(int x,int y,int w, int h,const char *label) : Fl_Input_Choice(x,y,w,h,label)
{
ctrl_key = 0;
shift_key = 0;

b_allow_wheel_inc = 1;
b_allow_enter_key_cb = 1;
b_allow_enter_key_store_to_dropdown = 1;
b_auto_sort = 0;

var_type = 0;

d_std_step = 1;
d_ctrl_step = 1;
d_shift_step = 1;
d_ctrl_shift_step = 1;

}




void My_Input_Choice::std_step( double dd )
{
d_std_step = dd;
}







void My_Input_Choice::ctrl_key_step( double dd )
{
d_ctrl_step = dd;
}







void My_Input_Choice::shift_key_step( double dd )
{
d_shift_step = dd;
}







void My_Input_Choice::ctrl_shift_key_step( double dd )
{
d_ctrl_shift_step = dd;
}











/*
// compare character case-insensitive
struct my_input_choice_icompare_char 
{
bool operator()( char c1, char c2 ) 
	{
	return std::toupper( c1 ) < std::toupper( c2 );
	}
};





// return true if s1 comes before s2
bool my_choice_alpha_compare( std::string const& s1, std::string const& s2 ) 
{
if ( s1.length() < s2.length() ) return true;
if ( s1.length() > s2.length() ) return false;
return std::lexicographical_compare( s1.begin(), s1.end(), s2.begin(), s2.end(), my_input_choice_icompare_char() );
}









// return true if s1 comes before s2
bool my_input_choice_alpha_compare( const string &s1, const string &s2 ) 
	{
	if ( s1.length() < s2.length() ) return true;
	if ( s1.length() > s2.length() ) return false;
	return std::lexicographical_compare( s1.begin(), s1.end(), s2.begin(), s2.end(), my_input_choice_icompare_char() );
	}
*/








//sort lowest to highest
static int my_input_choice_lex_cmp_str( const string &o1, const string &o2 )
{

if( std::lexicographical_compare( o1.begin(), o1.end(), o2.begin(), o2.end() ) ) return 1;


return 0;
}







//sort lowest to highest using nemerical value
static int my_input_choice_numerical_compare( const string &o1, const string &o2 )
{

double d1, d2;

sscanf( o1.c_str(), "%lf", &d1 );
sscanf( o2.c_str(), "%lf", &d2 );

if( d1 < d2 ) return 1;


return 0;
}






void My_Input_Choice::alpha_or_numerical_sort()
{
if( var_type == 0 ) std::sort( vdropdown.begin(), vdropdown.end(), my_input_choice_lex_cmp_str );
if( var_type == 1 ) std::sort( vdropdown.begin(), vdropdown.end(), my_input_choice_numerical_compare );
if( var_type == 2 ) std::sort( vdropdown.begin(), vdropdown.end(), my_input_choice_numerical_compare );

reload_hist();
}






//reload dropdown entries 
void My_Input_Choice::reload_hist()
{
string s1;

clear();

for( int i = 0; i < vdropdown.size(); i++ )
	{
	strpf( s1, "%s", vdropdown[ i ].c_str() );

	add( s1.c_str() );
	}
}








void My_Input_Choice::add_to_dropdown( int ii )
{
string s1, s2;

strpf( s1, "%d" , ii );

int count = vdropdown.size();

for( int i = 0; i < count; i++ )		            //check if already in history
	{
	if( s1.compare( vdropdown[ i ] ) == 0 )
		{
		s2 = vdropdown[ 0 ];
		vdropdown[ 0 ] = vdropdown[ i ];
		vdropdown[ i ] = s2;
		goto skip_adding;							//if so skip adding it to history
		}
	}

vdropdown.insert( vdropdown.begin(), s1 );	    //add to top if not already in history

skip_adding:

//clear();

if ( b_auto_sort ) alpha_or_numerical_sort();

reload_hist();

/*
else{
    count = vdropdown.size();
    for(int i = 0; i < count; i++ )
        {
        add( vdropdown[ i ].c_str() );
        }
    }
*/
}








void My_Input_Choice::add_to_dropdown( double dd )
{
string s1, s2;

strpf( s1, "%g" , dd );

int count = vdropdown.size();

for( int i = 0; i < count; i++ )		            //check if already in history
	{
	if( s1.compare( vdropdown[ i ] ) == 0 )
		{
		s2 = vdropdown[ 0 ];
		vdropdown[ 0 ] = vdropdown[ i ];
		vdropdown[ i ] = s2;
		goto skip_adding;							//if so skip adding it to history
		}
	}

vdropdown.insert( vdropdown.begin(), s1 );	    //add to top if not already in history

skip_adding:

if ( b_auto_sort ) alpha_or_numerical_sort();

reload_hist();

/*
clear();

count = vdropdown.size();
for(int i = 0; i < count; i++ )
	{
	add( vdropdown[ i ].c_str() );
	}
*/
}












void My_Input_Choice::add_to_dropdown( string ss, bool strip_crlf )
{
string s1, s2;

mystr m1;

m1 = ss;

if( strip_crlf )
    {
    m1.strip_cr_or_lf( s1, 1, 1 );
    }
else{
    s1 = ss;
    }

int count = vdropdown.size();

for( int i = 0; i < count; i++ )		            //check if already in history
	{
	if( s1.compare( vdropdown[ i ] ) == 0 )
		{
		s2 = vdropdown[ 0 ];
		vdropdown[ 0 ] = vdropdown[ i ];
		vdropdown[ i ] = s2;
		goto skip_adding;							//if so skip adding it to history
		}
	}

vdropdown.insert( vdropdown.begin(), s1 );	    //add to top if not already in history

skip_adding:

if ( b_auto_sort ) alpha_or_numerical_sort();

reload_hist();
/*
clear();

count = vdropdown.size();
for(int i = 0; i < count; i++ )
	{
	add( vdropdown[ i ].c_str() );
	}
*/
}














void My_Input_Choice::load_settings( string sprofile_fname, string ssection, string skey, string sdef, char delimit, bool strip_crlf )
{
string s1;

GCProfile p( sprofile_fname );

p.GetPrivateProfileStr( ssection, skey, sdef, &s1 );


mystr m1;
vector<string>vstr;
m1 = s1;
m1.LoadVectorStrings( vstr, delimit );


int count = vstr.size();

for( int i = 0; i < count; i++ )            //load control
	{
    add_to_dropdown( vstr[ i ], strip_crlf );
	}
}





void My_Input_Choice::save_settings( string sprofile_fname, string ssection, string skey, char delimit, bool strip_crlf )
{
string s1, s2, stot;

GCProfile p( sprofile_fname );


int count = vdropdown.size();

stot = "";
for( int i = 0; i < count; i++ )
	{
    s2 = vdropdown[ i ].c_str();

    if( strip_crlf )
        {
        mystr m1;
        m1.strip_cr_or_lf( s2, 1, 1 );
        }
    else{
        s2 = s1;
        }

	if( i != ( count - 1 )  )
        {
        strpf( s1 ,"%s", s2.c_str() );
        s1 += delimit;
        }
    else{
        strpf( s1 ,"%s", s2.c_str() );
        }
	stot += s1;
	}

p.WritePrivateProfileStr( ssection, skey, stot );
}









void My_Input_Choice::get_value( int *ii )
{
string s1;

s1 = value();

sscanf( s1.c_str(), "%d", ii );
}







void My_Input_Choice::get_value( double *dd )
{
string s1;

s1 = value();

sscanf( s1.c_str(), "%lf", dd );
}








void My_Input_Choice::set_value( int ii )
{
string s1;

strpf( s1, "%d", ii );

value( s1.c_str() );
}








void My_Input_Choice::set_value( double dd )
{
string s1;

strpf( s1, "%g", dd );

value( s1.c_str() );
}







int My_Input_Choice::handle(int e)
{
string s1;
int len;
char *szTmp;
bool need_redraw = 0;
bool dont_pass_on = 0;
int mousewheel;




if ( e == FL_MOUSEWHEEL )
	{
	mousewheel = Fl::event_dy();
	double dval;
    int ii;

	if( b_allow_wheel_inc )
		{
        if( var_type == 1 )                               //integer
            {
            get_value( &ii );
            dval = ii;
            }

        if( var_type == 2 ) get_value( &dval );            //double

        double step = d_std_step;
        double step_dir = 0;

        if( ctrl_key ) step = d_ctrl_step;
        if( shift_key ) step = d_shift_step;
        if( ctrl_key & shift_key ) step = d_ctrl_shift_step;

		if( mousewheel > 0 ) step_dir = step;
		if( mousewheel < 0 ) step_dir = -step;

		dval += step_dir;
//printf( "%g\n", dval );

        if( var_type == 1 ) set_value( (int)dval );       //integer
        if( var_type == 2 ) set_value( dval );            //double

        if( b_allow_wheel_cb ) do_callback();
		}

	need_redraw = 1;
	dont_pass_on = 1;
	}

    
if ( ( e == FL_KEYDOWN ) || ( e == FL_SHORTCUT ) )		//key pressed?
	{
	int key = Fl::event_key();

	if( key == FL_Enter )		                        //is it CR ?
		{
        if( b_allow_enter_key_cb )
            {
            do_callback();
            dont_pass_on = 1;
            }
            
        if( b_allow_enter_key_store_to_dropdown )
            {
            add_to_dropdown( value(), 1 );
            }
		}
	
	if( ( key == FL_Control_L ) || (  key == FL_Control_R ) ) ctrl_key = 1;
	if( ( key == FL_Shift_L ) || (  key == FL_Shift_R ) ) shift_key = 1;
	
	need_redraw = 1;
    dont_pass_on = 1;
	}


if ( e == FL_KEYUP )  				                    //key release?
	{
	int key = Fl::event_key();
	if( ( key == FL_Control_L ) || ( key == FL_Control_R ) ) ctrl_key = 0;
	if( ( key == FL_Shift_L ) || (  key == FL_Shift_R ) ) shift_key = 0;

	need_redraw = 1;
    dont_pass_on = 1;
	}


//if ( need_redraw ) redraw();

if( dont_pass_on ) return 1;

return Fl_Input_Choice::handle(e);
}



//----------------------------------------------------------

















//----------------------------------------------------------



void ucde_wnd::outcsl( string s )
{

if ( te_csl ==0 ) return;

te_csl->insert( s.c_str() );
te_csl->show_insert_position();

}




void update_fonts()
{
//wndMain->redraw();
//fi_unicode->textfont( font_num );
//fi_unicode->textsize( font_size );
//fi_unicode->redraw();

fl_message_font( (Fl_Font) font_num, font_size );

}











































//make sure 'picked' is not the same string as 'pathfilename'
//string 'picked' is loaded with selected path and filename, on 'OK' 
//on 'Cancel' string 'picked' is set to a null string
//returns 1 if 'OK', else 0
//set 'type' to Fl_File_Chooser::CREATE to allow a new filename to be entered

//linux code
#ifndef compile_for_windows
bool my_file_chooser( string &picked, const char* title, const char* pat, const char* start_path_filename, int type, Fl_Font fnt = -1, int fntsze = -1 )
{
picked = "";

//show file chooser
Fl_File_Chooser fc	( 	 start_path_filename,		// directory
						 pat,                       // filter
						 Fl_File_Chooser::SINGLE | type,   // chooser type
						 title						// title
					);

if ( fnt != -1 )fc.textfont( fnt );
if ( fntsze != -1 )fc.textsize( fntsze );

fc.show();

while( fc.shown() )
	{
	Fl::wait();
	}


if( fc.value() == 0 ) return 0;


picked = fc.value();

//windows code
//#ifdef compile_for_windows
//make the slash suit Windows OS
//mystr m1;
//m1 = fc.value();
//m1.FindReplace( picked, "/", "\\",0);
//#endif


return 1;
}
#endif












//windows code
#ifdef compile_for_windows
bool my_file_chooser( string &picked, const char* title, const char* pat, const char* start_path_filename, int type, Fl_Font fnt = -1, int fntsze = -1 )
{
OPENFILENAME ofn;
char szFile[ 8192 ];
string fname;

mystr m1;

m1 = start_path_filename;

m1.ExtractFilename( dir_seperator[ 0 ],  fname );		 //remove path from filename

strncpy( szFile, fname.c_str(), sizeof( szFile ) );		//put supplied fname as default

memset( &ofn, 0, sizeof( ofn ) );

ofn.lStructSize = sizeof ( ofn );
ofn.hwndOwner = NULL ;
ofn.lpstrFile = szFile ;
//ofn.lpstrFile[ 0 ] = '\0';
ofn.nMaxFile = sizeof( szFile );
ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
ofn.nFilterIndex = 1;
ofn.lpstrFileTitle = 0;
ofn.nMaxFileTitle = 0;
ofn.lpstrInitialDir = start_path_filename ;
ofn.lpstrTitle = title;
ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR ;

if( type == Fl_File_Chooser::CREATE )
	{
	if( GetSaveFileName( &ofn ) )
		{
		picked = szFile;
		return 1;
		}
	}
else{
	if( GetOpenFileName( &ofn ) )
		{
		picked = szFile;
		return 1;
		}
	}
return 0;
}
#endif




















//linux code
#ifndef compile_for_windows
//make sure 'picked' is not the same string as 'pathfilename'
//string 'picked' is loaded with selected dir, on 'OK'
//on 'Cancel' string 'picked' is set to a null string
//returns 1 if 'OK', else 0
//set 'type' to  Fl_File_Chooser::CREATE to allow a new directory to be entered
bool my_dir_chooser( string &picked, const char* title, const char* pat, const char* start_path_filename, int type, Fl_Font fnt = -1, int fntsze = -1 )
{
picked = "";
mystr m1;

//show file chooser
Fl_File_Chooser fc	( 	 start_path_filename,				// directory
						 pat,                     		  	// filter
						 Fl_File_Chooser::DIRECTORY | type,	// chooser type
						 title								// title
					);

if ( fnt != -1 )fc.textfont( fnt );
if ( fntsze != -1 )fc.textsize( fntsze );

fc.show();

while( fc.shown() )
	{
	Fl::wait();
	}


if( fc.value() == 0 ) return 0;


picked = fc.value();

//make the slash suit Windows OS
//m1 = fc.value();
//m1.FindReplace( picked, "/", "\\",0);



return 1;
}
#endif












//windows code
#ifdef compile_for_windows



int CALLBACK BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData ) 
{
TCHAR szDir[MAX_PATH];

switch( uMsg ) 
	{
	case BFFM_INITIALIZED: 
	if ( GetCurrentDirectory( sizeof(szDir) / sizeof(TCHAR), szDir ) )
		{
         // WParam is TRUE since you are passing a path.
         // It would be FALSE if you were passing a pidl.
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szDir);
		}
	break;

	case BFFM_SELCHANGED: 
	// Set the status window to the currently selected path.
	if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
		{
		SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
		}
	break;
   }
return 0;
}








/*

//this returns the PIDL for a particular folder, can be use with GetFolderSelection(..) code further below,
//if used in 'bi.pidlRoot', it only shows folders below the 'path', you can't seem to navigate upward past 'path'
LPITEMIDLIST get_pidl_from_path( string path )
{
LPITEMIDLIST  pidl = 0;
LPSHELLFOLDER pDesktopFolder;
char		szPath[MAX_PATH];
OLECHAR		olePath[MAX_PATH];
char		szDisplayName[MAX_PATH];
ULONG		chEaten;
ULONG		dwAttributes;
HRESULT		hr;



// 
// Get a pointer to the Desktop's IShellFolder interface.
// 
if ( SUCCEEDED( SHGetDesktopFolder( &pDesktopFolder ) ) )
	{
	// 
	// IShellFolder::ParseDisplayName requires the file name be in
	// Unicode.
	// 
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, path.c_str(), -1, olePath, MAX_PATH );

	// 
	// Convert the path to an ITEMIDLIST.
	// 
	hr = pDesktopFolder->ParseDisplayName( NULL, NULL, olePath, &chEaten, &pidl, &dwAttributes );

	if ( FAILED( hr ) )
		{
		pidl = 0;
		printf( "grrr() - failed\n" );
        // Handle error.
		}

	// 
	// pidl now contains a pointer to an ITEMIDLIST for .\readme.txt.
	// This ITEMIDLIST needs to be freed using the IMalloc allocator
	// returned from SHGetMalloc().
	// 

	//release the desktop folder object
	pDesktopFolder->Release();
	}

return 	pidl;
}

*/












//Displays a directory selection dialog. CoInitialize must be called before calling this function.
//szBuf must be MAX_PATH characters in length. hWnd may be NULL.
//Note: the get_pidl_from_path(..) call is commented out
BOOL GetFolderSelection( HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle, string starting_path )
{
LPITEMIDLIST pidl     = NULL;
BROWSEINFO   bi       = { 0 };
BOOL         bResult  = FALSE;

bi.hwndOwner      = hWnd;
bi.pszDisplayName = szBuf;
bi.pidlRoot       = 0;//  get_pidl_from_path( starting_path );		//get_pidl_from_path(..) is not used as it only shows folders below the path, you can't navigate upward past 'pidlRoot'
bi.lpszTitle      = szTitle;											
bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
bi.lpfn = BrowseCallbackProc;


if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
	bResult = SHGetPathFromIDList(pidl, szBuf);
	CoTaskMemFree(pidl);
	}

return bResult;
}











bool my_dir_chooser( string &picked, const char* title, const char* pat, const char* start_path_filename, int type, Fl_Font fnt = -1, int fntsze = -1 )
{
OPENFILENAME ofn;
char szFile[ 8192 ];



TCHAR szFolder[ MAX_PATH ];
CoInitialize( NULL );

if( GetFolderSelection( NULL, szFolder, title, start_path_filename ))
	{
	printf( "The folder selected was %s\n", szFolder );
	picked = szFolder;
	return 1;
	}

return 0;



/*
memset( &ofn, 0, sizeof( ofn ) );

ofn.lStructSize = sizeof ( ofn );
ofn.hwndOwner = NULL ;
ofn.lpstrFile = szFile ;
ofn.lpstrFile[ 0 ] = '\0';
ofn.nMaxFile = sizeof( szFile );
ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
ofn.nFilterIndex =1;
ofn.lpstrFileTitle = 0;
ofn.nMaxFileTitle = 0;
ofn.lpstrInitialDir = start_path_filename ;
ofn.lpstrTitle = title;
ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
GetOpenFileName( &ofn );

picked = szFile;
*/

}
#endif















void cb_file_open( Fl_Widget *w, void* v )
{
string s1, s2;

//linux code
#ifndef compile_for_windows
s1 = "/mnt/home/PuppyLinux/MyPrj/Skeleton Unicode fltk";
#endif

//windows code
#ifdef compile_for_windows
s1 = "c:\\gc\\MyPrj\\Skeleton Unicode fltk";
#endif

if( my_file_chooser( s2, "Open File", "*", s1.c_str(), 0, font_num, font_size ) )
	{
//	u_wnd->fi_filename->value( s2.c_str() );
	}

/*
u_wnd->fc = new ucde_file_chooser( s1.c_str(), 0, FL_SINGLE, "Open a file" );
u_wnd->fc->textfont( font_num );
u_wnd->fc->textsize( font_size );
u_wnd->fc->callback( cb_ucde_file_chooser, 0 );

//fc->redraw();
u_wnd->fc->show();

while( u_wnd->fc->shown() )
	{
	Fl::wait();
	}
GCCol.o:  GCCol.h

u_wnd->fi_filename->value( u_wnd->fc->value() );
//u_wnd->fi_filename->value("Help!");
*/

//fc->hide();
//fc->show();
//fc->fileName->show();

}





















void cb_dir_open( Fl_Widget *w, void* v )
{
string s1, s2;
//linux code
#ifndef compile_for_windows
s1 = "/mnt/home/PuppyLinux/MyPrj/Skeleton Unicode fltk";
#endif

//windows code
#ifdef compile_for_windows
s1 = "c:\\gc\\MyPrj\\Skeleton Unicode fltk";
#endif


if( my_dir_chooser( s2, "Open Dir", "*", s1.c_str(), 0, font_num, font_size ) )
	{
//	u_wnd->fi_filename->value( s2.c_str() );
	}

return;

char *ptr;

ptr = fl_dir_chooser( "Open Dir", s1.c_str() );

if( ptr )
	{
//	u_wnd->fi_filename->value( ptr );
	}
}











/*
bool check_file_exists( string fname )
{
FILE *fp;

mystr m1;

FILE* wcfopen( wstring &wstr );





//linux code
#ifndef compile_for_windows
//cslpf("src file exists: %s \n", fname.c_str() );

fp = fopen( fname.c_str() ,"rb" );		 //open file
#endif



//windows code
#ifdef compile_for_windows
wstring ws1;
mystr m1 = fname;

m1.mbcstr_wcstr( ws1 );	//convert utf8 string to windows wchar string array

fp = _wfsopen( ws1.c_str(), L"rb", _SH_DENYNO );
#endif


if( fp == 0 )
	{
	return 0;
	}


fclose( fp );
return 1;
}

*/


























//----------------------------------------------------------







dble_wnd::dble_wnd( int xx, int yy, int wid, int hei, const char *label ) : Fl_Double_Window( xx, yy, wid ,hei, label )
{
dble_wnd_verbose = 0;
maximize_boundary_x = 5;                //assume this window's title bar and borders, see dble_wnd::resize()
maximize_boundary_y = 20;
maximize_boundary_w = 10;
maximize_boundary_h = 25;

}







dble_wnd::~dble_wnd()
{

}












//try to determine if window maximize was pressed, store previous size so we can save final un-maximized values,
//makes assumption on when a window has been maximized using window title bar, border widths and actual screen resolution,
//you need to change maximize_boundary_x, maximize_boundary_y, maximize_boundary_w, maximize_boundary_h, if your window title bar/borders are different

void dble_wnd::resize( int xx, int yy, int ww, int hh )
{

//assuming screen: 0, 0, 1920, 1080
//got this resize when maximize button was pressed: 5, y: 20, w: 1910, h: 1055
if( dble_wnd_verbose ) printf( "dble_wnd::resize() - x: %d, y: %d, w: %d, h: %d\n", xx, yy, ww, hh );


int xxx, yyy, www, hhh;

Fl::screen_xywh( xxx, yyy, www, hhh );
if( dble_wnd_verbose ) printf( "dble_wnd::resize() - screen_xywh() is x: %d, y: %d, w: %d, h: %d\n", xxx, yyy, www, hhh );

if( xx >= xxx + maximize_boundary_x )                                 //detect a possible the maximize button press happened
    {
    if( yy >= ( yyy + maximize_boundary_y ) )
        {
        if( ww >= ( www - maximize_boundary_w ) )
            {
            if( hh >= ( hhh - maximize_boundary_h ) )
                {
                if( dble_wnd_verbose ) printf( "dble_wnd::resize() - possibly maximize was pressed, storing original pos/size for later\n" );

                goto probably_maximized;
                }
            }
        }
   
    }


restore_size_x = xx;                    
restore_size_y = yy;
restore_size_w = ww;
restore_size_h = hh;

Fl_Double_Window::resize( xx, yy, ww, hh );

if( dble_wnd_verbose ) printf( "dble_wnd::resize() - actually resizing to x: %d, y: %d, w: %d, h: %d\n", xx, yy, www, hh );


probably_maximized:

return;
}

//----------------------------------------------------------








void clear_csl()
{
if ( tb_csl ==0 ) return;
int len = tb_csl->length();
if( len > 0 ) tb_csl->replace(0,len,"");		//clear text buf if anything there
}



//scroll to last char in csl
void scroll_end_csl()
{
te_csl->insert_position( tb_csl->length() );                   //scroll to top
}



void outcsl(string s)
{

if ( te_csl ==0 ) return;

//scroll_end_csl();

te_csl->insert( s.c_str() );
te_csl->show_insert_position();

}








 


//console printf(...) like function
//e.g. cslpf( "\nTesing int=%05d, float=%f str=%s\n",z,(float)4.567,"I was Here" );

void cslpf( const char *fmt,... )
{
string s1;

int buf_size =  10 * 1024 * 1024;
char* buf = new char[ buf_size ];				//create a data buffer


if( buf == 0 )
	{
	printf("cslpf() - no memory allocated!!!!!!!!!!!!!!!!!!!!!!!\n");
	return;
	}

string s;
va_list ap;

va_start( ap, fmt );
vsnprintf( buf, buf_size - 1, fmt, ap );
if( my_verbose ) outcsl( buf );


//prevent string growing anywhere near 'max_size()' and causing a std::__throw_length_error(char const*)
unsigned int len = sdebug.length();
unsigned int max = sdebug.max_size();
unsigned int limit = (double)max / 1.25;

//printf("cslpf() - culling %d\n", (int)limit );

if( len > limit )
	{
	s1 = sdebug.substr( max - limit, len - ( max - limit ) );	//cut off begining section of str
	sdebug = s1;
	printf("cslpf() - culling begining of str to avoid hitting string::max_size()\n");
	}
sdebug += buf;								//for diag

va_end(ap);

delete buf;

}











void cb_pref2(Fl_Widget*w, void* v)
{
if(pref_wnd2) pref_wnd2->Show(1);
}




void make_pref2_wnd()
{
sControl sc;

if(pref_wnd2==0)
	{
	pref_wnd2 = new PrefWnd(wndMain->x()+20,wndMain->y()+20,730,330,"Preferences2","Settings","PrefWnd2Pos");
	}
else{
	pref_wnd2->Show(1);
	return;
	}


// -- dont need to do the below manual default load as "ClearToDefCtrl()" does this for you --
/*
pref_wnd2->sc.type=cnNone;						//blank gap from top of window
pref_wnd2->sc.x=105;
pref_wnd2->sc.y=0;
pref_wnd2->sc.w=150;
pref_wnd2->sc.h=20;
pref_wnd2->sc.label="";
pref_wnd2->sc.label_type=FL_ALIGN_CENTER;
pref_wnd2->sc.tooltip="";						//tool tip
pref_wnd2->sc.options="";	//menu button drop down options
pref_wnd2->sc.labelfont=-1;					//-1 means use fltk default
pref_wnd2->sc.labelsize=-1;					//-1 means use fltk default
pref_wnd2->sc.textfont=fl_font();
pref_wnd2->sc.textsize=fl_size();
pref_wnd2->sc.section="";
pref_wnd2->sc.key="";
pref_wnd2->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd2->sc.def="";							//default to use if ini value not avail
pref_wnd2->sc.iretval=(int*)-1;					//address of int to be modified, -1 means none
pref_wnd2->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
pref_wnd2->sc.uniq_id = 5;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd2->sc.cb=0;								//address of a callback if any, 0 means none
*/

pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values
pref_wnd2->AddControl();
pref_wnd2->CreateRow(10);			//specify optional row height


pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

pref_wnd2->sc.type=cnMenuChoicePref;
pref_wnd2->sc.x=120;
pref_wnd2->sc.y=0;
pref_wnd2->sc.w=150;
pref_wnd2->sc.h=20;
pref_wnd2->sc.label="Initial Execution:";
pref_wnd2->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd2->sc.label_align = FL_ALIGN_LEFT;
pref_wnd2->sc.tooltip="";						//tool tip
pref_wnd2->sc.options="&None,&Step into main(...),&Run";	//menu button drop down options
pref_wnd2->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd2->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd2->sc.textfont=-1;//fl_font();
pref_wnd2->sc.textsize=-1;//fl_size();
pref_wnd2->sc.section="MyPref";
pref_wnd2->sc.key="InitExec";
pref_wnd2->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd2->sc.def="0";							//default to use if ini value not avail
pref_wnd2->sc.iretval=&gi0;						//address of int to be modified, -1 means none
pref_wnd2->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
pref_wnd2->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
pref_wnd2->sc.cb = cb_user2;					//address of a callback if any, 0 means none
pref_wnd2->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd2->sc.uniq_id = 0;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd2->AddControl();

pref_wnd2->CreateRow(25);			//specify optional row height

pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

pref_wnd2->sc.type=cnCheckPref;
pref_wnd2->sc.x=0;
pref_wnd2->sc.y=0;
pref_wnd2->sc.w=300;
pref_wnd2->sc.h=20;
pref_wnd2->sc.label="Bring Watch Window to Front at each BreakPoint";
pref_wnd2->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd2->sc.label_align = FL_ALIGN_LEFT;
pref_wnd2->sc.tooltip="This is My Tool Tip";						//tool tip
pref_wnd2->sc.options="";						//menu button drop down options
pref_wnd2->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd2->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd2->sc.textfont=4;
pref_wnd2->sc.textsize=12;
pref_wnd2->sc.section="MyPref";
pref_wnd2->sc.key="WatchToFront";
pref_wnd2->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd2->sc.def="0";							//default to use if ini value not avail
pref_wnd2->sc.iretval=&gi1;						//address of int to be modified, -1 means none
pref_wnd2->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
pref_wnd2->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
pref_wnd2->sc.cb = cb_user2;					//address of a callback if any, 0 means none
pref_wnd2->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd2->sc.uniq_id = 1;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd2->AddControl();

pref_wnd2->CreateRow(25);					//specify optional row height


pref_wnd->ClearToDefCtrl();					//this will clear sc struct to safe default values

pref_wnd2->sc.type=cnCheckPref;
pref_wnd2->sc.x=0;
pref_wnd2->sc.y=0;
pref_wnd2->sc.w=300;
pref_wnd2->sc.h=20;
pref_wnd2->sc.label="Prompt to Reload, if Executable Datestamp Changes";
pref_wnd2->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd2->sc.label_align = FL_ALIGN_LEFT;
pref_wnd2->sc.tooltip="";						//tool tip
pref_wnd2->sc.options="";						//menu button drop down options
pref_wnd2->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd2->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd2->sc.textfont=-1;
pref_wnd2->sc.textsize=-1;
pref_wnd2->sc.section="MyPref";
pref_wnd2->sc.key="DateStampReload";
pref_wnd2->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd2->sc.def="0";							//default to use if ini value not avail
pref_wnd2->sc.iretval=&gi2;						//address of int to be modified, -1 means none
pref_wnd2->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
pref_wnd2->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
pref_wnd2->sc.cb = cb_user2;					//address of a callback if any, 0 means none
pref_wnd2->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd2->sc.uniq_id = 2;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd2->AddControl();

pref_wnd2->CreateRow(25);						//specify optional row height


pref_wnd->ClearToDefCtrl();					//this will clear sc struct to safe default values

pref_wnd2->sc.type=cnInputIntPref;
pref_wnd2->sc.x=190;
pref_wnd2->sc.y=0;
pref_wnd2->sc.w=150;
pref_wnd2->sc.h=20;
pref_wnd2->sc.label="Enter a Integer Num:";
pref_wnd2->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd2->sc.label_align = FL_ALIGN_LEFT;
pref_wnd2->sc.tooltip="";						//tool tip
pref_wnd2->sc.options="";						//menu button drop down options
pref_wnd2->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd2->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd2->sc.textfont=-1;
pref_wnd2->sc.textsize=-1;
pref_wnd2->sc.section="MyPref";
pref_wnd2->sc.key="Integer1";
pref_wnd2->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd2->sc.def="0";							//default to use if ini value not avail
pref_wnd2->sc.iretval=&gi3;						//address of int to be modified, -1 means none
pref_wnd2->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
pref_wnd2->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
pref_wnd2->sc.cb = cb_user2;					//address of a callback if any, 0 means none
pref_wnd2->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd2->sc.uniq_id = 3;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd2->AddControl();

pref_wnd2->CreateRow(25);						//specify optional row height


pref_wnd->ClearToDefCtrl();					//this will clear sc struct to safe default values

pref_wnd2->sc.type=cnInputDoublePref;
pref_wnd2->sc.x=200;
pref_wnd2->sc.y=0;
pref_wnd2->sc.w=150;
pref_wnd2->sc.h=20;
pref_wnd2->sc.label="Enter a Floating Point Num:";
pref_wnd2->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd2->sc.label_align = FL_ALIGN_LEFT;
pref_wnd2->sc.tooltip="";						//tool tip
pref_wnd2->sc.options="";						//menu button drop down options
pref_wnd2->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd2->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd2->sc.textfont=-1;
pref_wnd2->sc.textsize=-1;
pref_wnd2->sc.section="MyPref";
pref_wnd2->sc.key="Float1";
pref_wnd2->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd2->sc.def="0";							//default to use if ini value not avail
pref_wnd2->sc.iretval=(int*)-1;					//address of int to be modified, -1 means none
pref_wnd2->sc.dretval=&gd0;						//address of double to be modified, -1 means none
pref_wnd2->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
pref_wnd2->sc.cb = cb_user2;					//address of a callback if any, 0 means none
pref_wnd2->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd2->sc.uniq_id = 4;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd2->AddControl();

pref_wnd2->CreateRow(25);						//specify optional row height


pref_wnd->ClearToDefCtrl();					//this will clear sc struct to safe default values

pref_wnd2->sc.type=cnInputHexPref;
pref_wnd2->sc.x=190;
pref_wnd2->sc.y=0;
pref_wnd2->sc.w=150;
pref_wnd2->sc.h=20;
pref_wnd2->sc.label="Enter a Hex Num:";
pref_wnd2->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd2->sc.label_align = FL_ALIGN_LEFT;
pref_wnd2->sc.tooltip="";						//tool tip
pref_wnd2->sc.options="";						//menu button drop down options
pref_wnd2->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd2->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd2->sc.textfont=-1;
pref_wnd2->sc.textsize=-1;
pref_wnd2->sc.section="MyPref";
pref_wnd2->sc.key="Hex1";
pref_wnd2->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd2->sc.def="0";							//default to use if ini value not avail
pref_wnd2->sc.iretval=&gi4;						//address of int to be modified, -1 means none
pref_wnd2->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
pref_wnd2->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
pref_wnd2->sc.cb = cb_user2;					//address of a callback if any, 0 means none
pref_wnd2->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd2->sc.uniq_id = 5;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd2->AddControl();

pref_wnd2->CreateRow(25);						//specify optional row height



pref_wnd2->ClearToDefCtrl();					//this will clear sc struct to safe default values

pref_wnd2->sc.type=cnInputPref;
pref_wnd2->sc.x=100;
pref_wnd2->sc.y=0;
pref_wnd2->sc.w=570;
pref_wnd2->sc.h=25;
pref_wnd2->sc.label="FavEditor:";
pref_wnd2->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd2->sc.label_align = FL_ALIGN_LEFT;
pref_wnd2->sc.tooltip="Enter Appname of your favorite text editor"; //tool tip
pref_wnd2->sc.options="";						//menu button drop down options
pref_wnd2->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd2->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd2->sc.textfont=4;
pref_wnd2->sc.textsize=12;
pref_wnd2->sc.section="Pref";
pref_wnd2->sc.key="FavEditor";
pref_wnd2->sc.keypostfix=-1;						//ini file Key post fix
pref_wnd2->sc.def="";							//default to use if ini value not avail
pref_wnd2->sc.iretval=(int*)-1;			       	//address of int to be modified, -1 means none
pref_wnd2->sc.sretval=&fav_editor;				//address of string to be modified, -1 means none
pref_wnd2->sc.cb = cb_user2;					//address of a callback if any, 0 means none
pref_wnd2->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd2->sc.uniq_id = 6;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd2->AddControl();

pref_wnd2->CreateRow( 25 );							//specify optional row height




pref_wnd2->sc.type=cnGCColColour;
pref_wnd2->sc.x=70;
pref_wnd2->sc.y=2;
pref_wnd2->sc.w=84;
pref_wnd2->sc.h=20;
pref_wnd2->sc.label="Colour1";
pref_wnd2->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd2->sc.label_align = FL_ALIGN_LEFT;
pref_wnd2->sc.tooltip="Set r,g,b colour value"; //tool tip
pref_wnd2->sc.options="";						//menu button drop down options
pref_wnd2->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd2->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd2->sc.textfont=4;
pref_wnd2->sc.textsize=12;
pref_wnd2->sc.section="Pref";
pref_wnd2->sc.key="Colour1";
pref_wnd2->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd2->sc.def="255,255,0";						//default to use if ini value not avail
pref_wnd2->sc.iretval=(int*)-1;			       	//address of int to be modified, -1 means none
pref_wnd2->sc.sretval=&sg_col1;				//address of string to be modified, -1 means none
pref_wnd2->sc.cb = cb_user2;					//address of a callback if any, 0 means none
pref_wnd2->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd2->sc.uniq_id = 7;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd2->AddControl();

pref_wnd2->CreateRow(25);					//specify optional row height


pref_wnd2->sc.type=cnGCColColour;
pref_wnd2->sc.x=70;
pref_wnd2->sc.y=2;
pref_wnd2->sc.w=84;
pref_wnd2->sc.h=20;
pref_wnd2->sc.label="Colour2";
pref_wnd2->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd2->sc.label_align = FL_ALIGN_LEFT;
pref_wnd2->sc.tooltip="Set r,g,b colour value"; //tool tip
pref_wnd2->sc.options="";						//menu button drop down options
pref_wnd2->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd2->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd2->sc.textfont=4;
pref_wnd2->sc.textsize=12;
pref_wnd2->sc.section="Pref";
pref_wnd2->sc.key="Colour2";
pref_wnd2->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd2->sc.def="0,255,255";						//default to use if ini value not avail
pref_wnd2->sc.iretval=(int*)-1;			       	//address of int to be modified, -1 means none
pref_wnd2->sc.sretval=&sg_col2;				//address of string to be modified, -1 means none
pref_wnd2->sc.cb = cb_user2;					//address of a callback if any, 0 means none
pref_wnd2->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd2->sc.uniq_id = 8;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd2->AddControl();

pref_wnd2->CreateRow(25);					//specify optional row height


pref_wnd2->End();								//do end for all windows

}




void cb_pref(Fl_Widget*w, void* v)
{
if(pref_wnd) pref_wnd->Show(1);
}



#define cnrows 30
#define cnctrls 10

int val[ cnrows ][ cnctrls ];
string sval[ cnrows ][ cnctrls ];

void make_pref_wnd()
{
string s;
sControl sc;

if( pref_wnd == 0 )
	{
	pref_wnd = new PrefWnd(wndMain->x() + 20, wndMain->y() + 20, 1023, 300, "Preferences", "Settings", "PrefWnd1Pos");
	}
else{
	pref_wnd->Show(1);
	return;
	}

//pref_wnd->pck->begin();

for(int i=0;i<cnrows;i++)
	{
	pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

	pref_wnd->sc.type = cnStaticTextPref;
	pref_wnd->sc.x = 0;
	pref_wnd->sc.y = 0;
	pref_wnd->sc.w = 60;
	pref_wnd->sc.h = 20;
	pref_wnd->sc.label = "IP No:";
	pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
	pref_wnd->sc.label_align = FL_ALIGN_CENTER;
	pref_wnd->sc.tooltip = "Enter an IP number, e.g. 10.27.25.131";		//tool tip
	pref_wnd->sc.options = "";						//menu button drop down options
	pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
	pref_wnd->sc.labelsize = -1;					//-1 means use fltk default
	pref_wnd->sc.textfont = 4;
	pref_wnd->sc.textsize = 12;
	pref_wnd->sc.ctrl_style = 0;                    //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
	pref_wnd->sc.section = "";						//ini file Section
	pref_wnd->sc.key = "";							//ini file Key
	pref_wnd->sc.keypostfix = -1;					//ini file Key post fix
	pref_wnd->sc.def = "";							//default to use if ini value not avail
	pref_wnd->sc.iretval = (int*)-1;				//address of int to be modified, -1 means none
	pref_wnd->sc.sretval = (string*)-1;				//address of string to be modified, -1 means none
	pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
    pref_wnd->sc.uniq_id = i * 64 + 0;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
	pref_wnd->AddControl();


	pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values
	
	pref_wnd->sc.type=cnInputPref;
	pref_wnd->sc.x = 110;
	pref_wnd->sc.y = 0;
	pref_wnd->sc.w = 120;
	pref_wnd->sc.h = 20;
	pref_wnd->sc.label = "Input:";
	pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
	pref_wnd->sc.label_align = FL_ALIGN_LEFT;
	pref_wnd->sc.tooltip = "";						//tool tip
	pref_wnd->sc.options = "";						//menu button drop down options
	pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
	pref_wnd->sc.labelsize = -1;					//-1 means use fltk default
	pref_wnd->sc.textfont = 4;
	pref_wnd->sc.textsize = 12;
	pref_wnd->sc.ctrl_style = 0;                    //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
	pref_wnd->sc.section = "Pref";
	pref_wnd->sc.key = "IP";
	pref_wnd->sc.keypostfix = i;					//ini file Key post fix
	pref_wnd->sc.def = "0.0.0.0.0";					//default to use if ini value not avail
	pref_wnd->sc.iretval = (int*)-1;				//address of int to be modified, -1 means none
	pref_wnd->sc.sretval = &sval[i][0];				//address of string to be modified
	pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
    pref_wnd->sc.uniq_id = i * 64 + 1;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
	pref_wnd->AddControl();


	pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

	pref_wnd->sc.type = cnCheckPref;
	pref_wnd->sc.x = 300;
	pref_wnd->sc.y = 0;
	pref_wnd->sc.w = 20;
	pref_wnd->sc.h = 20;
	pref_wnd->sc.label = "Enable";
	pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
	pref_wnd->sc.label_align = FL_ALIGN_LEFT;
	pref_wnd->sc.tooltip = "";						//tool tip
	pref_wnd->sc.options = "";						//menu button drop down options
	pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
	pref_wnd->sc.labelsize = -1;				    //-1 means use fltk default
	pref_wnd->sc.textfont = 4;
	pref_wnd->sc.textsize = 12;
	pref_wnd->sc.ctrl_style = 0;                    //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
	pref_wnd->sc.section = "Pref";
	pref_wnd->sc.key = "Enb";
	pref_wnd->sc.keypostfix = i;					//ini file Key post fix
	pref_wnd->sc.def = "0";							//default to use if ini value not avail
	pref_wnd->sc.iretval = &val[ i ][ 0 ];				//address of int to be modified, -1 means none
	pref_wnd->sc.sretval = (string*)-1;				//address of string to be modified, -1 means none
	pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
    pref_wnd->sc.uniq_id = i * 64 + 2;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
	pref_wnd->AddControl();


	pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

	pref_wnd->sc.type = cnRoundButtonPref;
	pref_wnd->sc.x = 370;
	pref_wnd->sc.y = 0;
	pref_wnd->sc.w = 20;
	pref_wnd->sc.h = 20;
	pref_wnd->sc.label = "Cycle";
	pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
	pref_wnd->sc.label_align = FL_ALIGN_LEFT;
	pref_wnd->sc.tooltip = "";						//tool tip
	pref_wnd->sc.options = "";						//menu button drop down options
	pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
	pref_wnd->sc.labelsize = -1;					//-1 means use fltk default
	pref_wnd->sc.textfont = 4;
	pref_wnd->sc.textsize = 12;
	pref_wnd->sc.ctrl_style = 0;                    //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
	pref_wnd->sc.section = "Pref";
	pref_wnd->sc.key = "Round";
	pref_wnd->sc.keypostfix = i;					//ini file Key post fix
	pref_wnd->sc.def = "0";							//default to use if ini value not avail
	pref_wnd->sc.iretval = &val[ i ][ 1 ];			//address of int to be modified, -1 means none
	pref_wnd->sc.sretval = (string*)-1;				//address of string to be modified, -1 means none
	pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
    pref_wnd->sc.uniq_id = i * 64 + 3;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
	pref_wnd->AddControl();


	pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

	pref_wnd->sc.type = cnRoundButtonPref;
	pref_wnd->sc.x = 450;
	pref_wnd->sc.y = 0;
	pref_wnd->sc.w = 20;
	pref_wnd->sc.h = 20;
	pref_wnd->sc.label = "Cycle2";
	pref_wnd->sc.label_type = FL_SHADOW_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
	pref_wnd->sc.label_align = FL_ALIGN_LEFT;
	pref_wnd->sc.tooltip = "";						//tool tip
	pref_wnd->sc.options = "";						//menu button drop down options
	pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
	pref_wnd->sc.labelsize = -1;					//-1 means use fltk default
	pref_wnd->sc.textfont = 4;
	pref_wnd->sc.textsize = 12;
	pref_wnd->sc.ctrl_style = 0;                    //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
	pref_wnd->sc.section= "Pref";
	pref_wnd->sc.key = "Round2";
	pref_wnd->sc.keypostfix = i;					//ini file Key post fix
	pref_wnd->sc.def = "0";							//default to use if ini value not avail
	pref_wnd->sc.iretval = &val[ i ][ 2 ];			//address of int to be modified, -1 means none
	pref_wnd->sc.sretval = (string*)-1;				//address of string to be modified, -1 means none
	pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
    pref_wnd->sc.uniq_id = i * 64 + 4;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
	pref_wnd->AddControl();


	pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

	pref_wnd->sc.type = cnMenuChoicePref;
	pref_wnd->sc.x = 600;
	pref_wnd->sc.y = 0;
	pref_wnd->sc.w = 150;
	pref_wnd->sc.h = 20;
	pref_wnd->sc.label = "Initial Execution:";
	pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
	pref_wnd->sc.label_align = FL_ALIGN_LEFT;
	pref_wnd->sc.tooltip = "";						//tool tip
	pref_wnd->sc.options = "&None,&Step into main(...),&Run";	//menu button drop down options
	pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
	pref_wnd->sc.labelsize = -1;					//-1 means use fltk default
	pref_wnd->sc.textfont = -1;
	pref_wnd->sc.textsize = -1;
	pref_wnd->sc.ctrl_style = 0;                    //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
	pref_wnd->sc.section = "Pref";
	pref_wnd->sc.key = "MenuCh";
	pref_wnd->sc.keypostfix = i;					//ini file Key post fix
	pref_wnd->sc.def = "0";							//default to use if ini value not avail
	pref_wnd->sc.iretval = &val[ i ][ 3 ];			//address of int to be modified, -1 means none
	pref_wnd->sc.sretval = (string*)-1;				//address of string to be modified, -1 means none
	pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
    pref_wnd->sc.uniq_id = i * 64 + 5;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
	pref_wnd->AddControl();


	pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values
	
	pref_wnd->sc.type = cnRadioButtonPref;
	pref_wnd->sc.x = 810;
	pref_wnd->sc.y = 0;
	pref_wnd->sc.w = 20;
	pref_wnd->sc.h = 20;
	pref_wnd->sc.label = "Cycle3";
	pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
	pref_wnd->sc.label_align = FL_ALIGN_LEFT;
	pref_wnd->sc.tooltip = "";						//tool tip
	pref_wnd->sc.options = "";						//menu button drop down options
	pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
	pref_wnd->sc.labelsize = -1;					//-1 means use fltk default
	pref_wnd->sc.textfont = 4;
	pref_wnd->sc.textsize = 12;
	pref_wnd->sc.ctrl_style = 0;                    //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
	pref_wnd->sc.section = "Pref";
	pref_wnd->sc.key = "Radio";
	pref_wnd->sc.keypostfix = i;					//ini file Key post fix
	pref_wnd->sc.def = "0";							//default to use if ini value not avail
	pref_wnd->sc.iretval = &val[ i ][ 4 ];			//address of int to be modified, -1 means none
	pref_wnd->sc.sretval = (string*)-1;				//address of string to be modified, -1 means none
	pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
	pref_wnd->sc.radio_group_name = "my_radio_group";	//want this button to be exclusive, so only one on on all rows
    pref_wnd->sc.uniq_id = i * 64 + 6;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
	pref_wnd->AddControl();


	pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

	pref_wnd->sc.type = cnButtonPref;
	pref_wnd->sc.x = 840;
	pref_wnd->sc.y = 1;
	pref_wnd->sc.w = 45;
	pref_wnd->sc.h = 17;
	pref_wnd->sc.label = "Ping";
	pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
	pref_wnd->sc.label_align = FL_ALIGN_CENTER;
	pref_wnd->sc.tooltip = "";						//tool tip
	pref_wnd->sc.options = "";						//menu button drop down options
	pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
	pref_wnd->sc.labelsize = 12;					//-1 means use fltk default
	pref_wnd->sc.textfont = -1;
	pref_wnd->sc.textsize = -1;
	pref_wnd->sc.ctrl_style = 0;                    //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
	pref_wnd->sc.section = "";
	pref_wnd->sc.key = "";
	pref_wnd->sc.keypostfix = i;					//ini file Key post fix
	pref_wnd->sc.def = "0";							//default to use if ini value not avail
	pref_wnd->sc.iretval = (int*)-1;				//address of int to be modified, -1 means none
	pref_wnd->sc.sretval = (string*)-1;				//address of string to be modified, -1 means none
	pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
    pref_wnd->sc.uniq_id = i * 64 + 7;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
	pref_wnd->AddControl();


	pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

	pref_wnd->sc.type = cnButtonPref;
	pref_wnd->sc.x = 900;
	pref_wnd->sc.y = 1;
	pref_wnd->sc.w = 45;
	pref_wnd->sc.h = 17;
	pref_wnd->sc.label = "Ping2";
	pref_wnd->sc.label_type = FL_ENGRAVED_LABEL;    //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
	pref_wnd->sc.label_align = FL_ALIGN_CENTER;
	pref_wnd->sc.tooltip = "";						//tool tip
	pref_wnd->sc.options = "";						//menu button drop down options
	pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
	pref_wnd->sc.labelsize = 12;					//-1 means use fltk default
	pref_wnd->sc.textfont = -1;
	pref_wnd->sc.textsize = -1;
	pref_wnd->sc.ctrl_style = 0;                    //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
	pref_wnd->sc.section = "";
	pref_wnd->sc.key = "";
	pref_wnd->sc.keypostfix = i;					//ini file Key post fix
	pref_wnd->sc.def = "0";							//default to use if ini value not avail
	pref_wnd->sc.iretval = (int*)-1;				//address of int to be modified, -1 means none
	pref_wnd->sc.sretval = ( string*)-1;			//address of string to be modified, -1 means none
	pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
    pref_wnd->sc.uniq_id = i * 64 + 8;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
	pref_wnd->AddControl();


	pref_wnd->ClearToDefCtrl();			            //this will clear sc struct to safe default values

	pref_wnd->sc.type = cnGCLed;
	pref_wnd->sc.x = 980;
	pref_wnd->sc.y = 2;
	pref_wnd->sc.w = 15;
	pref_wnd->sc.h = 15;
	pref_wnd->sc.label = "Led";
	pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
	pref_wnd->sc.label_align = FL_ALIGN_LEFT;
	pref_wnd->sc.tooltip = "MyLed";					//tool tip
	pref_wnd->sc.options = "255 0 0, 0 255 0, 0 0 255";	//menu button drop down options or GCLed colour list like: "255 0 0, 0 255 0, 0 0 255"
	pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
	pref_wnd->sc.labelsize = -1;					//-1 means use fltk default
	pref_wnd->sc.textfont= -1;
	pref_wnd->sc.textsize = -1;
	pref_wnd->sc.ctrl_style = cn_gcled_style_round; //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
	pref_wnd->sc.section = "Pref";
	pref_wnd->sc.key = "Led";
	pref_wnd->sc.keypostfix = i;					//ini file Key post fix
	pref_wnd->sc.def = "0";						    //default to use if ini value not avail
	pref_wnd->sc.iretval = &val[ i ][ 5 ];			//address of int to be modified, -1 means none
	pref_wnd->sc.sretval =(string*)-1;				//address of string to be modified, -1 means none
	pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
    pref_wnd->sc.uniq_id = i * 64 + 9;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
	pref_wnd->AddControl();

	pref_wnd->CreateRow();							//specify optional row height
	}

pref_wnd->End();						//do end for all windows

}














//----------------------------------------------------------------------------------
//this is the callback that is called by buttons that specified is in - 
//definition ....pref_wnd->sc.cb=(void*)cb_user;
//'*o' is the PrefWnd* ptr 
//'row' is the row the control lives on, 0 is first row
//'ctrl' is the number of the controlon that row, 0 is first control
void cb_user1(void *o, int row,int ctrl)
{
PrefWnd *w = (PrefWnd *) o;


unsigned int id = w->ctrl_list[ row ][ ctrl ].uniq_id;  //get user id for controll making this call

if( id == 9 )                                   //first led control?
    {
    val[ 0 ][ 5 ]++;
    if( val[ 0 ][ 5 ] >=3 ) val[ 0 ][ 5 ] = 0;  //cycle led states
    
    GCLed *o = (GCLed *)w->ctrl_ptrs[ row ][ ctrl ];
    
    o->ChangeCol( val[ 0 ][ 5 ] );              //change led state
    }


printf("\ncb_user1() - Ping by Control on Row=%d at control count Ctrl=%d on this row, uniq_id: %d\n", row, ctrl, id );

}








//this is the callback that is called by buttons that specified is in - 
//definition ....pref_wnd->sc.cb=(void*)cb_user;
//'*o' is the PrefWnd* ptr 
//'row' is the row the control lives on, 0 is first row
//'ctrl' is the number of the controlon that row, 0 is first control
void cb_user2(void *o, int row,int ctrl)
{
PrefWnd *w = (PrefWnd *) o;


unsigned int id = w->ctrl_list[ row ][ ctrl ].uniq_id;

printf("\ncb_user2() - Ping by Control on Row=%d at control count Ctrl=%d on this row, uniq_id: %d\n", row, ctrl, id );
}






//this is the callback that is called by buttons that specified is in - 
//definition ....pref_wnd->sc.cb=(void*)cb_user;
//'*o' is the PrefWnd* ptr 
//'row' is the row the control lives on, 0 is first row
//'ctrl' is the number of the controlon that row, 0 is first control
void cb_user3(void *o, int row,int ctrl)
{
PrefWnd *w = (PrefWnd *) o;

unsigned int id = w->ctrl_list[ row ][ ctrl ].uniq_id;

printf("\ncb_user3() - Ping by Control on Row=%d at control count Ctrl=%d on this row, uniq_id: %d\n", row, ctrl, id );

update_fonts();
}





void cb_font_pref(Fl_Widget*w, void* v)
{
if(font_pref_wnd)
	{
	font_pref_wnd->hide();
	font_pref_wnd->Show(0);
	}
}











void make_font_pref_wnd()
{
sControl sc;


if( font_pref_wnd == 0 )
	{
	font_pref_wnd = new PrefWnd(wndMain->x()+20,wndMain->y()+20,400,115,"Font Preference","Settings","FontPrefWndPos");
	}
else{
	font_pref_wnd->Show(0);
	return;
	}

PrefWnd* o=font_pref_wnd;



string fnames;
string fsizes;



int maxfont = Fl::set_fonts("*");
string s;

for (int i = 0; i < maxfont; i++)
	{
    int t;
    const char *name = Fl::get_font_name((Fl_Font)i,&t);
//    printf("%d: %s\n",i,name);
    strpf(s,"%02d: %s",i,name);
    fnames+=s;
    fnames+=",";
	}


for (int i = 0; i <= 72; i++)
	{
	string s;
	
	strpf(s,"%d",i);
	fsizes+=s;
	fsizes+=",";
	}






// -- dont need to do the below manual default load as "ClearToDefCtrl()" does this for you --

o->ClearToDefCtrl();			//this will clear sc struct to safe default values
o->AddControl();
o->CreateRow(10);				//specify optional row height


o->ClearToDefCtrl();			//this will clear sc struct to safe default values

o->sc.type=cnMenuChoicePref;
o->sc.x=105;
o->sc.y=0;
o->sc.w=250;
o->sc.h=25;
o->sc.label="Font Type:";
o->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
o->sc.label_align = FL_ALIGN_LEFT;
o->sc.tooltip="";						//tool tip
o->sc.options=fnames;	//menu button drop down options
o->sc.labelfont=-1;						//-1 means use fltk default
o->sc.labelsize=-1;						//-1 means use fltk default
o->sc.textfont=-1;
o->sc.textsize=-1;
o->sc.section="Settings";
o->sc.key="FontTypeEditor";
o->sc.keypostfix=-1;					//ini file Key post fix
o->sc.def="0";							//default to use if ini value not avail
o->sc.iretval=&font_num;				//address of int to be modified, -1 means none
o->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
o->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
o->sc.cb=cb_user3;						//address of a callback if any, 0 means none
o->sc.dynamic=1;						//allow immediate dynamic change of user var
o->sc.uniq_id = 0;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
o->AddControl();

o->CreateRow(30);						//specify optional row height

o->ClearToDefCtrl();					//this will clear sc struct to safe default values

o->sc.type=cnMenuChoicePref;
o->sc.x=105;
o->sc.y=0;
o->sc.w=50;
o->sc.h=25;
o->sc.label="Font Size";
o->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
o->sc.label_align = FL_ALIGN_LEFT;
o->sc.tooltip="This is My Tool Tip";	//tool tip
o->sc.options=fsizes;					//menu button drop down options
o->sc.labelfont=-1;						//-1 means use fltk default
o->sc.labelsize=-1;						//-1 means use fltk default
o->sc.textfont=-1;
o->sc.textsize=-1;
o->sc.section="Settings";
o->sc.key="FontSizeEditor";
o->sc.keypostfix=-1;					//ini file Key post fix
o->sc.def="12";							//default to use if ini value not avail
o->sc.iretval=&font_size;			//address of int to be modified, -1 means none
o->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
o->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
o->sc.dynamic=1;						//allow immediate dynamic change of user var
o->sc.cb=cb_user3;						//address of a callback if any, 0 means none
o->sc.uniq_id = 1;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
o->AddControl();

o->CreateRow(30);						//specify optional row height



o->End();								//do end for all windows

}








//----------------------------------------------------------------------------------












void cb_btTest2(Fl_Widget *, void *)
{
cb_pref2(0,0);
}





void cb_btTest(Fl_Widget *, void *)
{
cb_pref(0,0);
}




void cb_bt_clear(Fl_Widget *, void *)
{
clear_csl();
}






















void cb_btRunAnyway(Fl_Widget *w, void* v)
{
Fl_Widget* wnd=(Fl_Widget*)w;
Fl_Window *win;
win=(Fl_Window*)wnd->parent();

iAppExistDontRun = 0;
win->~Fl_Window();
}






void cb_btDontRunExit(Fl_Widget* w, void* v)
{
Fl_Widget* wnd=(Fl_Widget*)w;
Fl_Window *win;
win=(Fl_Window*)wnd->parent();

win->~Fl_Window();
}









//linux code
#ifndef compile_for_windows 

//gets its ID, -- fixed memory leak using XFetchName (used XFree) 01-10-10
int FindWindowID(string csName,Display *display,Window &wid)
{
Window root, parent, *children;
unsigned int numWindows = 0;
int iRet=0;

//*display = XOpenDisplay(NULL);
//gDisp = XOpenDisplay(NULL);

//if(cnShowFindResults) printf("\nDispIn %x\n",display);

XQueryTree(gDisp, RootWindow(gDisp,0), &root, &parent, &children, &numWindows);

int i = 0;
for(i=0; i < numWindows ; i++)
	{
//	char *name;
	Window root2, parent2, *children2;
//	XFetchName(display, children[i], &name);

	
	unsigned int numWindows2 = 0;

//	if(cnShowFindResults) if(name) printf("Window name: %s\n", name);

	XQueryTree(display, children[i], &root2, &parent2, &children2, &numWindows2);
	for(int j=0; j < numWindows2 ; j++)
		{
		char *name;
		XFetchName(display, children2[j], &name);

		
//		unsigned int numWindows2 = 0;
//		Window root2, parent2, *children2;
//		XQueryTree(display, RootWindow(display,0), children[i], &parent2, &children2, &numWindows2);
		 
		if(name) 
			{
//		if(cnShowFindResults) printf("    Window2 name: %s  Id=%x\n", name2,children2[j]);

			if(strcmp(csName.c_str(),name)==0)
				{
//				if(cnShowFindResults) printf("Found It................\n");
//				XMoveWindow(display, children2[j], -100, -100);
//				XMoveWindow(display, children2[j], -100, -100);
//				XMoveWindow(display, children2[j], -100, -100);
//				XResizeWindow(display, children2[j], 1088, 612+22);
//				XMoveWindow(*display, children2[j], 1100, 22);
				wid=children2[j];
//				gw=children2[j];
				iRet=1;
//				return 0;
//				if(iRet)
//					{
//					printf("\n\nTrying to Move %x  %x\n\n",gDisp, gw);
//					XMoveWindow(gDisp, gw, 700, 22);
//					return 1;
//					}
				}
			XFree(name);
			}
		}
	 if(children2) XFree(children2);
	}

if(children) XFree(children);
return  iRet;
}

#endif




void BringWindowToFront(string csTitle)
{

//linux code
#ifndef compile_for_windows
Window wid;
if(FindWindowID(csTitle,gDisp,wid))
	{
	XUnmapWindow(gDisp, wid);
	XMapWindow(gDisp, wid);
	XFlush(gDisp);
	}
#endif


//windows code
#ifdef compile_for_windows
HWND hWnd;
//csAppName.LoadString(IDS_APPNAME);

hWnd = FindWindow( 0, cnsAppName );

if( hWnd )
	{
	::BringWindowToTop( hWnd );
//	::SetForegroundWindow( hWnd );
//	::PostMessage(hWnd,WM_MAXIMIZE,0,0);
	::ShowWindow( hWnd, SW_RESTORE );
	}
#endif

}







//linux code
#ifndef compile_for_windows 

//test if window with csAppName already exists, if so create inital main window with
//two options to either run app, or to exit.
//if no window with same name exists returns 0
//if 'exit' option chosen, exit(0) is called and no return happens
//if 'run anyway' option is chosen, returns 1
int CheckInstanceExists(string csAppName)
{
string csTmp;

gDisp = XOpenDisplay(NULL);

Window wid;


if(FindWindowID(csAppName,gDisp,wid))		//a window with same name exists?
	{
	BringWindowToFront( csAppName );

	XCloseDisplay(gDisp);		//added this to see if valgrind showed improvement - it didn't

	Fl_Window *wndInitial = new Fl_Window(50,50,330,90);
	wndInitial->label("Possible Instance Already Running");
	
	Fl_Box *bxHeading = new Fl_Box(10,10,200, 15, "Another Window With Same Name Was Found.");
	bxHeading->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

	strpf(csTmp,"App Name: '%s'",csAppName.c_str()); 
	Fl_Box *bxAppName = new Fl_Box(10,30,200, 15,csTmp.c_str());
	bxAppName->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

	Fl_Button *btRunAnyway = new Fl_Button(25,55,130,25,"Run App Anyway");
	btRunAnyway->labelsize(12);
	btRunAnyway->callback(cb_btRunAnyway,0);

	Fl_Button *btDontRunExit = new Fl_Button(btRunAnyway->x()+btRunAnyway->w()+15,55,130,25,"Don't Run App, Exit");
	btDontRunExit->labelsize(12);
	btDontRunExit->callback(cb_btDontRunExit,0);

	wndInitial->end();
	wndInitial->show();
	
	Fl::run();

	return iAppExistDontRun;
	}
else iAppExistDontRun=0;

XCloseDisplay(gDisp);		//added this to see if valgrind showed improvement - it didn't

return iAppExistDontRun;

}

#endif














//windows code
#ifdef compile_for_windows 

//test if window with csAppName already exists, if so create inital main window with
//two options to either run app, or to exit.
//if no window with same name exists returns 0
//if 'exit' option chosen, exit(0) is called and no return happens
//if 'run anyway' option is chosen, returns 1
int CheckInstanceExists( string csAppName )
{
string csTmp;

HWND hWnd;
//csAppName.LoadString(IDS_APPNAME);

hWnd = FindWindow( 0, csAppName.c_str() );

if( hWnd )
	{
	BringWindowToFront( csAppName );
//	::BringWindowToTop( hWnd );
//::SetForegroundWindow( hWnd );
//::PostMessage(hWnd,WM_MAXIMIZE,0,0);
//	::ShowWindow( hWnd, SW_RESTORE );
Sleep(1000);

	Fl_Window *wndInitial = new Fl_Window(50,50,330,90);
	wndInitial->label("Possible Instance Already Running");
	
	Fl_Box *bxHeading = new Fl_Box(10,10,200, 15, "Another Window With Same Name Was Found.");
	bxHeading->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

	strpf(csTmp,"App Name: '%s'",csAppName.c_str()); 
	Fl_Box *bxAppName = new Fl_Box(10,30,200, 15,csTmp.c_str());
	bxAppName->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

	Fl_Button *btRunAnyway = new Fl_Button(25,55,130,25,"Run App Anyway");
	btRunAnyway->labelsize(12);
	btRunAnyway->callback(cb_btRunAnyway,0);

	Fl_Button *btDontRunExit = new Fl_Button(btRunAnyway->x()+btRunAnyway->w()+15,55,130,25,"Don't Run App, Exit");
	btDontRunExit->labelsize(12);
	btDontRunExit->callback(cb_btDontRunExit,0);

	wndInitial->end();
	wndInitial->show();
	wndInitial->hide();
	wndInitial->show();
	Fl::run();

	return iAppExistDontRun;
	}
else iAppExistDontRun = 0;

return iAppExistDontRun;
}

#endif















/*
//find this apps path and prefix it to supplied filename
void MakeIniPathFilename(string csFilename,string &csPathFilename)
{

//linux code
#ifndef compile_for_windows

//get the actual path this app lives in
#define MAXPATHLEN 1025   // make this larger if you need to

int length;
char fullpath[MAXPATHLEN];

// /proc/self is a symbolic link to the process-ID subdir
// of /proc, e.g. /proc/4323 when the pid of the process
// of this program is 4323.
//
// Inside /proc/<pid> there is a symbolic link to the
// executable that is running as this <pid>.  This symbolic
// link is called "exe".
//
// So if we read the path where the symlink /proc/self/exe
// points to we have the full path of the executable.


length = readlink("/proc/self/exe", fullpath, sizeof(fullpath));
 
// Catch some errors:
if (length < 0)
	{
 
	syslog(LOG_ALERT,"Error resolving symlink /proc/self/exe.\n");
	fprintf(stderr, "Error resolving symlink /proc/self/exe.\n");
	exit(0);
	}

if (length >= MAXPATHLEN)
	{
	syslog(LOG_ALERT, "Path too long. Truncated.\n");
	fprintf(stderr, "Path too long. Truncated.\n");
	exit(0);
	}

//I don't know why, but the string this readlink() function 
//returns is appended with a '@'

fullpath[length] = '\0';      // Strip '@' off the end


//printf("Full path is: %s\n", fullpath);
//syslog(LOG_ALERT,"Full path is: %s\n", fullpath);

string csTmp;

csTmp=fullpath;
size_t found=csTmp.rfind("/");
if (found!=string::npos) csPathFilename=csTmp.substr(0,found);
//syslog(LOG_ALERT,"Path only is: %s\n", csPathFilename.c_str());

csPathFilename+='/';
csPathFilename+=csFilename;	
#endif



//windows code
#ifdef compile_for_windows 
csTmp = GetCommandLine();

size_t found = csTmp.rfind( dir_seperator );
if ( found != string::npos ) csPathFilename  =csTmp.substr( 0, found );

csPathFilename += dir_seperator;
csPathFilename+=csFilename;	


csPathFilename = csFilename;
#endif

}

*/





//moded: 07-feb 2017 to ensure no vparams that are null strings are stored
//extract command line details from windows, is called by:  get_path_app_params()

//from GCCmdLine::GetAppName()
//eg: prog.com											//no path
//eg. "c:\dos\edit prog.com"							//with path\prog in quotes
//eg. "c:\dos\edit prog.com" c:\dos\junk.txt			//with path\prog in quotes and path\file
//eg. c\dos\edit.com /p /c		(as in screen-savers)	//path\prog and params no quotes

void extract_cmd_line( string cmdline, string &path, string &appname, vector<string> &vparams )
{
string stmp;
char ch;

path = "";
appname = "";
vparams.clear();
bool in_str = 0;
bool in_quote = 0;
bool beyond_app_name = 0;
//bool app_name_in_quotes = 0;

//cmdline = "c:/dos/edit prog.com";
//cmdline = "\"c:/dos/edit prog.com\" hello 123";
//cmdline = "c:/dos/edit.com hello 123";

int len =  cmdline.length();

if( len == 0 ) return;

for( int i = 0; i < len; i++ )
	{
	ch = cmdline[ i ];
	
	if( ch == '\"' )									//quote?
		{
		if( in_quote )
			{
			in_quote = 0;								//if here found closing quote
			goto got_param;
			}
		else{
			in_quote = 1;
			}
		}
	else{
		if( ch == ' ' )									//space?
			{
			if( !in_quote )				//if not in quote and space must be end of param
				{
				if( in_str ) goto got_param;
				}
			}
		else{
			in_str = 1;
			}

		if( in_str ) stmp += ch;
		}

	continue;

	got_param:

	in_str = 0;
	if ( beyond_app_name == 0 )					//store where approp
		{
		path = stmp;
		beyond_app_name = 1;
		}
	else{
		//store if not just a space
		if( stmp.compare( " " ) != 0 ) vparams.push_back( stmp );
		}

	stmp = "";
	}


//if here end of params reached, store where approp
if ( beyond_app_name == 0 )
	{
	path = stmp;
	}
else{
	if( stmp.length() != 0 ) vparams.push_back( stmp );
	}




appname = path;

len = path.length();
if( len == 0 ) return;

int pos = path.rfind( dir_seperator );

if( pos == string::npos )					//no directory path found?
	{
	path = "";	
	return;
	}

if( ( pos + 1 ) < len ) appname = path.substr( pos + 1,  pos + 1 - len );	//extract appname
path = path.substr( 0,  pos );								//extract path


//windows code
#ifdef compile_for_windows 
#endif

}



















//fixed windows version: 07-feb-2017
//find this app's path, app name and cmdline params,
//linux version uses 'argc, argc' for cmdline param extraction
//windows version uses
void get_path_app_params( string dir_sep, int argc, char **argv, string &path_out, string &app_name, vector<string> &vparams )
{
string s1, s2, path;
mystr m1;

vparams.clear();

//linux code
#ifndef compile_for_windows

//get the actual path this app lives in
#define MAXPATHLEN 1025   // make this larger if you need to

int length;
char fullpath[MAXPATHLEN];

// /proc/self is a symbolic link to the process-ID subdir
// of /proc, e.g. /proc/4323 when the pid of the process
// of this program is 4323.
//
// Inside /proc/<pid> there is a symbolic link to the
// executable that is running as this <pid>.  This symbolic
// link is called "exe".
//
// So if we read the path where the symlink /proc/self/exe
// points to we have the full path of the executable.


length = readlink("/proc/self/exe", fullpath, sizeof(fullpath));

//printf("readlink: '%s'\n", fullpath );

// Catch some errors:
if (length < 0)
	{
	syslog(LOG_ALERT,"Error resolving symlink /proc/self/exe.\n");
	fprintf(stderr, "Error resolving symlink /proc/self/exe.\n");
	exit(0);
	}

if (length >= MAXPATHLEN)
	{
	syslog(LOG_ALERT, "Path too long. Truncated.\n");
	fprintf(stderr, "Path too long. Truncated.\n");
	exit(0);
	}

//I don't know why, but the string this readlink() function 
//returns is appended with a '@'

fullpath[length] = '\0';      // Strip '@' off the end


//printf("Full path is: %s\n", fullpath);
//syslog(LOG_ALERT,"Full path is: %s\n", fullpath);

m1 = fullpath;
m1.ExtractPath( dir_sep[ 0 ], path_out );

m1 = fullpath;
m1.ExtractFilename( dir_sep[ 0 ], app_name );


for( int i = 1; i < argc; i++ )
	{
	vparams.push_back( argv[ i ] );
	}
//syslog(LOG_ALERT,"Path only is: %s\n", csPathFilename.c_str());

#endif



//windows code
#ifdef compile_for_windows 
UINT i,uiLen;                    //eg. c\dos\edit.com /p /c		(as in screen-savers)
bool bQuotes;
string csCmdLineStr;


//----------------------------
//from GCCmdLine::GetAppName()
//eg: prog.com											//no path
//eg. "c:\dos\edit prog.com"							//with path\prog in quotes
//eg. "c:\dos\edit prog.com" c:\dos\junk.txt			//with path\prog in quotes and path\file
//eg. c\dos\edit.com /p /c		(as in screen-savers)	//path\prog and params no quotes
csCmdLineStr = GetCommandLine();

//csCmdLineStr = "skeleton.exe abc";
//printf("csCmdLineStr= '%s'\n", csCmdLineStr.c_str() );

char path_appname[ MAX_PATH ];
GetModuleFileName( NULL, path_appname, MAX_PATH );

csCmdLineStr += " ";
m1 = csCmdLineStr;
m1.cut_just_past_first_find_and_keep_right( s1, " ", 0 ); 	//strip off app name to just leave params

s2 = path_appname;
s2 += " ";
s2 += s1;			//make combined pathname appname and cmdline params

//printf( "get_path_app_params() - s2: '%s'\n", s2.c_str() );

extract_cmd_line( s2, path_out, app_name, vparams );

#endif




printf( "get_path_app_params() - path_out: '%s'\n", path_out.c_str() );
printf( "get_path_app_params() - app_name: '%s'\n", app_name.c_str() );


for( int i = 0; i < vparams.size(); i++ )
	{
	printf( "get_path_app_params() - vparams[%d]: '%s'\n", i, vparams[ i ].c_str() );
	}

}




















void LoadSettings(string csIniFilename)
{
string csTmp, s1;
int x,y,cx,cy;

GCProfile p(csIniFilename);
x=p.GetPrivateProfileLONG("Settings","WinX",100);
y=p.GetPrivateProfileLONG("Settings","WinY",100);
cx=p.GetPrivateProfileLONG("Settings","WinCX",750);
cy=p.GetPrivateProfileLONG("Settings","WinCY",550);

wndMain->position( x , y );	
//wndMain->size( cx , cy );	

p.GetPrivateProfileStr("Settings","usec", "500", &s1 );
ch_wnd->fi_usec->value( s1.c_str() );
ch_wnd->fi_usec->do_callback();

p.GetPrivateProfileStr("Settings","last_save_game", "game1.txt", &slast_save_game );

if(pref_wnd!=0) pref_wnd->Load(p);
if(pref_wnd2!=0) pref_wnd2->Load(p);
if(font_pref_wnd!=0) font_pref_wnd->Load(p);

}









void SaveSettings(string csIniFilename)
{
GCProfile p(csIniFilename);


//p.WritePrivateProfileLONG("Settings","WinX", wndMain->x()-iBorderWidth);
//p.WritePrivateProfileLONG("Settings","WinY", wndMain->y()-iBorderHeight);
//p.WritePrivateProfileLONG("Settings","WinCX", wndMain->w());
//p.WritePrivateProfileLONG("Settings","WinCY", wndMain->h());



//this uses previously saved window sizing value, hopefully grabbed before window was maximized by user see: dble_wnd::resize()
//remove window border offset when saving window pos settings
p.WritePrivateProfileLONG("Settings","WinX", wndMain->restore_size_x - iBorderWidth );
p.WritePrivateProfileLONG("Settings","WinY", wndMain->restore_size_y - iBorderHeight);
p.WritePrivateProfileLONG("Settings","WinCX", wndMain->restore_size_w );
p.WritePrivateProfileLONG("Settings","WinCY", wndMain->restore_size_h );

string s1 = ch_wnd->fi_usec->value();
p.WritePrivateProfileStr("Settings","usec", s1 );

p.WritePrivateProfileStr("Settings","last_save_game", slast_save_game );

if(pref_wnd!=0) pref_wnd->Save(p);
if(pref_wnd2!=0) pref_wnd2->Save(p);
if(font_pref_wnd!=0) font_pref_wnd->Save(p);


}




void DoQuit()
{
thrd_cpu_stop();



SaveSettings(csIniFilename);

if(pref_wnd!=0) delete pref_wnd;
if(pref_wnd2!=0) delete pref_wnd2;


//windows code
#ifdef compile_for_windows
CloseHandle( h_mutex1 );							//close mutex obj
#endif

exit(0);
}









void cb_wndmain(Fl_Widget*, void* v)
{
Fl_Window* wnd = (Fl_Window*)v;

//wndMain->iconize();
//wndMain->hide();
//wndMain->show();

//do_quit_via_timer = 1;
DoQuit();

}









void cb_btAbout(Fl_Widget *, void *)
{
string s1, st;

Fl_Window *wnd = new Fl_Window(wndMain->x()+20,wndMain->y()+20,500,100);
wnd->label("About");
Fl_Input *teText = new Fl_Input(10,10,wnd->w()-20,wnd->h()-20,"");
teText->type(FL_MULTILINE_OUTPUT);
teText->textsize(12);

strpf( s1, "%s,  %s,  Built: %s\n", cnsAppWndName, "v1.03", cns_build_date );
st += s1;


strpf( s1, "\nEmulation of Prodigy chess game hardware, circa 1981" );
st += s1;

teText->value(st.c_str());

wnd->end();

#ifndef compile_for_windows
wnd->set_modal();
#endif

wnd->show();
}










/*
void cb_btOpen(Fl_Widget *, void *)
{

//char *pPathName = fl_file_chooser("Open Record Schedule File?", "*",0);
//if (!pPathName) return;

//GCProfile p(csIniFilename);
//p.WritePrivateProfileStr("Settings","LastScheduleFile",pPathName);

}
*/



void cb_open_folder(Fl_Widget *, void *)
{
string s1, s2;

s1 = "./";

if( my_dir_chooser( s2, "Select Folder?", "*",  (char*)s1.c_str(), 0, font_num, font_size ) )
	{
	cslpf( "You select folder: '%s'\n", s2.c_str() );
	}
}








void cb_open_file(Fl_Widget *, void *)
{
string s1, s2;

s1 = slast_save_game;

if( my_file_chooser( s2, "Load Game?", "*", s1.c_str(), 0, font_num, font_size ) )
	{
    mystr m1 = s2;
    m1.ExtractFilename( dir_seperator[ 0 ], s2 );
    
	printf( "cb_open_file() - You select file: '%s'\n", s2.c_str() );
	
	if ( !go_into_pause_state() )
		{
		fl_alert( "Could not go into pause, load not performed.\n" );
		return;
		}

	slast_save_game = s2;
	
	GCProfile p( slast_save_game );

	thrd_cpu->lock_mutex();

		//load cpu and ram state
		mtx.cpu->pc = p.GetPrivateProfile_hex_uint64_t( "Settings","pc", 0 );
		mtx.cpu->sp = p.GetPrivateProfile_hex_uint64_t( "Settings","sp",  0 );
		mtx.cpu->status = p.GetPrivateProfile_hex_uint64_t( "Settings","flags", 0 );
		
		p.GetPrivateProfileStr( "Settings","mem_2KB", "", &s1 );
		m1 = s1;
		int cnt = m1.LoadArray_hex_uint8_t( mtx.cpumem, cn_cpu_mem_2KB, ',' );
		printf( "cb_open_file() - mem bytes loaded: %d\n", cnt );



		//load chessboard pieces
		p.GetPrivateProfileStr( "Settings", "white_pieces", "",  &s1 );
		m1 = s1;
		vector<int> vint;
		cnt = m1.LoadVectorInts( vint, ',' );
		
		ch_wnd->vwht.clear();
		
		for( int i = 0; i < cnt; i++ )
			{
			st_piece o;
			if( (i&3) == 0 ) o.type = vint[ i ];
			if( (i&3) == 1 ) o.x = vint[ i ];
			if( (i&3) == 2 ) o.y = vint[ i ];
			if( (i&3) == 3 ) { o.sel = vint[ i ]; ch_wnd->vwht.push_back( o ); }
			}



		p.GetPrivateProfileStr( "Settings", "black_pieces", "",  &s1 );
		m1 = s1;
		cnt = m1.LoadVectorInts( vint, ',' );

		ch_wnd->vblk.clear();
		
		for( int i = 0; i < cnt; i++ )
			{
			st_piece o;
			if( (i&3) == 0 ) o.type = vint[ i ];
			if( (i&3) == 1 ) o.x = vint[ i ];
			if( (i&3) == 2 ) o.y = vint[ i ];
			if( (i&3) == 3 ) { o.sel = vint[ i ]; ch_wnd->vblk.push_back( o ); }
			}

	thrd_cpu->unlock_mutex();
	
	strpf( s1, "Game loaded, to run enter 'p' in console or press pause button:  '%s'\n", slast_save_game.c_str() );
	fl_alert( s1.c_str() );
	}
}





void cb_save_file(Fl_Widget *, void *)
{
string s1, s2;

s1 = slast_save_game;

if( my_file_chooser( s2, "Save Current Game?", "*", s1.c_str(), Fl_File_Chooser::CREATE, font_num, font_size ) )
	{
    mystr m1 = s2;
    m1.ExtractFilename( dir_seperator[ 0 ], s2 );
    
	printf( "cb_save_file() - You select file: '%s'\n", s2.c_str() );
	slast_save_game = s2;
	
	if ( !go_into_pause_state() )
		{
		fl_alert( "Could not go into pause, save not performed.\n" );
		return;
		}

	GCProfile p( slast_save_game );

	thrd_cpu->lock_mutex();

		//save cpu state
		p.WritePrivateProfile_hex_uint64_t( "Settings","pc", mtx.cpu->pc );
		p.WritePrivateProfile_hex_uint64_t( "Settings","sp", mtx.cpu->sp );
		p.WritePrivateProfile_hex_uint64_t( "Settings","flags", mtx.cpu->status );
		
		string st;
		//make memory contents a hex string
		for( int i = 0; i < cn_cpu_mem_2KB; i++ )
			{
			strpf( s1, "%02x,", mtx.cpumem[ i ] );
			st += s1;
			}
		p.WritePrivateProfileStr( "Settings","mem_2KB", st );

		//save chessboard pieces
		st = "";
		for( int i = 0; i < ch_wnd->vwht.size(); i++ )
			{
			st_piece o = ch_wnd->vwht[ i ];
			strpf( s1, "%d,%d,%d,%d,", o.type, o.x, o.y, o.sel );
			st += s1;
			}
		p.WritePrivateProfileStr( "Settings","white_pieces", st );


		st = "";
		for( int i = 0; i < ch_wnd->vblk.size(); i++ )
			{
			st_piece o = ch_wnd->vblk[ i ];
			strpf( s1, "%d,%d,%d,%d,", o.type, o.x, o.y, o.sel );
			st += s1;
			}
		p.WritePrivateProfileStr( "Settings","black_pieces", st );

	thrd_cpu->unlock_mutex();
	}

}




/*
void cb_open_2(Fl_Widget *, void *)
{
string s1;

s1 ="/mnt/home/PuppyLinux/MyPrj/MyPrj-DevC++/bkup/";

Fl_File_Chooser *fc = new Fl_File_Chooser( s1.c_str(), 0, FL_SINGLE, "Open a file" );
fc->textfont( font_num );
fc->textsize( font_size );
//fc->redraw();
fc->show();
//fc->hide();
//fc->show();
//fc->fileName->show();
}

*/






void cb_btSave(Fl_Widget *, void *)
{
//char *pPathName = fl_file_chooser("Save Record Schedule File?", "*",0);
//if (!pPathName) return;

//GCProfile p(csIniFilename);
//p.WritePrivateProfileStr("Settings","LastScheduleFile",pPathName);

}







void cb_btQuit(Fl_Widget *, void *)
{
DoQuit();
}






void cb_bt_help( Fl_Widget *, void *)
{

string pathname;
pathname = '\"';					//incase path has white spaces
pathname += app_path;
pathname += dir_seperator;
pathname += cns_open_editor;
pathname += '\"';					//incase path has white spaces
pathname += " ";
pathname += cns_help_filename;

RunShell( pathname );					//do both shell cmds

printf( "path: '%s'\n", pathname.c_str() );

}











//keep edit obj lines within spec ranges
bool cull_console_lines_at_begining( int max_lines, int min_lines )
{

if( max_lines <= 0 ) return 0;
if( min_lines < 0 ) return 0;

if( min_lines >=  max_lines )  return 0;

int char_count = tb_csl->length();

int line_count = tb_csl->count_lines( 0, char_count );

if( line_count > max_lines )
	{
	int charpos_at_line = tb_csl->skip_lines( 0, line_count - min_lines );
	tb_csl->replace( 0, charpos_at_line, "" );
	
	}
	
return 1;
}




//vt100 hints
//http://www.real-world-systems.com/docs/ANSIcode.html
//http://tldp.org/HOWTO/Bash-Prompt-HOWTO/x361.html


void vt100_clear_entire_display()
{
printf( "\x1b[2J" );
}



void vt100_clear_to_end_of_display()
{
printf( "\x1b[0J" );
}



void vt100_clear_to_end_of_line()
{
printf( "\x1b[0K" );
}



void vt100_move_cursor_up( int lines )
{
printf( "\x1b[%dA", lines );
}



void vt100_move_cursor_down( int lines )
{
printf( "\x1b[%dB", lines );
}


void vt100_move_cursor_to_horiz_pos( int pos )
{
printf( "\x1b[%dG", pos );
}





//linux code
#ifndef compile_for_windows

//refer: https://cboard.cprogramming.com/c-programming/63166-kbhit-linux.html
//needs: #include <termios.h>
int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
//printf("ch=%x\n", ch );

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
 return 0;
}
#endif






//windows code
#ifdef compile_for_windows

#include <conio.h>
int kbhit(void)
{
if (_kbhit())
	{
	char key =_getch();
	return 1;
	}    

return 0;
}

#endif
























//function prototypes
void buswr( uint16_t addr, uint8_t data );
uint8_t busrd( uint16_t addr );


string sread;											//put this before cpu() obj is defined, else you'll get a crash
string swrite;											//put this before cpu() obj is defined, else you'll get a crash
string srdwr;											//put this before cpu() obj is defined, else you'll get a crash

bool need_filter_read = 0;
int read_filter_low = -1;								//mem access display filter vals
int read_filter_high = -1;

bool need_filter_write = 0;
int write_filter_low = -1;
int write_filter_high = -1;

uint64_t mem_access_count = 0;


vector <st_cpu_operation_tag> vcpu;						//holds cpu operations, registers, and memory accesses



//mos6502 cpu( busrd, buswr );							//cpu obj, !!! MAKE SURE you call 'mos6502::init_opcodes()', straight after calling its constructor,
//cpu.init_opcodes()

//uint8_t cpumem[ cn_cpu_mem_size ];						//cpu memory
//int trig_cnt = 0;




//flags     7   6   5   4   3   2   1   0
//          N   V   -   B   D   I   Z   C

void show_flags( string &ss )
{
string s1;

ss = "";

if( mtx.cpu->status & 0x80 ) { printf("N " ); strpf( s1, "N " ); }
else  { printf("n " ); strpf( s1, "n " ); }
ss += s1;

if( mtx.cpu->status & 0x40 ) { printf("V " ); strpf( s1, "V " ); }
else  { printf("v " ); strpf( s1, "v " ); }
ss += s1;

printf("- " ); strpf( s1, "- " );
ss += s1;

if( mtx.cpu->status & 0x10 ) { printf("B " ); strpf( s1, "B " ); }
else  { printf("b " ); strpf( s1, "b " ); }
ss += s1;

if( mtx.cpu->status & 0x08 ) { printf("D " ); strpf( s1, "D " ); }
else  { printf("d " ); strpf( s1, "d " ); }
ss += s1;

if( mtx.cpu->status & 0x04 ) { printf("I " ); strpf( s1, "I " ); }
else  { printf("i " ); strpf( s1, "i " ); }
ss += s1;

if( mtx.cpu->status & 0x02 ) { printf("Z " ); strpf( s1, "Z " ); }
else  { printf("z " ); strpf( s1, "z " ); }
ss += s1;

if( mtx.cpu->status & 0x01 ) { printf("C " ); strpf( s1, "C " ); }
else  { printf("c " ); strpf( s1, "c " ); }
ss += s1;
}








//flags     7   6   5   4   3   2   1   0
//          N   V   -   B   D   I   Z   C

void show_flags2( unsigned int status, string &ss )
{
string s1;

ss = "";

if( status & 0x80 ) { strpf( s1, "N " ); }
else  { strpf( s1, "n " ); }
ss += s1;

if( status & 0x40 ) { strpf( s1, "V " ); }
else  { strpf( s1, "v " ); }
ss += s1;

strpf( s1, "- " );
ss += s1;

if( status & 0x10 ) { strpf( s1, "B " ); }
else  { strpf( s1, "b " ); }
ss += s1;

if( status & 0x08 ) { strpf( s1, "D " ); }
else  { strpf( s1, "d " ); }
ss += s1;

if( status & 0x04 ) { strpf( s1, "I " ); }
else  { strpf( s1, "i " ); }
ss += s1;

if( status & 0x02 ) { strpf( s1, "Z " ); }
else  { strpf( s1, "z " ); }
ss += s1;

if( status & 0x01 ) { strpf( s1, "C " ); }
else  { strpf( s1, "c " ); }
ss += s1;
}



int last2_seg3;
int last2_seg2;
int last2_seg1;
int last2_seg0;



void update_7seg2( int which, int serial_data )
{
seg7_wdg *o;
int serial_fix = serial_data;

o = 0;

if( which == 0 ) o = ch_wnd->wdg_seg0;
if( which == 1 ) o = ch_wnd->wdg_seg1;
if( which == 2 ) o = ch_wnd->wdg_seg2;
if( which == 3 ) o = ch_wnd->wdg_seg3;


if( o == 0 ) return;


bool draw0, draw1, draw2, draw3;

draw0 = draw1 = draw2 = draw3 = 0;


//the display's 'serial_data' seems to be interleved with 50% seg cathode drive and 50% no drive (0x00), maybe for dimming purposes,
//below code uses a simple history mechanism to filter out the interleaving, thus attempting to keep 100% seg cathode drive(which avoids flickering). 
//However when there are 100% 0x00 (i.e: 2 consecutive: 0x00) which would indicate that a seg7 display actually needs to be blank, it does allow a
//0x00 to get to the appropriate display's seg cathodes.
if( which == 0 )
	{
	if( ( last2_seg0 == 0 ) & ( serial_data == 0 ) ) draw0 = 1;
	if( ( last2_seg0 == 0 ) & ( serial_data != 0 ) ) draw0 = 1; 
	if( ( last2_seg0 != 0 ) & ( serial_data == 0 ) ) { draw0 = 1; serial_fix = last2_seg0; }
	if( ( last2_seg0 != 0 ) & ( serial_data != 0 ) ) draw0 = 1;
//draw0 = 1;
	}

if( which == 1 )
	{
	if( ( last2_seg1 == 0 ) & ( serial_data == 0 ) ) draw1 = 1;
	if( ( last2_seg1 == 0 ) & ( serial_data != 0 ) ) draw1 = 1; 
	if( ( last2_seg1 != 0 ) & ( serial_data == 0 ) ) { draw1 = 1; serial_fix = last2_seg1; }
	if( ( last2_seg1 != 0 ) & ( serial_data != 0 ) ) draw1 = 1;
//draw1 = 1;
	}

if( which == 2 )
	{
	if( ( last2_seg2 == 0 ) & ( serial_data == 0 ) ) draw2 = 1;
	if( ( last2_seg2 == 0 ) & ( serial_data != 0 ) ) draw2 = 1; 
	if( ( last2_seg2 != 0 ) & ( serial_data == 0 ) ) { draw2 = 1; serial_fix = last2_seg2; }
	if( ( last2_seg2 != 0 ) & ( serial_data != 0 ) ) draw2 = 1;
//draw2 = 1;
	}

if( which == 3 )
	{
	if( ( last2_seg3 == 0 ) & ( serial_data == 0 ) ) draw3 = 1;
	if( ( last2_seg3 == 0 ) & ( serial_data != 0 ) ) draw3 = 1; 
	if( ( last2_seg3 != 0 ) & ( serial_data == 0 ) ) { draw3 = 1; serial_fix = last2_seg3; }
	if( ( last2_seg3 != 0 ) & ( serial_data != 0 ) ) draw3 = 1;
//draw3 = 1;
	}

o->diode_a = 0;
o->diode_b = 0;
o->diode_c = 0;
o->diode_d = 0;
o->diode_e = 0;
o->diode_f = 0;
o->diode_g = 0;
o->diode_dpl = 0;
o->diode_dpr = 0;

//map serial to diode segments
if( serial_fix & 0x1 ) o->diode_a = 1;
if( serial_fix & 0x2 ) o->diode_b = 1;
if( serial_fix & 0x4 ) o->diode_c = 1;
if( serial_fix & 0x8 ) o->diode_d = 1;
if( serial_fix & 0x10 ) o->diode_e = 1;
if( serial_fix & 0x20 ) o->diode_f = 1;
if( serial_fix & 0x40 ) o->diode_g = 1;

if( draw0 == 1 )
	{
	o->redraw();
	last2_seg0 = serial_data;
	}

if( draw1 == 1 )
	{
	o->redraw();
	last2_seg1 = serial_data;
	}

if( draw2 == 1 )
	{
	o->redraw();
	last2_seg2 = serial_data;
	}

if( draw3 == 1 )
	{
	o->redraw();
	last2_seg3 = serial_data;
	}

}




int last_seg3;
int last_seg2;
int last_seg1;
int last_seg0;


//7 segment demultiplexer 
//'which' is like the common anode of seg7 displays,
//'serial_data' is like the seg7 segment cathodes (A-G), however 'serial_data' here is positive logic, so a 1 means a led should be on
void update_7seg( int which, int serial_data )
{
seg7_wdg *o;
int serial_fix = serial_data;

o = 0;

if( which == 0 ) o = wdg_seg0;
if( which == 1 ) o = wdg_seg1;
if( which == 2 ) o = wdg_seg2;
if( which == 3 ) o = wdg_seg3;


if( o == 0 ) return;

bool draw0, draw1, draw2, draw3;

draw0 = draw1 = draw2 = draw3 = 0;


//the display's 'serial_data' seems to be interleved with 50% seg cathode drive and 50% no drive (0x00), maybe for dimming purposes,
//below code uses a simple history mechanism to filter out the interleaving, thus attempting to keep 100% seg cathode drive(which avoids flickering). 
//However when there are 100% 0x00 (i.e: 2 consecutive: 0x00) which would indicate that a seg7 display actually needs to be blank, it does allow a
//0x00 to get to the appropriate display's seg cathodes.
if( which == 0 )
	{
	if( ( last_seg0 == 0 ) & ( serial_data == 0 ) ) draw0 = 1;
	if( ( last_seg0 == 0 ) & ( serial_data != 0 ) ) draw0 = 1; 
	if( ( last_seg0 != 0 ) & ( serial_data == 0 ) ) { draw0 = 1; serial_fix = last_seg0; }
	if( ( last_seg0 != 0 ) & ( serial_data != 0 ) ) draw0 = 1;
//draw0 = 1;
	}

if( which == 1 )
	{
	if( ( last_seg1 == 0 ) & ( serial_data == 0 ) ) draw1 = 1;
	if( ( last_seg1 == 0 ) & ( serial_data != 0 ) ) draw1 = 1; 
	if( ( last_seg1 != 0 ) & ( serial_data == 0 ) ) { draw1 = 1; serial_fix = last_seg1; }
	if( ( last_seg1 != 0 ) & ( serial_data != 0 ) ) draw1 = 1;
//draw1 = 1;
	}

if( which == 2 )
	{
	if( ( last_seg2 == 0 ) & ( serial_data == 0 ) ) draw2 = 1;
	if( ( last_seg2 == 0 ) & ( serial_data != 0 ) ) draw2 = 1; 
	if( ( last_seg2 != 0 ) & ( serial_data == 0 ) ) { draw2 = 1; serial_fix = last_seg2; }
	if( ( last_seg2 != 0 ) & ( serial_data != 0 ) ) draw2 = 1;
//draw2 = 1;
	}

if( which == 3 )
	{
	if( ( last_seg3 == 0 ) & ( serial_data == 0 ) ) draw3 = 1;
	if( ( last_seg3 == 0 ) & ( serial_data != 0 ) ) draw3 = 1; 
	if( ( last_seg3 != 0 ) & ( serial_data == 0 ) ) { draw3 = 1; serial_fix = last_seg3; }
	if( ( last_seg3 != 0 ) & ( serial_data != 0 ) ) draw3 = 1;
//draw3 = 1;
	}

o->diode_a = 0;
o->diode_b = 0;
o->diode_c = 0;
o->diode_d = 0;
o->diode_e = 0;
o->diode_f = 0;
o->diode_g = 0;
o->diode_dpl = 0;
o->diode_dpr = 0;

//map serial to diode segments
if( serial_fix & 0x1 ) o->diode_a = 1;
if( serial_fix & 0x2 ) o->diode_b = 1;
if( serial_fix & 0x4 ) o->diode_c = 1;
if( serial_fix & 0x8 ) o->diode_d = 1;
if( serial_fix & 0x10 ) o->diode_e = 1;
if( serial_fix & 0x20 ) o->diode_f = 1;
if( serial_fix & 0x40 ) o->diode_g = 1;

if( draw0 == 1 )
	{
	o->redraw();
	last_seg0 = serial_data;
	}

if( draw1 == 1 )
	{
	o->redraw();
	last_seg1 = serial_data;
	}

if( draw2 == 1 )
	{
	o->redraw();
	last_seg2 = serial_data;
	}

if( draw3 == 1 )
	{
	o->redraw();
	last_seg3 = serial_data;
	}

if( which == 1 )
	{
	}

if( which == 1 )
	{
	}

if( which == 2 )
	{
	}

if( which == 3 )
	{
	}

//if( which == 3 ) printf("which: %d, %02x\n", which, serial_data );
//if( which == 2 ) printf("which: %d, %02x\n", which, serial_data );

//update_7seg2( which, serial_data );
}








int imin = 0;
int imax = 0;



int sw_matrix_butt[ cn_outs ][ cn_ins ];			//switch matrix for buttons, 1 on short of and 'out' to an 'in'
int sw_matrix_brd[ cn_outs ][ cn_ins ];
int sw_matrix_cpanel[ cn_outs ][ cn_ins ];


bool vb_csl = 1;


int via_200a;								//last via port write values
int via_2000;


int coord_led_x;							//2000<-04  via_200a		the letters: Ax...Hx
int coord_led_y;							//2000<-05  via_200a		the numbers: x1...x8


bool wrote_to_200a;							//this helps remove led muxing duty cycle blanking thats coded in rom, its objectional for emulation,
											//the duty cycle blanking would have helped reduce led flicker and current consumption i'd expect


//write callback
void buswr( uint16_t addr, uint8_t data )
{
string s1;

mem_access_count++;

mtx.cpumem[ addr ] = data;


//via specific
if( addr == 0x2000 )		//7 seg common anode drive, 00Hx 03Hx ORB VIA 4 outputs  PB3-PB0 probably go to SN74145 (4 to ten decoder [bcd to decimal:0-9] )
	{
//	if( data < imin ) imin = data;
//	if( data > imax ) imax = data;
//	printf("imin: %d, imax: %d\n", imin, imax );
	
	
	via_2000 = data;
	
	mtx.via_2000_changed = 1;


	if( wrote_to_200a )								//used to remove duty cycle led blanking
		{
		wrote_to_200a = 0;
		//4 to 10 dec: 74145
		switch( data )
			{
			case 0:
				mtx.anode00 = via_200a;				//7 seg, store cathode bit pattern for use when driving these led anodes
			break;
			
			case 1:
				mtx.anode01 = via_200a;
			break;
			
			case 2:
				mtx.anode02 = via_200a;
			break;
			
			case 3:
				mtx.anode03 = via_200a;
			break;
			
			case 4:
				mtx.anode04 = via_200a;				//led coords for x (letters), store cathode bit pattern for use when driving these led anodes
			break;

			case 5:
				mtx.anode05 = via_200a;				//led coords for y (numbers)
			break;

			case 6:
				printf("via_2000 = 6\n" );
			break;

			case 7:
				printf("via_2000 = 7\n" );
			break;

			case 8:
				printf("via_2000 = 8\n" );
			break;

			case 9:
//				printf("via_2000 = 9\n" );
				mtx.anode09 = via_200a;				//led coords for y (numbers)
			break;

			default:
			break;
			}

//	strpf( s1, "mtx.anode09 <- 0x%02x\n", mtx.anode09);
//	if( mtx.anode09 != 0x00 ) mtx.vcls3.push_back( s1 );

		}


	switch( data )
		{
		case 0:
			mtx.drv_out = 0;
		break;

		case 1:
			mtx.drv_out = 1;
		break;

		case 2:
			mtx.drv_out = 2;
		break;

		case 3:
			mtx.drv_out = 3;
		break;

		case 4:
			mtx.drv_out = 4;
		break;

		case 5:
			mtx.drv_out = 5;
		break;

		case 6:
			mtx.drv_out = 6;
		break;

		case 7:
			mtx.drv_out = 7;
		break;

		case 8:
			mtx.drv_out = 8;
//				printf("via_2000 = 8, via_200a: %d\n", via_200a );
		break;

		case 9:
			mtx.drv_out = 9;
//				printf("via_2000 = 9, via_200a: %d\n", via_200a );
		break;

		default:
		break;
		}



//	if( ( data >= 0 ) & ( data <= 5 ) ) 					//7 seg anodes
//		{
//		printf("via_2000: %d\n", via_2000);
//		printf("%04x<-%02x\n", addr, data );

//		printf("%04x<-%02x\n", addr, data );

//		update_7seg( via_2000, via_200a );
//		update_7seg2( via_2000, via_200a );
//		}


//	if( ( data >= 4 ) & ( data <= 4 ) )						//coord leds (letters)
//		{
//		}



//	if( ( data >= 5 ) & ( data <= 5 ) )						//coord leds (numbers)
//		{
//--		strpf( s1, "%04x<-%02x 200a: %x\n", addr, data, via_200a );
//--		if(vb_csl) wdg_csl3->add( s1 );
//
//		coord_led_y = via_200a;
//		}

//printf("via_2000<- %d\n", data);
//	strpf( s1, "via_2000 <- 0x%02x\n", data);
//	if( data & 0x80 ) mtx.vcls3.push_back( s1 );
	
	goto chk_verbose;
	}





if( addr == 0x2005 )										//VIA timer1 period, sets ptich produced by piezo speaker's through PB7
	{
	mtx.via_wr_0x2005 = data;
	
	if( mtx.via_PB7 == 0 ) 
		{
		
		}
		
//	printf("via_wr_0x2005: %d\n", mtx.via_wr_0x2005 );
	goto chk_verbose;
	}

if( addr == 0x200b )										//VIA PB7 enable, for piezo speaker to be driven by timer 1's period					
	{
	mtx.via_wr_0x200b = data;
	
	if( ( mtx.via_PB7 == 0 ) && ( mtx.via_wr_0x200b & 0x80 ) )
		{
		mtx.via_PB7 = 1;
		mtx.via_timer1_period = mtx.via_wr_0x2005;
		}
	
//	printf("via_wr_0x200b: %x\n", mtx.via_wr_0x200b );
	goto chk_verbose;
	}






if( addr == 0x200a )
	{
	wrote_to_200a = 1;								//used to remove duty cycle led blanking
	
	via_200a = data;

	
//printf("via_200a<- %d\n", data);

//	strpf( s1, "via_200a <- 0x%02x\n", data);
//	if( data & 0x80 ) mtx.vcls3.push_back( s1 );

//strpf( s1, "via_200a <- 0x%02x\n", via_200a);
//if( via_200a > 0x7f ) printf( "via_200a <- 0x%02x\n", via_200a);
	goto chk_verbose;
	}











chk_verbose:

if( vb_csl ) 
	{
//	bool use_filt = 1;
	bool filt_pass = 1;
	if( need_filter_read ) filt_pass = 0;		//assume no writes to be shown when a read filter is active, unless a write filter has also been specified

	if( ( write_filter_low != -1 ) && ( write_filter_high != -1 ) ) need_filter_write = 1;
	else need_filter_write = 0;


//	if( write_filter_low == -1 ) use_filt = 0;
//	if( write_filter_high == -1 ) use_filt = 0;

	if( need_filter_write )
		{
		if( ( addr >= write_filter_low ) & ( addr <= write_filter_high ) ) filt_pass = 1;
		else filt_pass = 0;
		}
		
	if( filt_pass )
		{
		st_cpu_operation_tag o;

		o.mem_access_count = mem_access_count;
		o.pc = mtx.cpu->pc;
		o.rd_wr = 1;
		o.addr = addr;
		o.data = data;
		o.flags = mtx.cpu->status;

		if( vb_csl ) vcpu.push_back( o );

	//	printf(    "addr_wr: %04x, %02x -------------------  ", addr, data );
	//	strpf( s1, "addr_wr: %04x, %02x -------------------  ", addr, data );
	//	swrite += s1;

	//	show_flags( s1 );
	//	swrite += s1;

	//	printf("\n" );
	//	strpf( s1, "\n" );
	//	swrite += s1;

	//	srdwr += swrite;
		}
	}
}











//read callback
uint8_t busrd( uint16_t addr )
{
uint8_t data;
string s1;

mem_access_count++;

data = mtx.cpumem[ addr ];

//printf( "busrd(): addr: [0x%04x] 0x%02x", addr, data );		
//getchar();




if( mtx.via_2000_changed )
	{
	mtx.via_2000_changed = 0;
	
	if( ch_wnd )
		{
		int mx = ch_wnd->mouse_coord_x;				//get chess brd coords
		int my = ch_wnd->mouse_coord_y;
		if( ch_wnd->left_pressed )
			{
			sw_matrix_brd[ mx ][ my ] = 1;			//e.g: [0][0] = a1, [0][1] = a2
													//e.g: [1][0] = b1, [1][1] = b2
			}
		else{
			sw_matrix_brd[ mx ][ my ] = 0;
			}
		}


	mtx.dat2000 = 0xff;									//data for VIA port PB
	mtx.dat2001 = 0xff;									//data for VIA port PA

	for( int ii = 0; ii < cn_ins; ii++ ) 				//cycle switch matrix ins
		{
		if( ( sw_matrix_butt[ mtx.drv_out ][ ii ] ) || ( sw_matrix_brd[ mtx.drv_out ][ ii ] ) || ( sw_matrix_cpanel[ mtx.drv_out ][ ii ] ) )		//is there a switch on?
			{
			switch( ii )
				{
				case 0:
					mtx.dat2001 &= 0xfe;
				break;

				case 1:
					mtx.dat2001 &= 0xfd;
				break;

				case 2:
					mtx.dat2001 &= 0xfb;
				break;

				case 3:
					mtx.dat2001 &= 0xf7;
				break;

				case 4:
					mtx.dat2001 &= 0xef;
				break;

				case 5:
					mtx.dat2001 &= 0xdf;
				break;

				case 6:
					mtx.dat2001 &= 0xbf;
				break;

				case 7:
					mtx.dat2001 &= 0x7f;
				break;

				case 8:
					mtx.dat2000 &= 0xef;
				break;

				case 9:
					mtx.dat2000 &= 0xdf;
				break;

				case 10:
					mtx.dat2000 &= 0xbf;
				break;

				default:
				break;
				}

//	printf(  "dat2000 %02x  dat2001 %02x\n", mtx.dat2000, mtx.dat2001 );			


			}
		}
	}

if( addr == 0x2000 )
	{
	data = mtx.dat2000;
	}

if( addr == 0x2001 )
	{
	data = mtx.dat2001;
	}



if( vb_csl ) 
	{
//	bool use_filt = 1;

	bool filt_pass = 1;
	if( need_filter_write ) filt_pass = 0;		//assume no reads to be shown when a write filter is active, unless a read filter has also been specified
	
	if( ( read_filter_low != -1 ) && ( read_filter_high != -1 ) ) need_filter_read = 1;
	else need_filter_read = 0;
	
//	if( read_filter_low == -1 ) use_filt = 0;
//	if( read_filter_high == -1 ) use_filt = 0;

	if( need_filter_read )
		{
		if( ( addr >= read_filter_low ) & ( addr <= read_filter_high ) ) filt_pass = 1;
		else filt_pass = 0;
		}

	if( filt_pass )
		{

		st_cpu_operation_tag o;

		o.mem_access_count = mem_access_count;

		//	printf(    "5757 mtx.cpu %x\n", mtx.cpu );

		o.pc = mtx.cpu->pc;
		o.rd_wr = 0;
		o.addr = addr;
		o.data = data;
		o.flags = mtx.cpu->status;
		o.A = mtx.cpu->A;
		o.X = mtx.cpu->X;
		o.Y = mtx.cpu->Y;

		vcpu.push_back( o );

		//	printf(    "addr_rd: %04x, %02x -- A:%02x, X:%02x, Y:%02x  ", addr, data, cpu.A, cpu.X, cpu.Y );
		//	strpf( s1, "addr_rd: %04x, %02x -- A:%02x, X:%02x, Y:%02x  ", addr, data, cpu.A, cpu.X, cpu.Y );
		//	sread += s1;

		//	show_flags( s1 );
		//	sread += s1;

		//	printf("\n" );
		//	strpf( s1, "\n" );
		//	sread += s1;

		//	srdwr += sread;
		}
	}


return data;
}





void cpu_str( int idx1, int idx2, string &ss )
{
string s1;

ss = "";

if( idx1 < 0 ) idx1 = 0;
if( idx2 < 0 ) idx2 = 0;

//printf("idx1: %d, idx2: %d, vcpu.size(): %d\n", idx1, idx2, vcpu.size() );

if( idx1 >= vcpu.size() ) return;
if( idx2 >= vcpu.size() ) return;


if( idx1 > idx2 ) return;

st_cpu_operation_tag o;

//printf("here1\n" );

for( int i = idx1; i <= idx2; i++ )
	{
	o = vcpu[ i ];

	if( o.rd_wr == 0 )
		{
		strpf( s1, "%016" PRIu64 " rd: %04x->%02x - PC: %04x, A:%02x, X:%02x, Y:%02x  ", o.mem_access_count, o.addr, o.data, o.pc, o.A, o.X, o.Y );
		ss += s1;

		show_flags2( o.flags, s1 );
		ss += s1;
		}
	else{
		strpf( s1, "%016" PRIu64 " wr: %04x<-%02x - PC: %04x, ----------------  ", o.mem_access_count, o.addr, o.data, o.pc );
		ss += s1;

		show_flags2( o.flags, s1 );
		ss += s1;
		}

	strpf( s1, "\n" );
	ss += s1;
	}
//printf("ss: '%s'\n", ss.c_str() );
}









int load_6502( string srom )
{
string s1;

mtx.rom_loaded = 0;

for( int i = 0; i < cn_cpu_mem_size; i++ )
	{
	mtx.cpumem[ i ] = 0x0;
	}

FILE *fp = fopen( srom.c_str(), "rb" );

if( !fp ) 
	{
	strpf( s1, "Prodigy - Can't open file for reading: '%s'\n", srom.c_str() );
	if( wdg_csl ) wdg_csl->add( s1 );
	
	fl_alert( s1.c_str() );
	return 0;
	}

int rom_size = 0x2000;	//8192
int rom_addr = 0x6000;

int read = fread( mtx.cpumem + rom_addr, 1, rom_size, fp );

if( read != rom_size ) 
	{
	printf( "Read mismatch size (%d, needed: %d), file: '%s'\n", read, rom_size, srom.c_str() );
	return 0;
	}

fclose( fp );

printf( "Read %d bytes: '%s'\n", read, srom.c_str() );


//place rom image at 0xe000 using rom at 0x6000
for( int i = 0; i < rom_size; i++ )
	{
	mtx.cpumem[ i + 0xe000 ] = mtx.cpumem[ i + rom_addr ];
	}

mtx.rom_loaded = 1;

//cpumem[ 0xfffc ] = 0xb7;				//reset vector
//cpumem[ 0xfffd ] = 0x60;

//cpumem[ 0x60b7 ] = 0xea;				//nop


//cpu.Reset();

//cpu.Run( 2000 );

//printf( "--------------------Trig IRQ------------------------\n" );
//getchar();

//trig_cnt = 1;
//cpu.IRQ();
//cpu.Run( 500 );

return 1;
}








void help()
{
string s1;

s1 =   "---------------------- help ------------------------\n";
wdg_csl->add( s1 );

s1 = "---> xx/yy are hex nums, dd is decimal number <---\n";
wdg_csl->add( s1 );
s1 = "---> leave nums blank to query their vals <---\n\n";
wdg_csl->add( s1 );

s1 = "cl           - clear all\n";
wdg_csl->add( s1 );

s1 = "bo           - reboot\n";
wdg_csl->add( s1 );

s1 = "ss           - show settings\n";
wdg_csl->add( s1 );

s1 = "ex xxxx      - examine mem addr\n";
wdg_csl->add( s1 );

s1 = "wr xxxx yy   - write mem addr\n";
wdg_csl->add( s1 );

s1 = "st ddddddd   - step xxxx times (1 to 9999999)\n";
wdg_csl->add( s1 );

s1 = "bp xxxx      - breakpoint at xxxx, use -1 to disable\n";
wdg_csl->add( s1 );

//s1 = "wa xxxx      - watch access at xxxx, show\n";
//wdg_csl->add( s1 );

s1 = "fr xxxx yyyy - filt read addr range, xxxx<=yyyy (-1 is off)\n";
wdg_csl->add( s1 );

s1 = "fw xxxx yyyy - filt write addr range, xxxx<=yyyy (-1 is off)\n";
wdg_csl->add( s1 );

s1 = "df fname.txt - dump captured data to file using spec filename\n";
wdg_csl->add( s1 );

s1 = "ds file.txt  - dump stepsz, bp and filt vals to spec filename\n";
wdg_csl->add( s1 );

s1 = "ls file.txt  - load stepsz, bp and filt vals from spec filename\n";
wdg_csl->add( s1 );

s1 = "vb           - toggle verbose mode\n";
wdg_csl->add( s1 );

s1 = "pc           - show program counter contents\n";
wdg_csl->add( s1 );

s1 = "sp           - show stack pointer contents\n";
wdg_csl->add( s1 );

s1 = "a            - show accumulator contents\n";
wdg_csl->add( s1 );

s1 = "x            - show X reg contents\n";
wdg_csl->add( s1 );

s1 = "y            - show Y reg contents\n";
wdg_csl->add( s1 );

s1 = "f            - show status reg flags\n";
wdg_csl->add( s1 );

//   "wr xxxx yy   - write mem addr\n"
s1 = "p            - pause toggle\n";
wdg_csl->add( s1 );

s1 = "s            - step once only\n";
wdg_csl->add( s1 );

s1 = "t            - trigger irq\n";
wdg_csl->add( s1 );


s1 = "----------------------------------------------------\n";
wdg_csl->add( s1 );

}















void load_all_settings( string fname )
{
string s1, s2;
mystr m1;

strpf( s2, "loading settings: '%s'\n", fname.c_str() );
wdg_csl->add( s2 );

if( m1.readfile( fname, 10 ) )
	{
	s2 = m1.str();
	wdg_csl->add( "--file read completed--\n" );
	sscanf( s2.c_str(), "%d %x %x %x %x %x", &run_step_size, &bp_addr, &read_filter_low, &read_filter_high, &write_filter_low, &write_filter_high );

	strpf( s1, "step size:         %d\n", run_step_size );
	wdg_csl->add( s1 );
	strpf( s1, "breakpoint:        %04x\n", bp_addr );
	wdg_csl->add( s1 );
	strpf( s1, "read filter low:   %04x\n", read_filter_low );
	wdg_csl->add( s1 );
	strpf( s1, "read filter high:  %04x\n", read_filter_high );
	wdg_csl->add( s1 );
	strpf( s1, "write filter low:  %04x\n", write_filter_low );
	wdg_csl->add( s1 );
	strpf( s1, "write filter high: %04x\n", write_filter_high );
	wdg_csl->add( s1 );
	}
else{
	wdg_csl->add( "--file read failed--\n" );
	}
}














void update_leds()
{
string s1;

uint8_t data = via_2000;


update_7seg( 0, mtx.anode00 );
update_7seg( 1, mtx.anode01 );
update_7seg( 2, mtx.anode02 );
update_7seg( 3, mtx.anode03 );

update_7seg2( 0, mtx.anode00 );
update_7seg2( 1, mtx.anode01 );
update_7seg2( 2, mtx.anode02 );
update_7seg2( 3, mtx.anode03 );


//if( ( data >= 4 ) & ( data <= 4 ) )						//coord leds (letters)
	{
//	strpf( s1, "%04x<-%02x 200a: %x\n", addr, data, via_200a );
	
//if(vb_csl) wdg_csl3->add( s1 );
	

	//letter leds, update chess brd leds
	coord_led_x = mtx.anode04;
	
	ch_wnd->set_led_x( 0, 0 );
	ch_wnd->set_led_x( 1, 0 );
	ch_wnd->set_led_x( 2, 0 );
	ch_wnd->set_led_x( 3, 0 );
	ch_wnd->set_led_x( 4, 0 );
	ch_wnd->set_led_x( 5, 0 );
	ch_wnd->set_led_x( 6, 0 );
	ch_wnd->set_led_x( 7, 0 );

	if( coord_led_x == 0x01 ) ch_wnd->set_led_x( 0, 1 );
	if( coord_led_x == 0x02 ) ch_wnd->set_led_x( 1, 1 );
	if( coord_led_x == 0x04 ) ch_wnd->set_led_x( 2, 1 );
	if( coord_led_x == 0x08 ) ch_wnd->set_led_x( 3, 1 );
	if( coord_led_x == 0x10 ) ch_wnd->set_led_x( 4, 1 );
	if( coord_led_x == 0x20 ) ch_wnd->set_led_x( 5, 1 );
	if( coord_led_x == 0x40 ) ch_wnd->set_led_x( 6, 1 );
	if( coord_led_x == 0x80 ) ch_wnd->set_led_x( 7, 1 );



	//number leds, update chess brd leds
	coord_led_y = mtx.anode05;
	
	ch_wnd->set_led_y( 0, 0 );
	ch_wnd->set_led_y( 1, 0 );
	ch_wnd->set_led_y( 2, 0 );
	ch_wnd->set_led_y( 3, 0 );
	ch_wnd->set_led_y( 4, 0 );
	ch_wnd->set_led_y( 5, 0 );
	ch_wnd->set_led_y( 6, 0 );
	ch_wnd->set_led_y( 7, 0 );

	if( coord_led_y == 0x01 ) ch_wnd->set_led_y( 7, 1 );
	if( coord_led_y == 0x02 ) ch_wnd->set_led_y( 6, 1 );
	if( coord_led_y == 0x04 ) ch_wnd->set_led_y( 5, 1 );
	if( coord_led_y == 0x08 ) ch_wnd->set_led_y( 4, 1 );
	if( coord_led_y == 0x10 ) ch_wnd->set_led_y( 3, 1 );
	if( coord_led_y == 0x20 ) ch_wnd->set_led_y( 2, 1 );
	if( coord_led_y == 0x40 ) ch_wnd->set_led_y( 1, 1 );
	if( coord_led_y == 0x80 ) ch_wnd->set_led_y( 0, 1 );
	}
}












//piezo speaker
void update_beep()
{
mystr m1;


if( mtx.via_PB7 )
	{
	if( mtx.via_timer1_period == 0x02 ) beep( cn_beep_freq2, 1, cn_beep_delay );
	else beep( cn_beep_freq1, 1, cn_beep_delay );
	
//printf("via_wr_0x2005: %x\n", mtx.via_timer1_period );
//	m1.delay_ms( 500 );
	mtx.via_PB7 = 0;
//		mtx.via_wr_0x200b = data;
	}
}







int irq_timer = 0;

int disp_cnt = 0;


bool bflash = 0;


void cb_timer1(void *)
{
string s1, s2, st;
mystr m1;

Fl::repeat_timeout( 0.1, cb_timer1 );

bool bdisp = 0;
disp_cnt++;
if( !(disp_cnt%10 ) ) bdisp = 1;


bool bdisp2 = 0;
if( !(disp_cnt%3 ) ) bdisp2 = 1;
if(bdisp2) ch_wnd->tick();

if( !(disp_cnt%3 ) ) bflash = !bflash;

if( bfirst_run )
	{
	
	thrd_cpu->lock_mutex();

/*	
		if( !load_6502( cns_prodigy_rom ) ) exit(0);

		bfirst_run = 0;

		mtx.cpu->Reset();
		mtx.via_wr_0x2000 = 0;
		mtx.via_wr_0x200a = 0;
		mtx.cnt_via_rd_0x2000 = 0;
		mtx.cnt_via_rd_0x2001 = 0;
		mtx.dat2000 = 0xff;											//data for VIA port PB
		mtx.dat2001 = 0xff;											//data for VIA port PA
		mtx.illegal_opcode_pc = -1;


		trig_cnt = 1;
*/
/*
		for( int oo = 0; oo < cn_outs; oo++ )
			{
			for( int ii = 0; ii < cn_ins; ii++ )
				{
				sw_matrix_butt[ oo ][ ii ] = 0;					//assume all switches open
				sw_matrix_brd[ oo ][ ii ] = 0;
				sw_matrix_cpanel[ oo ][ ii ] = 0;
				}
			}
*/
	thrd_cpu->unlock_mutex();
	}




thrd_cpu->lock_mutex();

//	process_switch_changes();





	if( mtx.illegal_opcode_pc > -1 )
		{
//		pause_state = 2;
		
		
		strpf( s1, "Illegal opcode encountered, PC: %04x, data: %02x\n", mtx.illegal_opcode_pc, mtx.cpumem[ mtx.illegal_opcode_pc ] );
		wdg_csl3->add( s1 );	
		mtx.illegal_opcode_pc = -1;
		}




//	printf( "mtx.via_wr_0x200a: %02x\n", mtx.via_wr_0x200a );
thrd_cpu->unlock_mutex();



update_leds();

update_beep();


//if ( vt100_count == 50 ) vt100_entire_display();

vt100_count++;
//printf( "\x1b[7A\x1b[1;34m BASH \1b[7B\e[6D" );		//prints BASH in blue bold


int addr, data, inum;
bool got_cmd = 0;


thrd_cpu->lock_mutex();
	if(bdisp) wdg_csl->tick( 0 );					//flash cursor

	if( mtx.vcls3.size() != 0 )
		{
		for( int i = 0; i < mtx.vcls3.size(); i++ )
			{
			wdg_csl3->add( mtx.vcls3[i] );	
			}
		mtx.vcls3.clear();
		}
thrd_cpu->unlock_mutex();

//wdg_csl->save_cursor();

//if( ( pause_state == 0 ) | ( need_update ) )

if( pause_state > 0 ) ch_wnd->paused = 1;
else  ch_wnd->paused = 0;


wdg_csl->save_cursor();
wdg_csl->move_cursor( 31, 0 );



if( pause_state == 0 )
	{
			   strpf( s1, "    running - cpu cycles  : %020" PRIu64 "", mtx.cpu->cycles );
	}
else{
	if(bflash) strpf( s1, "            - cpu cycles  : %020" PRIu64 "", mtx.cpu->cycles );
	else       strpf( s1, "    paused  - cpu cycles  : %020" PRIu64 "", mtx.cpu->cycles );
	}
if(vb_csl)	wdg_csl->add( s1 );


wdg_csl->move_cursor( 31, 1 );
			   strpf( s1, "              mem accesses: %020" PRIu64 "", mem_access_count );
if(vb_csl)	wdg_csl->add( s1 );

wdg_csl->restore_cursor();





//need_update = 1;
if(( pause_state == 0 ) | ( need_update ) )
	{
	irq_timer++;
	
	if( irq_timer >= 2 )
		{
		irq_timer = 0;
		}

	need_update = 0;

//printf("pause_state %d, vcpu.size(): %d\n", pause_state, vcpu.size()  );

//	printf("vcpu.size(): %d\n", (int)vcpu.size() );
	if( vcpu.size() > cn_vcpu_cull_limit_high )
		{
//		printf("cn_vcpu_cull_limit_high reached (%d) for vcpu\n", cn_vcpu_cull_limit_high );
		vcpu.erase(  vcpu.begin() + 0, vcpu.begin() + vcpu.size() - cn_vcpu_cull_limit_low );
//		printf("new size - vcpu.size(): %d\n", (int)vcpu.size() );
		}

	cpu_str( vcpu.size() - cn_max_lines, vcpu.size() - 1, s1 );			//make mem accesses vector a readable string

	wdg_csl2->scroll->maximum( vcpu.size() - 1 );


	wdg_csl2->scroll->value( vcpu.size() );


	wdg_csl2->clear();
	if(vb_csl)	wdg_csl2->add( s1 );




//printf("vcpu.size(): %d\n", (int)vcpu.size() );
	
	}
else{
//	if( pause_state >=2 ) ch_wnd->paused = 1;
//	else  ch_wnd->paused = 0;
	if( wdg_csl2->scroll->changed() ) 
		{
		double dd = wdg_csl2->scroll->value();
		wdg_csl2->scroll->value( dd );

		printf("scroll: %f\n", dd );

		int idx1 = dd - cn_max_lines;
		int idx2 = dd;

		if( idx2 < ( cn_max_lines - 2 ) ) idx2 = cn_max_lines - 2 ;

//		if( idx1 < 0 ) idx1 = 0;
//		if( idx2 < 0 ) idx2 = 0;

		if( idx2 >= vcpu.size() ) idx2 = vcpu.size() - 1;
		cpu_str( idx1, idx2, s1 );

		wdg_csl2->clear();
		wdg_csl2->add( s1 );
		}
	}





if( wdg_csl->kbhit() ) 
	{
	got_cmd = 0;

	int keycode = wdg_csl->keycode;
	if( keycode == FL_Enter )
		{
		wdg_csl->add( '\n' );

		got_cmd = 1;
		goto done;
		}

	if( keycode == FL_BackSpace )
		{
		int len = skey.length();
		s1 = skey.substr( 0, len - 1 );
		skey = s1;

		wdg_csl->clear_line();
		wdg_csl->add( skey );
		goto done;
		}

	if( ( keycode >= ' ' ) & ( keycode <= '~' )  )
		{
		skey += keycode;
		wdg_csl->clear_line();
		wdg_csl->add( skey );
		goto done;
		}


done:

	if( got_cmd )
		{
		if( skey.length() >= 3 )
			{

//-------------
			if( ( skey[ 0 ] == 'e' ) & ( skey[ 1 ] == 'x' ) & ( skey[ 2 ] == ' ' ) )			//examine mem
				{
				m1 = skey;
				m1.cut_just_past_first_find_and_keep_right( s1, " ", 0 );
				sscanf( s1.c_str(), "%x", &addr );


//				strpf( s1, " addr: %04x", addr );


				if( ( addr >= 0 ) & ( addr <= 0xffff ) )
					{
					int cx = 16;
					int cy = 16;

					addr &= 0xfff0;
					int count = 0;

					for( int yy = 0; yy < cy; yy ++ )
						{
						st = "";
						strpf( s1, "%04x:  ", addr + count );
						st += s1;
						for( int xx = 0; xx < cx; xx++ )
							{
							strpf( s1, "%02x ", mtx.cpumem[ addr + count ] );
							st += s1;

							if( ( count & 0xf ) == 7 ) st +=  ' ';
							count++;
							}

						st += '\n';
						wdg_csl->add( st );
						if( addr + count >= 0xffff ) break;
						}
					goto ok;
					}
				}
//-------------


//-------------
			if( ( skey[ 0 ] == 'w' ) & ( skey[ 1 ] == 'r' ) & ( skey[ 2 ] == ' ' ) )		//write mem
				{
				m1 = skey;
				m1.cut_just_past_first_find_and_keep_right( s1, " ", 0 );
				sscanf( s1.c_str(), "%x %x", &addr, &data );

				if( ( addr >= 0 ) & ( addr <= 0xffff ) )
					{
					mtx.cpumem[ addr ] = data;
					goto ok;
					}
				}
//-------------


//-------------

			if( ( skey[ 0 ] == 's' ) & ( skey[ 1 ] == 't' ) & ( skey[ 2 ] == ' ' ) )		//step (number of steps)
				{
				m1 = skey;
				m1.cut_just_past_first_find_and_keep_right( s1, " ", 0 );
				sscanf( s1.c_str(), "%d", &inum );
				if( ( inum >= 1 ) & ( inum <= 999999999 ) )
					{
					run_step_size = inum;

					strpf( s1, "stepping: %d (p to contin)\n", run_step_size );
					wdg_csl->add( s1 );
					run_step_size_set = 1;
					pause_state = 0;
					
//					mtx.cpu->Run_breakpoint( run_step_size, bp_addr );
					need_update = 1;
					goto ok;
					}
//				wdg_csl->add( sread );

				}
//-------------


//-------------

			if( ( skey[ 0 ] == 'b' ) & ( skey[ 1 ] == 'p' ) & ( skey[ 2 ] == ' ' ) )		//breakpoint (addr)
				{
				m1 = skey;
				m1.cut_just_past_first_find_and_keep_right( s1, " ", 0 );
				sscanf( s1.c_str(), "%x", &inum );


				bp_addr = inum;
				if( bp_addr < -1  ) bp_addr = -1;
				
				if( ( inum > -1 ) & ( inum <= 0xffff ) )
					{
					bp_set = 1;
					pause_state = 0;
					}
				else{
					bp_set = 0;
					}
				goto ok;
				}
//-------------


//-------------
			if( ( skey[ 0 ] == 'f' ) & ( skey[ 1 ] == 'r' ) & ( skey[ 2 ] == ' ' ) )		//filter selective read mem access for display
				{
				m1 = skey;
				m1.cut_just_past_first_find_and_keep_right( s1, " ", 0 );
				sscanf( s1.c_str(), "%x %x", &addr, &data );

				if( ( addr >= -1 ) & ( addr <= 0xffff ) )
					{
					if( ( data >= -1 ) & ( data <= 0xffff ) )
						{
						read_filter_low = addr;
						read_filter_high = data;
						goto ok;
						}
					}
				}
//-------------




//-------------
			if( ( skey[ 0 ] == 'f' ) & ( skey[ 1 ] == 'w' ) & ( skey[ 2 ] == ' ' ) )		//filter selective write mem access for display
				{
				m1 = skey;
				m1.cut_just_past_first_find_and_keep_right( s1, " ", 0 );
				sscanf( s1.c_str(), "%x %x", &addr, &data );

				if( ( addr >= -1 ) & ( addr <= 0xffff ) )
					{
					if( ( data >= -1 ) & ( data <= 0xffff ) )
						{
						write_filter_low = addr;
						write_filter_high = data;
						goto ok;
						}
					}
				}
//-------------


//-------------
			if( ( skey[ 0 ] == 'd' ) & ( skey[ 1 ] == 'f' ) & ( skey[ 2 ] == ' ' ) )		//save data to file
				{
				m1 = skey;
				m1.cut_just_past_first_find_and_keep_right( s1, " ", 0 );

				strpf( s2, "saving file: '%s'\n", s1.c_str() );
				wdg_csl->add( s2 );
	
				unsigned int ii = 0;

				if( vcpu.size() >= 4000000 )
					{
					ii = vcpu.size() - 1 - 3999999;
					strpf( s2, "found %d lines to write, limiting to last 4000000 lines\n", vcpu.size() );
					wdg_csl->add( s2 );
					}

				cpu_str( ii, vcpu.size() - 1, s2 );				//make mem accesses text strings

				m1 = s2;
				if( m1.writefile( s1 ) )
					{
					wdg_csl->add( "--file write completed--\n" );
					}
				else{
					wdg_csl->add( "--file write failed--\n" );
					}

				goto ok;
				}
//-------------


//-------------
			if( ( skey[ 0 ] == 'd' ) & ( skey[ 1 ] == 's' ) & ( skey[ 2 ] == ' ' ) )		//save step size, bp, filter details
				{
				m1 = skey;
				m1.cut_just_past_first_find_and_keep_right( s1, " ", 0 );

				strpf( s2, "saving file: '%s'\n", s1.c_str() );
				wdg_csl->add( s2 );
	
				strpf( s2, "%d %04x %04x %04x %04x %04x", run_step_size, bp_addr, read_filter_low, read_filter_high, write_filter_low, write_filter_high );

				m1 = s2;
				if( m1.writefile( s1 ) )
					{
					wdg_csl->add( "--file write completed--\n" );
					}
				else{
					wdg_csl->add( "--file write failed--\n" );
					}

				goto ok;
				}
//-------------


//-------------
			if( ( skey[ 0 ] == 'l' ) & ( skey[ 1 ] == 's' ) & ( skey[ 2 ] == ' ' ) )		//load step size, bp, filter details
				{
				m1 = skey;
				m1.cut_just_past_first_find_and_keep_right( s1, " ", 0 );

				load_all_settings( s1 );
/*
				strpf( s2, "loading file: '%s'\n", s1.c_str() );
				wdg_csl->add( s2 );

				if( m1.readfile( s1, 10 ) )
					{
					s2 = m1.str();
					wdg_csl->add( "--file read completed--\n" );
					sscanf( s2.c_str(), "%d %04x %04x %04x %04x %04x", &run_step_size, &bp_addr, &read_filter_low, &read_filter_high, &write_filter_low, &write_filter_high );

					strpf( s1, "step size:         %d\n", run_step_size );
					wdg_csl->add( s1 );
					strpf( s1, "breakpoint:        %04x\n", bp_addr );
					wdg_csl->add( s1 );
					strpf( s1, "read filter low:   %04x\n", read_filter_low );
					wdg_csl->add( s1 );
					strpf( s1, "read filter high:  %04x\n", read_filter_high );
					wdg_csl->add( s1 );
					strpf( s1, "write filter low:  %04x\n", write_filter_low );
					wdg_csl->add( s1 );
					strpf( s1, "write filter high: %04x\n", write_filter_high );
					wdg_csl->add( s1 );
					}
				else{
					wdg_csl->add( "--file read failed--\n" );
					}
*/
				goto ok;
				}
//-------------



			}




		if( skey.length() == 2 )
			{
//-------------
			if( ( skey[ 0 ] == 'c' ) & ( skey[ 1 ] == 'l' ) )		//clear all
				{
				mem_access_count = 0;
				vcpu.clear();
				wdg_csl2->clear();
				wdg_csl3->clear();
				goto ok;
				}
//-------------

//-------------
			if( ( skey[ 0 ] == 'b' ) & ( skey[ 1 ] == 'o' ) )		//boot
				{
					cpu_boot();
//					load_6502(cns_prodigy_rom );
//					mtx.cpu->Reset();
//				thrd_cpu->lock_mutex();
//					vcpu.clear();
					wdg_csl->add( "loaded rom - rebooted\n" );
	//				sread = "";
	//				swrite = "";
	//				srdwr = "";
//				thrd_cpu->unlock_mutex();

				goto ok;
				}
//-------------


//-------------
			if( ( skey[ 0 ] == 'v' ) & ( skey[ 1 ] == 'b' ) )		//verbose
				{
				vb_csl = !vb_csl;
				if( vb_csl ) wdg_csl->add( "verbose on\n" );
				else wdg_csl->add( "verbose off\n" );
//				sread = "";
//				swrite = "";
//				srdwr = "";
				goto ok;
				}
//-------------



//-------------
			if( ( skey[ 0 ] == 's' ) & ( skey[ 1 ] == 't' ) )		//st query
				{
				strpf( s1, "st %d\n", run_step_size );
				wdg_csl->add( s1 );
				goto ok;
				}
//-------------


//-------------
			if( ( skey[ 0 ] == 'b' ) & ( skey[ 1 ] == 'p' ) )		//bp query
				{
				strpf( s1, "bp %04x (ffffffff means disabled, i.e: -1)\n", bp_addr );
				wdg_csl->add( s1 );
				goto ok;
				}
//-------------


//-------------
			if( ( skey[ 0 ] == 'f' ) & ( skey[ 1 ] == 'r' ) )		//fr query
				{
				strpf( s1, "fr %04x %04x (ffffffff means disabled, i.e: -1)\n", read_filter_low, read_filter_high );
				wdg_csl->add( s1 );
				goto ok;
				}
//-------------


//-------------
			if( ( skey[ 0 ] == 'f' ) & ( skey[ 1 ] == 'w' ) )		//fw query
				{
				strpf( s1, "fw %04x %04x (ffffffff means disabled, i.e: -1)\n", write_filter_low, write_filter_high );
				wdg_csl->add( s1 );
				goto ok;
				}
//-------------

//-------------
			if( ( skey[ 0 ] == 's' ) & ( skey[ 1 ] == 's' ) )		//show settings
				{
				strpf( s1, "step size:         %d\n", run_step_size );
				wdg_csl->add( s1 );
				strpf( s1, "breakpoint:        %04x\n", bp_addr );
				wdg_csl->add( s1 );
				strpf( s1, "read filter low:   %04x\n", read_filter_low );
				wdg_csl->add( s1 );
				strpf( s1, "read filter high:  %04x\n", read_filter_high );
				wdg_csl->add( s1 );
				strpf( s1, "write filter low:  %04x\n", write_filter_low );
				wdg_csl->add( s1 );
				strpf( s1, "write filter high: %04x\n", write_filter_high );
				wdg_csl->add( s1 );

				goto ok;
				}
//-------------



			if( ( skey[ 0 ] == 'p' ) & ( skey[ 1 ] == 'c' ) )
				{
				strpf( s1, "pc: %04x\n", mtx.cpu->pc );
				wdg_csl->add( s1 );
				goto ok;
				}


			if( ( skey[ 0 ] == 's' ) & ( skey[ 1 ] == 'p' ) )
				{
				strpf( s1, "sp: %04x\n", mtx.cpu->sp );
				wdg_csl->add( s1 );
				goto ok;
				}

			}


		if( skey.length() == 1 )
			{
//-------------
			if( skey[ 0 ] == 't' )				//trig irq
				{
//				trig_cnt = 1;
				mtx.cpu->IRQ();
				sread = "";
				swrite = "";
				goto ok;
				}
//-------------


//-------------
//			if( skey[ 0 ] == 'b' )				//boot
//				{
//				load_6502();
//				cpu.Reset();
//				goto ok;
//				}
//-------------

//-------------
			if( skey[ 0 ] == 'p' )				//pause
				{
				if( pause_state == 0 ) set_pause_state_to( 2 );
				else set_pause_state_to( 0 );
//				if( pause_state == 0 ) pause_state = 2;
//				else pause_state = 0;

//				if( pause_state != 0 ) strpf( s1, " - paused\n" );
//				else  strpf( s1, " - not paused\n" );
//				wdg_csl->add( s1 );
//				need_update = 1;
				goto ok;
				}
//-------------


//-------------
			if( skey[ 0 ] == 's' )				//step
				{
				go_into_pause_state();
				wdg_csl->add( "single step (p to contin)\n" );
				need_update = 1;
				single_step = 1;
				pause_state = 0;
//				mtx.cpu->Run_breakpoint( run_step_size, bp_addr );
				goto ok;
				}
//-------------


//-------------
			if( skey[ 0 ] == 'h' )				//help
				{
				help();
				goto ok;
				}
//-------------

//-------------
			if( skey[ 0 ] == 'a' )				//acc
				{
				strpf( s1, "reg Acc: %02x\n",  mtx.cpu->A );
				wdg_csl->add( s1 );
				goto ok;
				}
//-------------

//-------------
			if( skey[ 0 ] == 'x' )
				{
				strpf( s1, "reg X: %02x\n",  mtx.cpu->X );
				wdg_csl->add( s1 );
				goto ok;
				}
//-------------

//-------------
			if( skey[ 0 ] == 'y' )
				{
				strpf( s1, "reg Y: %02x\n",  mtx.cpu->Y );
				wdg_csl->add( s1 );
				goto ok;
				}
//-------------

//-------------
			if( skey[ 0 ] == 'f' )
				{
				show_flags2( mtx.cpu->status, s1 );
				strpf( s1, "flags: %s\n", s1.c_str() );
				wdg_csl->add( s1 );
				goto ok;
				}
//-------------

			}

		

bad:

strpf( s1, "%s ? - type h for help\n", skey.c_str() );
wdg_csl->add( s1 );

ok:

//		wdg_csl->restore_cursor();
		skey = "";
//		wdg_csl->clear_line();
		}
	}




/*
if( st_p.p_pipe )
	{
	if( st_p.p_pipe->data_avail )
		{
		m1 = st_p.spipe_in;

		m1.FindReplace( s1, "\r", "", 0 );
		outcsl( s1 );
		
		st_p.p_pipe->data_avail = 0;			//tell thrd_pipe_cb and gcpipe obj we've finished with string
		}
	}
*/

//cull_console_lines_at_begining( 200, 180 );

/*
if( logr )
	{
	vector<string >vs;

	if( logr->get_culled_state( 0 ) )
		{
		outcsl("\n\n ------- Logged was culled -------\n\n");
		}
	
	if ( logr->get_all_vlog_entries( vs, 1 ) != 0 )
		{

		for( int i = 0; i < vs.size(); i++ )
			{
			string s1;
			
			int char_count = tb_csl->length();
			int line_count = tb_csl->count_lines( 0, char_count );
			
			strpf( s1, "%05d: %s", line_count, vs[ i ].c_str() );
			outcsl( s1 );
			}
		}

//	while( 1 )
//		{
//		if ( !logr->get_vlog_entry( 0, s1, 1 ) ) break;	
//		outcsl( s1 );
//		}

	}






if ( sThrd1.iThreadFinished == 1 )
	{
	mode = enStop;
	fi_status->value( "Stopped" );
	led1->ChangeCol( 0 );
	}
else{
	mode = enRun;
	fi_status->value( "Running" );
	led1->ChangeCol( 1 );
	}


if ( mode == enRun )
	{
	
//linux code
#ifndef compile_for_windows
	pthread_mutex_lock( &mutex1 );			//block other thread to gain mutually exclusive access to sglobal
#endif

//windows code
#ifdef compile_for_windows
	WaitForSingleObject( h_mutex1, INFINITE );
#endif

	strpf(s1, "%s\n" , sglobal.c_str() );	//i.e. Thread1() will be blocked if it calls pthread_mutex_lock(..)

//linux code
#ifndef compile_for_windows
	pthread_mutex_unlock( &mutex1 );		//release mutex
#endif


//windows code
#ifdef compile_for_windows
	ReleaseMutex( h_mutex1 );
#endif

	printf( "%s\n", s1.c_str() );
	outcsl( s1);
//	sglobal = ".";
	}
*/

}












//linux code
#ifndef compile_for_windows

//execute shell cmd
int RunShell(string sin)
{

if ( sin.length() == 0 ) return 0;

//make command to cd working dir to app's dir and execute app (params in "", incase of spaces)
//strpf(csCmd,"cd \"%s\";\"%s\" \"%s\"",csPath.c_str(),sEntry[iEntryNum].csStartFunct.c_str(),csFile.c_str());

pid_t child_pid;

child_pid=fork();		//create a child process	

if(child_pid==-1)		//failed to fork?
	{
	printf("\nRunShell() failed to fork\n");
	return 0;
	}

if(child_pid!=0)		//parent fork? i.e. child pid is avail
	{
	int status;
	printf("\nwaitpid: %d, RunShell start\n",child_pid);	

	while(1)
		{
		waitpid(child_pid,&status,0);		//wait for return val from child so a zombie process is not left in system
		printf("\nwaitpid %d RunShell stop\n",child_pid);
		if(WIFEXITED(status)) break;		//confirm status returned shows the child terminated
		}	
	}
else{					//child fork (0) ?
//	printf("\nRunning Shell: %s\n",csCmd.c_str());
	printf("\nRunShell system cmd started: %s\n",sin.c_str());	
	system(sin.c_str());
	printf("\nRunShell system cmd finished \n");	
	exit(1);
	}
return 1;
}

#endif











//windows code
#ifdef compile_for_windows

//execute shell cmd as a process that can be monitored
int RunShell( string sin )
{
BOOL result;
wstring ws1;

if ( sin.length() == 0 ) return 0;


mystr m1 = sin;

m1.mbcstr_wcstr( ws1 );	//convert utf8 string to windows wchar string array


memset(&processInformation, 0, sizeof(processInformation));


STARTUPINFOW StartInfoW; 							// name structure
memset(&StartInfoW, 0, sizeof(StartInfoW));
StartInfoW.cb = sizeof(StartInfoW);

StartInfoW.wShowWindow = SW_HIDE;

result = CreateProcessW( NULL, (WCHAR*)ws1.c_str(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartInfoW, &processInformation);

if ( result == 0)
	{
	
	return 0;
	}

return 1;



//bkup_filelist_SEI[ 0 ].cbSize = sizeof( bkup_filelist_SEI[ 0 ] ); 
//bkup_filelist_SEI[ 0 ].lpVerb = "open"; 
//bkup_filelist_SEI[ 0 ].lpFile = sin.c_str(); 
//bkup_filelist_SEI[ 0 ].lpParameters= 0; 
//bkup_filelist_SEI[ 0 ].nShow = SW_HIDE; 
//bkup_filelist_SEI[ 0 ].fMask = SEE_MASK_NOCLOSEPROCESS; 

//ShellExecuteEx( &bkup_filelist_SEI[ 0 ] );     //execute batch file



//WCHAR cmd[] = L"cmd.exe /c pause";
//LPCWSTR dir = L"c:\\";
//STARTUPINFOW si = { 0 };
//si.cb = sizeof(si);
//PROCESS_INFORMATION pi;

//STARTUPINFO StartInfo; 							// name structure
//PROCESS_INFORMATION ProcInfo; 						// name structure
//memset(&ProcInfo, 0, sizeof(ProcInfo));				// Set up memory block
//memset(&StartInfo, 0 , sizeof(StartInfo)); 			// Set up memory block
//StartInfo.cb = sizeof(StartInfo); 					// Set structure size

//int res = CreateProcess( NULL, (char*)sin.c_str(), 0, 0, TRUE, 0, NULL, NULL, &StartInfo, &ProcInfo );

}

#endif









//open preference specified editor with supplied fname as parameter 
void open_editor( string fname )
{
string s1;

s1 = "\"";
s1 += fav_editor;
s1 += "\"";
s1 += " ";
s1 += "\"";
s1 += fname;
s1 += "\"";


//linux code
#ifndef compile_for_windows
RunShell( s1 );
#endif



//windows code
#ifdef compile_for_windows
wstring ws1;
mystr m1 = s1;

m1.mbcstr_wcstr( ws1 );	//convert utf8 string to windows wchar string array

//WCHAR cmd[] = L"cmd.exe /c pause";
//LPCWSTR dir = L"c:\\";
//STARTUPINFOW si = { 0 };
//si.cb = sizeof(si);
//PROCESS_INFORMATION pi;

STARTUPINFOW StartInfoW; 							// name structure
PROCESS_INFORMATION ProcInfo; 						// name structure
memset(&ProcInfo, 0, sizeof(ProcInfo));				// Set up memory block
memset(&StartInfoW, 0 , sizeof(StartInfoW)); 		// Set up memory block
StartInfoW.cb = sizeof(StartInfoW); 				// Set structure size

int res = CreateProcessW(NULL, (WCHAR*)ws1.c_str(), 0, 0, TRUE, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &StartInfoW, &ProcInfo );

#endif
}










//a gcthrd thread callback
void thrd_cb( void* args )
{
mystr m1;

string nm;
int count = 0;

//printf("callback1\n");

gcthrd *o = (gcthrd*) args;

strpf( nm, "%s -", o->obj_name.c_str() );

if( o->logr ) o->logr->pf( o->pr, "%s thrd started\n", nm.c_str(), count );
if( o->logr ) o->logr->pf( o->pr, "%s cb() - callback\n",  nm.c_str() );



while( 1 )
	{
	if( o->thrd.kill ) goto finthread;


//	timespec ts, tsret;			//don't hog processor in this while()
//	ts.tv_sec = 0;
//	ts.tv_nsec = 1;				//4294967269 
//	nanosleep( &ts, &tsret );
	
	double dt = m1.time_passed( m1.ns_tim_start );
	if( dt > 0.05 )
		{
		m1.time_start( m1.ns_tim_start );
		if( o->thrd_dbg ) printf( "%s cb() -  hello %05d\n", nm.c_str(), count );

		if( o->logr ) o->logr->pf( o->pr, "%s cb() - hello %05d\n", nm.c_str(), count );
		}
	
	m1.delay_ms( 1 );			//don't hog processor in this while()
	count++;
	}

finthread:
o->thrd.finished = 1;
o->thrd.kill = 0;

if( o->thrd_dbg ) printf( "%s thrd finished\n", nm.c_str() );

if( o->logr ) o->logr->pf( o->pr, "%s thrd finished\n", nm.c_str() );
}




bool set_pause_state_to( int state )
{
if( state < 0 ) state = 0;
if( state > 2 ) state = 2;

pause_state = state;

string s1;
if( pause_state != 0 ) strpf( s1, " - paused\n" );
else  strpf( s1, " - not paused\n" );
wdg_csl->add( s1 );
need_update = 1;
}




//flag for pause, wait for 'thrd_cpu_cb()' to confirm
bool go_into_pause_state()
{
mystr m1;
pause_state = 1;

for( int i = 0; i < 250; i++ )
	{
	if( pause_state == 2 ) return 1;
	m1.delay_ms(1);
	}

printf( "go_into_pause_state() - timeout\n" );
return 0;
}





uint64_t thrd_dbg_cnt = 0;


int time_for_irq = 0;



//a gcthrd thread callback
void thrd_cpu_cb( void* args )
{
string s1;
mystr m1;

string nm;
int count = 0;

//printf("callback1\n");

gcthrd *o = (gcthrd*) args;

strpf( nm, "%s -", o->obj_name.c_str() );

if( o->logr ) o->logr->pf( o->pr, "%s thrd started\n", nm.c_str(), count );
if( o->logr ) o->logr->pf( o->pr, "%s cb() - callback\n",  nm.c_str() );


int cpu_steps = 1000;

while( 1 )
	{
	if( o->thrd.kill ) goto finthread;

	thrd_dbg_cnt++;
//	if( !(thrd_dbg_cnt%500) )
//		{
//		printf("thrd_cpu_cb 1: %" PRIu64 ", %d, %d %d %d\n", thrd_dbg_cnt, pause_state, single_step, cpu_steps, bp_set );
//		}



	if( pause_state > 0 )
		{
		pause_state = 2;

//linux code
#ifndef compile_for_windows
		pthread_yield();
#endif

//windows code
#ifdef compile_for_windows
		Sleep( 0 );
#endif

		m1.delay_ms( 50 );			//don't hog processor in this while()
		continue;
		}


//	timespec ts, tsret;			//don't hog processor in this while()
//	ts.tv_sec = 0;
//	ts.tv_nsec = 1;				//4294967269 
//	nanosleep( &ts, &tsret );
	
	
	double dt = m1.time_passed( m1.ns_tim_start );
	if( dt > 0.0001 )
		{
//		if( !(thrd_dbg_cnt%500) )
//			{
//			printf("thrd_cpu_cb 0: %" PRIu64 "\n", thrd_dbg_cnt );
//			}

		if( single_step )
			{
			strpf( s1, "single step (p to contin): %04x\n", mtx.cpu->pc );
			printf( "%s", s1.c_str() );
			cpu_run( 1 );
			single_step = 0;
			pause_state = 2;
			goto chk_irq;
			}
		else{
//			if( !(thrd_dbg_cnt%500) )
//				{
//				printf("thrd_cpu_cb 2: %" PRIu64 "\n", thrd_dbg_cnt );
//				}

			if( bp_set )
				{
				if( bp_addr != -1 )
					{
					if ( mtx.cpu->Run_breakpoint( cpu_steps, bp_addr ) )
						{
						strpf( s1, "hit breakpoint (p to contin): %04x\n", bp_addr );
						printf( "%s", s1.c_str() );
						thrd_cpu->lock_mutex();
							mtx.vcls3.push_back( s1 );
						thrd_cpu->unlock_mutex();
						
						bp_set = 0;
						pause_state = 2;
						need_update = 1;
						continue;
						}
					}
				else{

//					if( !(thrd_dbg_cnt%500) )
//						{
//						printf("thrd_cpu_cb 3: %" PRIu64 "\n", thrd_dbg_cnt );
//						}
					}
				}
			else{
				if( run_step_size_set )
					{
					run_step_size_set = 0;
					cpu_run( run_step_size );				//mutex locks, will change memory
					pause_state = 2;
					need_update = 1;

//					if( !(thrd_dbg_cnt%500) )
//						{
//						printf("thrd_cpu_cb 4: %" PRIu64 "\n", thrd_dbg_cnt );
//						}
					continue;
					}
				else{
					cpu_run( cpu_steps );				//mutex locks, will change memory
//					if( !(thrd_dbg_cnt%500) )
//						{
//						printf("thrd_cpu_cb 5: %" PRIu64 "\n", thrd_dbg_cnt );
//						}
					}
				}
			}
chk_irq:
		time_for_irq++;
		if( time_for_irq > 1 )
			{
			mtx.cpu->IRQ();
			time_for_irq = 0;
//				if( !(thrd_dbg_cnt%500) )
//					{
//					printf("thrd_cpu_cb 6: %" PRIu64 "\n", thrd_dbg_cnt );
//					}
			}

		m1.time_start( m1.ns_tim_start );
//		if( o->thrd_dbg ) printf( "%s cb() -  hello %05d\n", nm.c_str(), count );

//		if( o->logr ) o->logr->pf( o->pr, "%s cb() - hello %05d\n", nm.c_str(), count );
		}

//	timespec ts, tsret;			//don't hog processor in this while()
//	ts.tv_sec = 0;
//	ts.tv_nsec = 100000;				//4294967269 
//	nanosleep( &ts, &tsret );

	if( ch_wnd ) m1.delay_us( ch_wnd->usec );
	else m1.delay_us( 1000 );

//	m1.delay_ms( 1 );			//don't hog processor in this while()
	count++;
	}

finthread:
o->thrd.finished = 1;
o->thrd.kill = 0;

if( o->thrd_dbg ) printf( "%s thrd finished\n", nm.c_str() );

if( o->logr ) o->logr->pf( o->pr, "%s thrd finished\n", nm.c_str() );
}














void thrd_cpu_start()
{

if( thrd_cpu == 0 ) 
	{
	thrd_cpu = new gcthrd();
//	thrd_cpu->set_log_ptr( 5, logr );			//set thrd to use a prev created log obj
	thrd_cpu->thrd_dbg = 1;
	thrd_cpu->set_name( "thrd_cpu" );
	thrd_cpu->set_thrd_callback( thrd_cpu_cb, (void*)thrd_cpu );		//set thread callback

	thrd_cpu->create_thread();
	thrd_cpu->create_mutex();
	}

}





void thrd_cpu_stop()
{

if( thrd_cpu != 0 )
	{
	thrd_cpu->unlock_mutex();
	thrd_cpu->destroy_mutex();
	thrd_cpu->destroy_thread( );
	if( thrd_cpu->wait_till_thread_destroyed( 1000 ) )
		{
		delete thrd_cpu;
		thrd_cpu = 0;
		}
	}
}






















































void cb_ic_dummy( Fl_Widget *w, void * )
{
My_Input_Choice *o = (My_Input_Choice*) w;

string s1;

s1 = o->value();
printf("My_Input_Choice - cb_ic_dummy() - '%s'\n", s1.c_str() );
}





int pressed_outs, pressed_ins;

void cb_bt_sw( Fl_Widget *w, void *v )
{
string s1;

Fl_Button *o = (Fl_Button*) w;

int butcode = (int) v;



pressed_outs = butcode >> 8;							//extract xy from code
pressed_ins = butcode & 0xff;

//printf( "pushed: %d, str:'%s'\n", ii, labl.c_str() );


//sscanf( labl.c_str(),"%d.%d", &pressed_outs, &pressed_ins ); 

printf( "pushed: outs: %d, ins: %d, state: %d\n", pressed_outs, pressed_ins, o->value() );

sw_matrix_butt[ pressed_outs ][ pressed_ins ] = o->value();
}









//not a thread safe callback by obj gclog, no mutex is used, dont use in multithread app
void log_callback( string s )
{

//vlog.push_back( s );

//outcsl( s );
}





















void beep( int pitch, int count, int length )
{
string s1;

//beep -f xxx -d xxx -r x

strpf( s1, " %s -f %d -d %d -r %d\n", cns_beep_cmd, pitch, length, count );
printf( "%s",  s1.c_str() );

if( pitch == cn_beep_freq1 )
	{
	strpf( s1, "%s",  cns_script_beep1 );
	RunShell( s1 );
	}

if( pitch == cn_beep_freq2 )
	{
	strpf( s1, "%s",  cns_script_beep2 );
	RunShell( s1 );
	}
}





void cpu_boot()
{
if( thrd_cpu ) thrd_cpu->lock_mutex();


vcpu.clear();
mtx.vcls.clear();
mtx.vcls2.clear();
mtx.vcls3.clear();

wdg_csl2->clear();
wdg_csl3->clear();

	for( int oo = 0; oo < cn_outs; oo++ )
		{
		for( int ii = 0; ii < cn_ins; ii++ )
			{
			sw_matrix_butt[ oo ][ ii ] = 0;					//assume all switches open
			sw_matrix_brd[ oo ][ ii ] = 0;
			sw_matrix_cpanel[ oo ][ ii ] = 0;
			}
		}

//	mtx.cpu->Reset();
//	mtx.cpu->Run( 10 );

	if( ch_wnd ) ch_wnd->new_game();

	bool pause_grab = pause_state;

	go_into_pause_state();


	mem_access_count = 0;
	mtx.cpu->Reset();

	if( !load_6502( cns_prodigy_rom ) ) exit(0);

	mtx.cnt_busrd = 0;
	mtx.cnt_buswr = 0;
	mtx.via_wr_0x2000 = 0;
	mtx.via_wr_0x2005 = 0;
	mtx.via_wr_0x200a = 0;
	mtx.via_wr_0x200b = 0;
	mtx.cnt_via_rd_0x2000 = 0;
	mtx.cnt_via_rd_0x2001 = 0;
	mtx.dat2000 = 0xff;											//data for VIA port PB
	mtx.dat2001 = 0xff;											//data for VIA port PA

	mtx.via_PB7 = 0;
	mtx.illegal_opcode_pc = -1;

	//	wdg_csl->add( "\n\nloaded rom, enter 'p' to un-pause, 'h' for help.\n\n" );

if( thrd_cpu ) thrd_cpu->unlock_mutex();

pause_state = pause_grab;

need_update = 1;
}











int cpu_run( unsigned int steps )
{
if( thrd_cpu == 0 ) return 0;

if( steps > 1000000 ) steps = 1000000;

thrd_cpu->lock_mutex();
	mtx.cpu->Run( steps );
thrd_cpu->unlock_mutex();

return 1;
}









int main(int argc, char **argv)
{
//getchar();


Fl::scheme("plastic");								//optional
string s, fname, dir_sep;
bool add_ini = 1;									//assume need to add ini extension	

//Fl::set_font( FL_HELVETICA, "Helvetica bold");
//Fl::set_font( FL_COURIER, "");
//Fl::set_font( FL_COURIER, "Courier bold italic");

//fl_font( (Fl_Font) FL_COURIER, 12 );

fname = cnsAppName;							//assume ini file will have same name as app
dir_sep = "";								//assume no path specified, so no / or \ (dos\windows)



//test if window with same name found and ask if still to run this -
// will return 1 if user presses 'Don't Run, Exit' button
if( CheckInstanceExists( cnsAppWndName ) ) return 0;

//linux code
#ifndef compile_for_windows
dir_seperator = "/";									//use unix folder directory seperator
#endif


dir_sep = dir_seperator;



//windows code
//attach a command line console, so printf works
#ifdef compile_for_windows
int hCrt;
FILE *hf;

AllocConsole();
hCrt = _open_osfhandle( (long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
hf = _fdopen( hCrt, "w" );
*stdout = *hf;
setvbuf( stdout, NULL, _IONBF, 0 );
#endif



string app_name;
vector<string> vparams;

get_path_app_params( dir_seperator, argc, argv, app_path, app_name, vparams );








//path = app_path;


//windows code
#ifdef compile_for_windows
h_mutex1 = CreateMutex( NULL, FALSE, NULL );  // make a win mutex obj, not signalled ( not locked)
#endif




//handle command line params
printf("\n\nProdigy App\n");
    printf("~~~~~~~~~~~~");


printf("\nSpecify no switches to use config with app's filename in app's dir\n");
printf("\nSpecify '--cf ffff' to use config with filename 'ffff'\n");


if( argc == 1 )
	{
	printf( "-- Using app's dir for config storage.\n" );
	}


if( argc == 3 )
	{
	if( strcmp( argv[ 1 ], "--cf" ) == 0 )
		{
		printf( "-- Using specified filename for config storage.\n" );
		fname = argv[ 2 ];
		app_path = "";
		dir_sep = "";								//no directory seperator
		add_ini = 0;								//user spec fname don't add ini ext
		}
	}





if( app_path.length() == 0 )
	{
	csIniFilename = app_path + fname;					//make config file pathfilename
	}
else{
	csIniFilename = app_path + dir_sep + fname;			//make config file pathfilename
	}
if( add_ini ) csIniFilename += ".ini";                  //need ini if no user spcified fname





printf("\n\n-> Config pathfilename is:'%s', this will be used for config recall and saving.\n",csIniFilename.c_str());


logr = new gclog( );								//remember to delete at end of app
//logr->set_log_callback( log_callback );				//called when log.pf(..) is called
logr->set_fname( cnsLogName );
if( !logr->set_max_log_size( cnsLogSwapName, 10000, 5000 ) )	//set a limit to log filesize
	{
	printf("\nFailed to create log swap file\n");
	}


//for testing only!!!!!!
//logr->destructive_stress_test_of_log_file( cnsLogName, cnsLogSwapName);	//for testing only!!!!!!
//return 1;																//for testing only!!!!!!


logr->create_mutexes( );	//ensure via a mutex that only one thread can access log output using log.pf(..)  


wndMain = new dble_wnd( 50, 50, 1195, 705 );
wndMain->label( cnsAppWndName );



//calc offset window border, for removal when saving settings
iBorderWidth=wndMain->x();
iBorderHeight=wndMain->y();
wndMain->border(1);
iBorderWidth=wndMain->x()-iBorderWidth;
iBorderHeight=wndMain->y()-iBorderHeight;

//menu bar
meMain = new Fl_Menu_Bar(0, 0, wndMain->w(), 25);
meMain->textsize(12);
meMain->copy(menuitems, wndMain);


//Fl_Box *bx1 = new Fl_Box( 0, meMain->h(), 700, 150, "");
//bx1->box( FL_BORDER_BOX );



wdg_csl = new csl_wdg( 5, meMain->h()+155 -150, 590, 450, "Csl");
wdg_csl2 = new csl_wdg( 600, meMain->h()+155-150, 580, 450, "Csl2");
wdg_csl3 = new csl_wdg( 5, meMain->h()+615-155, 590, 220, "Csl3");


wdg_csl->fontsize = 11;
wdg_csl2->fontsize = 11;
wdg_csl3->fontsize = 11;

wdg_csl2->add_hscrollbar();

wdg_csl->show_have_focus = 1;
wdg_csl2->show_have_focus = 0;
wdg_csl3->show_have_focus = 0;

wdg_seg0 = new seg7_wdg( 600, meMain->h()+460, 26, 35, "seg0");
wdg_seg1 = new seg7_wdg( 626, meMain->h()+460, 26, 35, "seg1");
wdg_seg2 = new seg7_wdg( 652, meMain->h()+460, 26, 35, "seg2");
wdg_seg3 = new seg7_wdg( 678, meMain->h()+460, 26, 35, "seg3");

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


wdg_csl->set_char_dimensions( 80, cn_max_lines );
wdg_csl2->set_char_dimensions( 80, cn_max_lines );
wdg_csl3->set_char_dimensions( 80, 15 );

wdg_csl->move_cursor( 0, 10 );

Fl_Button *bt_sw[ cn_outs ][ cn_ins ];
string s_bt_tooltip[ cn_outs ][ cn_ins ];

int offx = 30;
int offy = 20;

int yy = 0;
for( int ii = 0; ii < cn_ins; ii++ )
	{
	int xx = 0;
	for( int oo = 0; oo < cn_outs; oo++ )
		{
		string s1, s2;

		int out, in;

		out = cn_outs - 1 - oo;
		in = cn_ins - 1 - ii;

		xx += offx;
		strpf( s1, "%d.%d", out, in );
		s_bt_tooltip[ out ][ in ] = s1;


		Fl_Button *zz = new Fl_Button( 680 + xx, 485 + yy, offx, offy, "" );
		zz->copy_label( s1.c_str() );
		zz->labelsize( 9 );
		zz->tooltip( s_bt_tooltip[ out ][ in ].c_str() );

		int butcode = ( out << 8 ) + in;		//make a xy code with respresents x in 0xff00 mask, andy=0x00ff

		zz->callback( cb_bt_sw, (void*)butcode );
		zz->when( FL_WHEN_CHANGED );

		//set worked out function labels
		if( ( ( out ) == 9 ) & ( ( in ) == 9 ) ) zz->label( "B2" );
		if( ( ( out ) == 9 ) & ( ( in ) == 8 ) ) zz->label( "Black" );
		if( ( ( out ) == 9 ) & ( ( in ) == 7 ) ) zz->label( "White" );
		if( ( ( out ) == 9 ) & ( ( in ) == 6 ) ) zz->label( "C3" );
		if( ( ( out ) == 9 ) & ( ( in ) == 5 ) ) zz->label( "F6" );
		if( ( ( out ) == 9 ) & ( ( in ) == 4 ) ) zz->label( "Time" );
		if( ( ( out ) == 9 ) & ( ( in ) == 3 ) ) zz->label( "Audio" );
		if( ( ( out ) == 9 ) & ( ( in ) == 2 ) ) zz->label( "Hint" );
		if( ( ( out ) == 9 ) & ( ( in ) == 1 ) ) zz->label( "H8" );
		if( ( ( out ) == 9 ) & ( ( in ) == 0 ) ) zz->label( "E5" );
		if( ( ( out ) == 8 ) & ( ( in ) == 9 ) ) zz->label( "A1" );
		if( ( ( out ) == 8 ) & ( ( in ) == 8 ) ) zz->label( "Go" );
		if( ( ( out ) == 8 ) & ( ( in ) == 7 ) ) zz->label( "Enter" );
		if( ( ( out ) == 8 ) & ( ( in ) == 6 ) ) zz->label( "Verfy" );
		if( ( ( out ) == 8 ) & ( ( in ) == 5 ) ) zz->label( "ChBrd" );
		if( ( ( out ) == 8 ) & ( ( in ) == 4 ) ) zz->label( "Level" );
		if( ( ( out ) == 8 ) & ( ( in ) == 3 ) ) zz->label( "CE" );
		if( ( ( out ) == 8 ) & ( ( in ) == 2 ) ) zz->label( "Restr" );
		if( ( ( out ) == 8 ) & ( ( in ) == 1 ) ) zz->label( "G7" );
		if( ( ( out ) == 8 ) & ( ( in ) == 0 ) ) zz->label( "D4" );

		//show chess sqr coords
		if( ( out >= 0 ) & ( out <= 7 ) )
			{
			strpf( s1, "%c", out + 1 + 0x60 );
			if( ( in >= 0 ) & ( in <= 7 ) )
				{
				strpf( s2, "%s%d", s1.c_str(), in + 1 );
				zz->copy_label( s2.c_str() );
				}
			}

		bt_sw[ oo ][ ii ] = zz;
		}

	yy += offy;
	}




wndMain->end();
wndMain->callback((Fl_Callback *)cb_wndmain, wndMain);

make_pref_wnd();
make_pref2_wnd();
make_font_pref_wnd();


ch_wnd = new chess_wnd( 50, 50, 980, 670, "Chess Window" );
ch_wnd->menu = new Fl_Menu_Bar( 0, 0, ch_wnd->w(), 25 );
ch_wnd->menu->textsize(12);
ch_wnd->menu->copy( menuitems, ch_wnd );
ch_wnd->end();
ch_wnd->callback((Fl_Callback *)cb_wndmain, wndMain);


//ch_wnd->resizable( ch_wnd );	
//ch_wnd->callback( (Fl_Callback *)cb_chess_wnd, ch_wnd );


//wndMain->size_range( 700, 450 );
//wndMain->resizable( wndMain );	//note this must be before LoadSettings(), where the window is resized

LoadSettings(csIniFilename); 

update_fonts(); //needed this after LoadSettings() so the ini font value is loaded via font_pref_wnd->Load(p);


wndMain->show(argc, argv);
ch_wnd->show();



pause_state = 2;
thrd_cpu_start();

if (!load_6502( cns_prodigy_rom ) ) exit(0);
//mtx.cpu = new mos6502( cpu_busrd, cpu_buswr );				// !!! MAKE SURE you call 'init_opcodes()', straight after calling this constructor
mtx.cpu = new mos6502( busrd, buswr );						// !!! MAKE SURE you call 'init_opcodes()', straight after calling this constructor
mtx.cpu->init_opcodes();

load_all_settings( "zall-settings.txt" );
wdg_csl->add( "\n\nloaded rom, enter 'p' to un-pause, 'h' for help.\n\n" );

cpu_boot();

pause_state = 0;


st_p.p_thrd = 0;



mode = enStop;
sThrd1.iKillThread=0;
sThrd1.iThreadFinished=1;


cslpf( "\ncslpf() - Testing %d\n", 123 );

Fl::add_timeout( 0.5, cb_timer1 );		//update controls, post queued messages

//cb_bt_convert_str_to_mcb_hex( 0, 0 );

fl_message_font( (Fl_Font) font_num, font_size );

 
int iAppRet=Fl::run();

delete logr;

return iAppRet;

}














//start a timing timer with internal ns resolution
//the supplied var 'start' will hold an abitrary count in ns used by call time_passed()
void time_start( unsigned long long int &start )
{

//linux code
#ifndef compile_for_windows 

timespec ts_now;

clock_gettime( CLOCK_MONOTONIC, &ts_now );		//initial time read
start = ts_now.tv_nsec;							//get ns count
start += (double)ts_now.tv_sec * 1e9;					//make secs count a ns figure

#endif




//windows code
#ifdef compile_for_windows

LARGE_INTEGER li;
unsigned long long int big;

QueryPerformanceFrequency( &li );		//get performance counter's freq


big = 0;
big |= li.HighPart;
big = big << 32;
big |= li.LowPart;

perf_period = (double)1.0 / (double)big;		//derive performance counter's period

//cslpf("Freq = %I64u\n", big );
//cslpf("Period = %g\n", perf_period );

QueryPerformanceCounter( &li );

start = 0;
start |= li.HighPart;
start = start << 32;
start |= li.LowPart;

start = start * perf_period * 1e9;					//make performance counter a ns figure
#endif

}








//calc secs that have passed since call to time_start()
//calcs are maintained in ns internally and in supplied var 'start'
//returns time that has passed in seconds
double time_passed( unsigned long long int start )
{
unsigned long long int now, time_tot;

//linux code
#ifndef compile_for_windows 

timespec ts_now;

clock_gettime( CLOCK_MONOTONIC, &ts_now );		//read current time
now = ts_now.tv_nsec;							//get ns count
now += (double)ts_now.tv_sec * 1e9;				//make secs count a ns figure

time_tot = now - start;

#endif





//windows code
#ifdef compile_for_windows

LARGE_INTEGER li;

QueryPerformanceCounter( &li );

now = 0;
now |= li.HighPart;
now = now << 32;
now |= li.LowPart;

now = now * perf_period * 1e9;			//conv performance counter figure into ns

time_tot = now - start;
#endif


return (double)time_tot * 1e-9;			//conv internal ns delta to secs
}



















///linux code
#ifndef compile_for_windows

//---------------------- thread -------------------------
void* Thread1( void* arg )
{
string *s1 = ( string * ) arg;
timespec ts_old, ts_new;
bool need_mutex = use_mutex;


sThrd1.iThreadFinished = 0;

printf( "\nThread1 started, stopping after 10 secs ...\n" );


clock_gettime( CLOCK_MONOTONIC, &ts_old );				//initial time read

#define maxcount 25
for ( int z = 0; z < 100 ; z++ )
	{
	if( sThrd1.iKillThread == 1 ) goto finthread1;

	clock_gettime( CLOCK_MONOTONIC, &ts_new );			//get new time

	if ( ts_new.tv_sec >= ts_old.tv_sec + 7 ) break;	//finish thread after x secs
	
	timespec ts, tsret;				//don't hog processor in this for() loop
	ts.tv_sec = 0;					//this can hold seconds
//	ts.tv_nsec = 0x3b9ac9ff;		//0x3b9ac9ff = 999999999 nS or 0.999999999 secs
	ts.tv_nsec = 500000000;			//500000000 nS or 0.5 secs
//	nanosleep( &ts , &tsret );

	int endptr;
	for ( int i = 0; i < 1000000; i ++ )
		{
    	if( sThrd1.iKillThread == 1 ) goto finthread1;

		if (need_mutex )
			{
			//block other thread to gain mutually exclusive access to sglobal
			//i.e. cb_timer will be blocked if it calls pthread_mutex_lock(..)

			pthread_mutex_lock( &mutex1 );			//block other thread to gain mutually exclusive access to sglobal
			}

		for ( int k = 0; k < maxcount; k ++ )
			{
			sglobal += "*";
			}

		endptr = sglobal.length() - 1;
		for ( int j = 0; j < maxcount; j++ )
			{
			sglobal.erase( endptr );
			endptr--;
			}

		if (need_mutex ) 
			{
			pthread_mutex_unlock( &mutex1 );		//release mutex
			}
		}


//	printf( "Thread1 loop %02d: Passed arg is a string pointer: %s\n" , gi , s1->c_str() );
//	break;
	}

finthread1:

printf( "\nThread1 finished...\n" );

sThrd1.iThreadFinished = 1;
return  (void*) 1;	
}
//-------------------------------------------------------
#endif












//windows code
#ifdef compile_for_windows
//only use mutex locked logpf_thrd() calls
//---------------------- thread -------------------------
DWORD WINAPI DosThread(void* lpData)
{
bool need_mutex = use_mutex;
string s1;
double duration;

printf( "\nThread1 started, stopping after 10 secs ...\n" );

sThrd1.iThreadFinished = 0;

time_start( ns_tim_start1 );

#define maxcount 25
for ( int z = 0; z < 100 ; z++ )
	{
	if( sThrd1.iKillThread == 1 ) goto finthread1;

    Sleep( 500 );

	duration = time_passed( ns_tim_start1 );
	if ( duration >= 7 ) break;
	


	int endptr;
	for ( int i = 0; i < 1000000; i ++ )
		{
    	if( sThrd1.iKillThread == 1 ) goto finthread1;

		if (need_mutex )
			{
			//block other thread to gain mutually exclusive access to sglobal
			//i.e. cb_timer will be blocked if it calls pthread_mutex_lock(..)

			WaitForSingleObject( h_mutex1, INFINITE );		//block other thread to gain mutually exclusive access to sglobal
			}

		for ( int k = 0; k < maxcount; k ++ )
			{
			sglobal += "*";
			}

		endptr = sglobal.length() - 1;
		for ( int j = 0; j < maxcount; j++ )
			{
			sglobal.erase( endptr );
			endptr--;
			}

		if (need_mutex ) 
			{
			ReleaseMutex( h_mutex1 );						//release mutex
			}
		}


//	printf( "Thread1 loop %02d: Passed arg is a string pointer: %s\n" , gi , s1->c_str() );
//	break;
	}



finthread1:

sThrd1.iThreadFinished = 1;
printf( "\nThread1 finished...\n" );

return 1;

}
#endif
//-------------------------------------------------------

