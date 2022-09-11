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

#ifndef globals_h
#define globals_h


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


#include "mos6502.h"


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




#define cns_build_date build_date       //this holds the build date e.g: 2016 Mar 23, obtained automatically by a shell cmd and sets '-Dbuild_date' option in Makefile, you could manually use e.g: -Dbuild_date="\"2016 Mar 23\""

#define cn_outs 10					//switch matrix: sw_matrix[outs][ins]
#define cn_ins 11

#define cn_cpu_mem_size 65536
#define cn_cpu_mem_2KB 2048


#define cns_help_filename "help.txt"			// !! don't use white spaces

//linux code
#ifndef compile_for_windows
#define cns_open_editor "open_editor.sh"		// !! don't use white spaces
#define cns_script_beep1 "./beep1.sh"
#define cns_script_beep2 "./beep2.sh"
#endif

//windows code
#ifdef compile_for_windows
#define cns_open_editor "open_editor.bat"		// !! don't use white spaces
#define cns_script_beep1 "beep1.bat"
#define cns_script_beep2 "beep2.bat"
#endif


#define cns_beep_cmd "beep"
#define cn_beep_freq1 5000
#define cn_beep_freq2 7000
#define cn_beep_delay 200
#define cn_beep_repetition 2



struct colref
{
int r, g, b;
};




struct st_cpu_operation_tag
{
uint64_t mem_access_count;
unsigned int pc;
int rd_wr;								//0 = a read operation, 1 = a write op
uint16_t addr;
uint8_t data;
uint8_t A, X, Y, flags;
};




class mos6502;




struct mtx_tag
{
bool rom_loaded;

uint8_t cpumem[ cn_cpu_mem_size ];						//cpu memory
mos6502 *cpu;
unsigned int cnt_busrd;
unsigned int cnt_buswr;

int illegal_opcode_pc;

bool via_2000_changed;

int via_wr_0x2000;
int via_wr_0x2005;
int via_wr_0x200a;
int via_wr_0x200b;

int via_rd_0x2000;
int via_rd_0x2001;
int dat2000;											//data for VIA port PB
int dat2001;											//data for VIA port PA

int cnt_via_wr_0x2000;
int cnt_via_wr_0x200a;

int cnt_via_rd_0x2000;
int cnt_via_rd_0x2001;

int anode00;											//cpu writing 00->05 to VIA 0x2000, but these hold respective VIA 0x200a write vals, i.e: this is demuxed led muxing
int anode01;
int anode02;
int anode03;
int anode04;
int anode05;
int anode09;

int drv_out;

bool via_PB7;
int via_timer1_period;

vector<string> vcls;
vector<string> vcls2;
vector<string> vcls3;

//csl_wdg *wdg_csl;
//csl_wdg *wdg_csl2;
//csl_wdg *wdg_csl3;

};









class csl_wdg : public Fl_Widget
{
private:										//private var
bool have_focus;


public:
Fl_Scrollbar *scroll;
string sbuf;
int fonttype;
int fontsize;
colref col_bkgd;
colref col_text1;
int line_offs;									//set this by eye after seeing your font/size and considering your widget size
int chars_horiz, chars_vert;
int cursx, cursy;
char char_cursor;
int curs_flash_count;
bool curs_flash;
int save_cursx, save_cursy;
bool keypressed;
int keycode;

int *buf;

bool show_have_focus;

private:
void setcolref( colref col );
void force_cursor();

public:
csl_wdg( int x,int y,int w, int h,const char *label );
~csl_wdg();
void draw();
int handle( int );
bool kbhit();
void set_char_dimensions( int char_x, int char_y );
bool set_char_buf( int char_x, int char_y, char cc );
bool get_char_buf( int char_x, int char_y, char &cc );
bool scroll_up();
void add( char cc );
void add( string ss );
void backspace();
void clear();
void clear_line();
void tick( int ratio );
void move_cursor( int char_x, int char_y );
void save_cursor();
void restore_cursor();
void add_hscrollbar();

};





#endif

