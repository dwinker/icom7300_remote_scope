// ----------------------------------------------------------------------------
// waterfall.cxx - Waterfall Spectrum Analyzer Widget
//
// Copyright (C) 2018
//      Dan Winker, WV0VW
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
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "waterfall.h"
#include "fldigi-config.h"

// Height in pixels of the frequency scale at the bottom of the waterfall display.
const int SCALE_H = 26;
// Height in pixels of the grey line between the FFT and WF displays.
const int LINE_H = 1;
// Amount of vertical space used by top FFT display. Only makes much sense to use
// numbers from about 0.1 to 0.9.
const double RATIO = 0.3;

static RGBI mag2RGBI[256];

// Force palette from ~/.fldigi/paletts/default.pal
static RGB palette[9] = {
    {  0,  0,  0},
    {  0,  6,136},
    {  0, 19,198},
    {  0, 32,239},
    {172,167,105},
    {194,198, 49},
    {225,228,107},
    {255,255,  0},
    {255, 51,  0}
};

// Sort of a singelton.
waterfall *waterfall::wf = NULL;
waterfall *waterfall::get_waterfall(void)
{
    return wf;
}

FFTdisp::FFTdisp(int x0, int y0, int w0, int h0, char *lbl) : Fl_Widget(x0, y0, w0, h0, lbl)
{
}

FFTdisp::~FFTdisp()
{
}

void FFTdisp::draw()
{
    fl_color(FL_DARK3);
    fl_rectf(x(), y(), w(), h());
    redraw();
}

WFdisp::WFdisp(int x0, int y0, int w0, int h0, char *lbl) : Fl_Widget(x0, y0, w0, h0, lbl)
{
}

WFdisp::~WFdisp()
{
}

void WFdisp::sig_data(
    const unsigned char *sig,
    unsigned int first_point_n,
    unsigned int length)
{
}

void WFdisp::draw()
{
    fl_color(FL_BLUE);
    fl_rectf(x(), y(), w(), h());
    redraw();
}

Scale::Scale(int x0, int y0, int w0, int h0, char *lbl) : Fl_Widget(x0, y0, w0, h0, lbl)
{
}

Scale::~Scale()
{
}

void Scale::draw()
{
    fl_color(FL_DARK_GREEN);
    fl_rectf(x(), y(), w(), h());
    redraw();
}

waterfall::waterfall(int x0, int y0, int w0, int h0, char *lbl) : Fl_Group(x0, y0, w0, h0, lbl)
{
    int   fft_y,   fft_h;
    int  line_y;
    int    wf_y,    wf_h;
    int scale_y;

    fft_y   = BEZEL;
    fft_h   = static_cast<int>(RATIO * (double)h0);
    line_y  = fft_y + fft_h;
    wf_y    = line_y + LINE_H;
    wf_h    = h0 - fft_h - LINE_H - SCALE_H;
    scale_y = wf_y + wf_h;

    fftdisp = new FFTdisp(BEZEL,   fft_y, w0,   fft_h, (char *)"fftdisp");
    wfdisp  = new WFdisp( BEZEL,    wf_y, w0,    wf_h, (char *)"wfdisp");
    scale   = new Scale(  BEZEL, scale_y, w0, SCALE_H, (char *)"scale");
}

waterfall::~waterfall()
{
    delete fftdisp;
    delete wfdisp;
    delete scale;
}

void waterfall::sig_data(
    unsigned char *sig,
    unsigned int  first_point_n,
    unsigned int  length)
{
    wfdisp->sig_data(sig, first_point_n, length);
}

int waterfall::Speed() {
    return (int)wfdisp->Speed();
}

void waterfall::Speed(int rate)
{
    WFspeed speed = static_cast<WFspeed>(rate);
    wfdisp->Speed(speed);

    const char* label;
    switch (speed) {
    case SLOW:
        label = "SLOW";
        break;
    case NORMAL: default:
        label = "NORM";
        break;
    case FAST:
        label = "FAST";
        break;
    case PAUSE:
        label = "PAUSE";
        break;
    }

    wfrate->label(label);
    wfrate->redraw_label();
}

