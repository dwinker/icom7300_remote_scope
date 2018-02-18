// ----------------------------------------------------------------------------
// Waterfall Spectrum Analyzer Widget
// Copyright (C) 2018 Dan Winker, WV0VW
//
// This file is part of Remscp.
//
// Remscp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remscp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Remscp.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------
//#include "_waterfall.h"
#ifndef _WF_H
#define _WF_H

#include <cstdio>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>

struct RGB {
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

struct RGBI {
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char I;
};

enum WFspeed { PAUSE = 0, FAST = 1, NORMAL = 2, SLOW = 4 };

class FFTdisp : public Fl_Widget {
public:
	FFTdisp(int x, int y, int w, int h, char *lbl = 0);
    void sig_data(const unsigned char *sig,
                  unsigned int first_point_n,
                  unsigned int length);
	~FFTdisp();
	void draw();
};

class WFdisp : public Fl_Widget {
public:
	WFdisp(int x, int y, int w, int h, char *lbl = 0);
	~WFdisp();
	WFspeed Speed() { return wfspeed;}
	void Speed(WFspeed rate) { wfspeed = rate;}
    void sig_data(const unsigned char *sig,
                  unsigned int first_point_n,
                  unsigned int length);
	void draw();
private:
    WFspeed wfspeed;
};

class Scale : public Fl_Widget {
public:
	Scale(int x, int y, int w, int h, char *lbl = 0);
	~Scale();
	void draw();
};

class waterfall: public Fl_Group {
public:
	waterfall(int x, int y, int w, int h, char *lbl= 0);
	~waterfall();
    static waterfall *get_waterfall(void);

    void sig_data(unsigned char *sig,
                  unsigned int  first_point_n,
                  unsigned int  length);
	int Speed();
	void Speed(int rate);

private:
    static waterfall *wf;
	FFTdisp   *fftdisp;
	WFdisp    *wfdisp;
    Scale     *scale;
	Fl_Button *wfrate;
};

#endif
