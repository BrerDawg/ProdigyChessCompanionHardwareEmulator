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

//prodigy_main.h
//v1.03	

#ifndef prodigy_main_h
#define prodigy_main_h


#define _FILE_OFFSET_BITS 64			//large file handling, must be before all #include...
//#define _LARGE_FILES

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <locale.h>
#include <string>
#include <vector>
#include <wchar.h>
#include <algorithm>
#include <stdint.h>

#define __STDC_FORMAT_MACROS			//needed this with windows/msys/mingw for: PRIu64
#include <inttypes.h>

//#include <ncurses.h>

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

#include "globals.h"
#include "pref.h"
#include "GCProfile.h"
#include "GCLed.h"
#include "gclog.h"
#include "gcthrd.h"
#include "gcpipe.h"
#include "mos6502.h"
#include "chss.h"
#include "seg7.h"

//linux code
#ifndef compile_for_windows

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
//#include <X11/Xaw/Form.h>
//#include <X11/Xaw/Command.h>

#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <syslog.h>		//MakeIniPathFilename(..) needs this
#include <termios.h>					//for kbhit() like function

#endif


//windows code
#ifdef compile_for_windows
#include <windows.h>
#include <process.h>
#include <winnls.h>
#include <share.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <conio.h>

#define WC_ERR_INVALID_CHARS 0x0080		//missing from gcc's winnls.h
#endif


using namespace std;

#define cnsAppName "prodigy"
#define cnsAppWndName "Prodigy Chess Emulator"

#define cnFontEditor 4
#define cnSizeEditor 12

#define cnGap 2
#define cnCslHei 30

#define cnsLogName "log.txt"						//log filename
#define cnsLogSwapName "log_swap.txt"	//log swap file for culling older entries at begining of log file

//linux code
#ifndef compile_for_windows
#define LLU "llu"
#endif


//windows code
#ifdef compile_for_windows
#define LLU "I64u"
#endif

//#define cns_prodigy_rom "prodigy_0x6000.bin"
#define cns_prodigy_rom "0x2000.bin"

#define cn_max_lines 31
#define cn_cpu_mem_size 65536
#define cn_vcpu_cull_limit_high (1024*1024 * 16) 
#define cn_vcpu_cull_limit_low (1024*1024 * 4) 


colref col_bkgd =   { 64,    64,    64 	};
colref col_yel = 	{ 255,   255,   0 	};
colref col_red =	{ 255,   0,     0	};
colref col_mag =	{ 255,   0,     255	};
colref col_wht =	{ 255,   255,   255	};






//struct to hold pointers for 'thrd_pipe_cb' and 'timer1' to utilise
struct pipethrd_tag
{
gcpipe *p_pipe;
gcthrd *p_thrd;
string spipe_in;						//used by thread_pipe and timer1 maybe with mutex protection
};

pipethrd_tag st_p;






// v1.02    14-nov-2014
class My_Input_Choice : public Fl_Input_Choice 
{
private:
bool ctrl_key;
bool shift_key;
double d_std_step;
double d_ctrl_step;
double d_shift_step;
double d_ctrl_shift_step;


public:
int var_type;                                      //0=string, 1=integer, 2=double
bool b_allow_wheel_inc;
bool b_allow_wheel_cb;
bool b_allow_enter_key_cb;
bool b_allow_enter_key_store_to_dropdown;           //enter key will store currentvalue into dropdown list
bool b_auto_sort;                                   //always sort after changes

vector<string> vdropdown;                           //holds drop down entries
//int ivar;
//double dvar;

private:
int handle(int);

public:
My_Input_Choice(int x,int y,int w, int h,const char *label=0);

void load_settings( string sprofile_fname, string ssection, string skey, string sdef, char delimit, bool strip_crlf );
void save_settings( string sprofile_fname, string ssection, string skey, char delimit, bool strip_crlf );
void add_to_dropdown( string ss, bool strip_crlf );
void add_to_dropdown( int ii );
void add_to_dropdown( double dd );
void alpha_or_numerical_sort();
void reload_hist();
void get_value( int *ii );
void get_value( double *dd );
void set_value( int ii );
void set_value( double dd );

void std_step( double dd );
void ctrl_key_step( double dd );
void shift_key_step( double dd );
void ctrl_shift_key_step( double dd );

};







class ucde_file_chooser : public Fl_File_Chooser
{
private:										//private var


public:											//public var
ucde_file_chooser( const char *d, const char *p, int t, const char *title );
void draw();

};







//use this class as it trys to hold un-maximised window size values by detecting a
//large window resize that is close to sreen resolution, see dble_wnd::resise()
class dble_wnd : public Fl_Double_Window
{
private:										//private var


public:											//public var
bool dble_wnd_verbose;
int maximize_boundary_x, maximize_boundary_y, maximize_boundary_w, maximize_boundary_h;     //see dble_wnd::resize()
int restore_size_x, restore_size_y, restore_size_w, restore_size_h;                         //see dble_wnd::resize()

public:											//public functions
dble_wnd( int xx, int yy, int wid, int hei, const char *label = 0 );
~dble_wnd();

private:										//private functions
void resize( int xx, int yy, int ww, int hh );

};








class ucde_wnd : public dble_wnd
{
private:										//private var

public:
Fl_Input *fi_unicode_greek;
Fl_Input *fi_unicode;
Fl_Input *fi_unicode_mcb;
Fl_Input *fi_wchar_str;
Fl_Input *fi_wchar_hex;
Fl_Text_Buffer *tb_csl;
Fl_Text_Editor *te_csl;
Fl_File_Input *fi_filename;
ucde_file_chooser *fc;
Fl_Input* fi_create_filename;


public:											//public var
ucde_wnd( int xx, int yy, int wid, int hei, const char *label );
~ucde_wnd();
void setfont( Fl_Font fnt );
void setsize( int size );
void outcsl( string s );


};





class mywnd : public dble_wnd
{
private:										//private var
int *buf;
int ctrl_key;
int left_button;
string dropped_str;

int mousewheel;
Fl_Box *bx_image;
Fl_JPEG_Image *jpg;


public:											//public var
int a_public_var;

public:											//public functions
mywnd( int xx, int yy, int wid, int hei, const char *label );
~mywnd();
void init();

private:										//private functions
void draw();
int handle( int );
void setcolref( colref col );

};


#endif


