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
#include <FL/Fl_Light_Button.H>

struct RGB {
    uchar R;
    uchar G;
    uchar B;
};

struct RGBI {
    uchar R;
    uchar G;
    uchar B;
    uchar I;
};

enum WFspeed { PAUSE = 0, FAST = 1, NORMAL = 2, SLOW = 4 };

class FFTdisp : public Fl_Widget {
public:
    FFTdisp(int x, int y, int w, int h, char *lbl = 0);
    ~FFTdisp(void);
    void update(const double *sigy_normalized,
                unsigned int first_x,
                unsigned int width);
    void draw(void);

private:
    unsigned int img_area;
    uchar       *img;
};

class WFdisp : public Fl_Widget {
public:
    WFdisp(int x, int y, int w, int h, char *lbl = 0);
    ~WFdisp(void);
    void setcolors(void);
    WFspeed Speed(void) { return wfspeed;}
    void Speed(WFspeed rate) { wfspeed = rate;}
    void update(const double *sigy_normalized,
                unsigned int first_x,
                unsigned int width);
    void draw(void);
private:
    RGBI         mag2RGBI[256];
    RGB          palette[9];
    WFspeed      wfspeed;
    unsigned int img_area;
    RGBI        *img;
    unsigned int head_y;
};

class Scale : public Fl_Widget {
public:
    Scale(int x, int y, int w, int h, char *lbl = 0);
    ~Scale(void);
    void makeScale(double f_low, double f_high);
    void draw(void);

private:
    double       freq_range;      // f_high - f_low in MHz.
    int          m_width;
    unsigned int max_nticks;      // TBD - if this moves to makeScale() it won't need to be a class variable.
    int          text_height;
    double       freq_low, freq_high;
    double       tick_delta_freq; // Distance between ticks in MHz.
    double       sub_tick_delta_freq;
    int          n_sub_ticks;
    const char  *format_string;
};

class waterfall: public Fl_Group {
public:
    waterfall(int x, int y, int w, int h, char *lbl= 0);
    ~waterfall(void);
    static waterfall *get_waterfall(void);
    void sig_data(unsigned char *sig,
                  unsigned int  first_point_n,
                  unsigned int  length);
    bool FakeFlag(void);
    void FakeFlag(bool f);
    int Speed(void);
    void Speed(int rate);
    int On(void);
    void On(int off_on);

    FFTdisp *fftdisp;
    WFdisp  *wfdisp;
    Scale   *scale;

private:
    static waterfall *wf;
    Fl_Group        *rs1;
    Fl_Button       *wfrate_btn;
    Fl_Light_Button *scope_on_btn;
    int  off_on;
    bool fake_flag;
};

#endif
