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

//seg7.cpp
//---- v1.01	25-09-2017


#include "seg7.h"



//----------------------------------------------------------
//                a
//              ____
//           f /   / b
//            /-g-/ 
// dpl *   e /___/ c   * dpr
//             d

seg7_wdg::seg7_wdg( int x,int y,int w, int h,const char *label ) : Fl_Widget( x, y, w, h, label )
{
col_bkgd.r = 64;
col_bkgd.g = 64;
col_bkgd.b = 64;

col_seg.r = 200;
col_seg.g = 0;
col_seg.b = 0;

gapx = 8;
gapy = 8;
skew = -4;						//lower horiz skew
line_width = 2;
inverted = 0;

diode_a = 1;
diode_b = 1;
diode_c = 1;
diode_d = 1;
diode_e = 1;
diode_f = 1;
diode_g = 1;
diode_dpl = 1;
diode_dpr = 1;

}




seg7_wdg::~seg7_wdg()
{
}






void seg7_wdg::setcolref( seg7_colref col )
{
fl_color( col.r , col.g , col.b );
}



void seg7_wdg::draw()
{

//Fl_Widget::draw();
//return;

int rght,bot;
string s1;
mystr m1;

int xx = x();
int yy = y();


//int iF = fl_font();
//int iS = fl_size();

//fl_font( fonttype, fontsize );



//clear wnd
setcolref( col_bkgd );
fl_rectf( xx , yy , w() , h() );

setcolref( col_seg );
fl_line_style( FL_SOLID, line_width, 0 );			//for windos set lie style after colour


//setcolref( col_mag );

bool da = diode_a;
bool db = diode_b;
bool dc = diode_c;
bool dd = diode_d;
bool de = diode_e;
bool df = diode_f;
bool dg = diode_g;
bool ddpl = diode_dpl;
bool ddpr = diode_dpr;

if( inverted )
	{
	da = !da;
	db = !db;
	dc = !dc;
	dd = !dd;
	de = !de;
	df = !df;
	dg = !dg;
	ddpl = !ddpl;
	ddpr = !ddpr;
	}

int halfy = h() / 2;

//int gap2x = gapx * 2;
//int gap2y = gapy * 2;

if( da ) fl_line( xx + gapx,					yy + gapy,			xx + w() - gapx,				yy + gapy );
if( db ) fl_line( xx + w() - gapx,				yy + gapy,			xx + w() - gapx + skew / 2,		yy + halfy );
if( dc ) fl_line( xx + w() - gapx + skew / 2,	yy + halfy,			xx + w() - gapx + skew,			yy + h() - gapy );
if( dd ) fl_line( xx + gapx + skew,				yy + h() - gapy,	xx + w() - gapx + skew,			yy + h() - gapy );
if( de ) fl_line( xx + gapx + skew,				yy + h() - gapy,	xx + gapx + skew / 2,			yy + halfy );
if( df ) fl_line( xx + gapx + skew / 2,			yy + halfy,			xx + gapx,						yy + gapy );
if( dg ) fl_line( xx + gapx + skew / 2,			yy + halfy,			xx + w() - gapx + skew / 2,		yy + halfy );
if(ddpl) fl_line( xx + gapx + skew - 3,			yy + h() - gapy,	xx + gapx + skew - 1,			yy + h() - gapy );
if(ddpr) fl_line( xx + w() - gapx + skew + 3,	yy + h() - gapy,	xx + w() - gapx + skew + 5,		yy + h() - gapy );

//if( da ) fl_line( xx + gapx, yy + gapy, xx + w() - gapx , yy + y() - gapy );

//fl_font( iF, iS );
}


//----------------------------------------------------------


