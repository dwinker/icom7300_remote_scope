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
#include <climits>
#include <float.h>
#include <math.h>
#include <assert.h>
//#include <cstdio>
#include "waterfall.h"
#include "ic7300.h"
#include "serial.h"
#include "scope_waveform_data.h"
#include "fldigi-config.h"
#include "progdefaults.h"

// For use with providing the necessary slop in floating point compares and
// calculations.  Because floats and doubles are actually stored as a base 2
// mantissa and exponent, often times a calculation that should end up being
// 0.10000000 ends up being something like 0.09999948 instead.
// Experimentation says '* 1000.0' is about right.
const double EPSILON = DBL_EPSILON * 1000.0;

// Height in pixels of the frequency scale at the bottom of the waterfall display.
const int SCALE_H = 26;
// Height in pixels of the grey line between the FFT and WF displays.
const int LINE_H = 1;
// Amount of vertical space used by top FFT display. Only makes much sense to use
// numbers from about 0.1 to 0.5.
const double RATIO = 0.2;
// Number of pixels between Scale tick and edge of Scale area, Scale tick and
// Scale tick label, or Scale tick label and edge of Scale area.
const int SCALE_MARGIN = 2;
// Sub tick is 1/3 the height of a regular tick.
const int SUB_TICK_HEIGH_DIVISOR = 3;

const size_t MAX_CHUNK_WIDTH = 256;

// Sort of a singelton.
waterfall *waterfall::wf = NULL;
waterfall *waterfall::get_waterfall(void)
{
    return wf;
}

static void wfrate_btn_cb(Fl_Widget *w, void *v);
static void scope_on_btn_cb(Fl_Widget *w, void *vi);

FFTdisp::FFTdisp(int x0, int y0, int w0, int h0, char *lbl) : Fl_Widget(x0, y0, w0, h0, lbl)
{
    img_area = w0 * h0;
    img = new uchar[img_area];
    memset(img, 0, img_area);
}

FFTdisp::~FFTdisp()
{
    //delete img;
}

void FFTdisp::update(
    const double *sigy_normalized,
    unsigned int first_x,
    unsigned int width)
{
    // TBD - Could keep track of which parts of img are dirty and communicate
    // that to draw() so only part of the image would have to be updated each
    // time.
    int disp_w;
    double sigy_norm;
    unsigned int ymax, ynext;
    static unsigned int ylast;
    unsigned int x, sigpixel;
    const int WHITE      = 255;
    const int GRAYLEVEL  = 255; // Set to 50 to see data points brighter than lines.
    const int BACKGROUND =  25; // Set to 0 for black background, 25 to check background.

    disp_w = w();
    ymax   = h()- 1;

    if(0 == first_x)
        ylast = ymax;

    x = first_x;
    for(unsigned int i = 0; i < width; i++) {

        // Clear this column.
        for(unsigned int y = 0; y <= ymax; y++) {
            sigpixel = x + y * disp_w;
            //assert(sig_image_area > sigpixel);
            img[sigpixel] = BACKGROUND;
        }

        sigy_norm= sigy_normalized[i];
        assert(1.0 >= sigy_norm);
        ynext = static_cast<unsigned int>((ymax * (1.0 - sigy_norm)));

        while(ylast < ynext) {
            sigpixel = x + ylast * disp_w;
            //assert(sig_image_area > sigpixel);
            img[sigpixel] = GRAYLEVEL;
            ylast++;
        }

        while(ylast > ynext) {
            sigpixel = x + ylast * disp_w;
            //assert(sig_image_area > sigpixel);
            img[sigpixel] = GRAYLEVEL;
            ylast--;
        }

        sigpixel = x + ynext * disp_w;
        //assert(sig_image_area > sigpixel);
        img[sigpixel] = WHITE;

        x++;
    }
    redraw();
}

void FFTdisp::draw()
{
    uchar *pixmap = static_cast<uchar *>(img);
    fl_color(FL_DARK3);
    fl_rectf(x(), y(), w(), h());
    fl_draw_image_mono(pixmap, x(), y(), w(), h());
}

WFdisp::WFdisp(int x0, int y0, int w0, int h0, char *lbl) : Fl_Widget(x0, y0, w0, h0, lbl)
{
    // Force palette from ~/.fldigi/paletts/default.pal. In fldigi palette is
    // read from a configuration file. This is just a simple hack to get it
    // working here for now.
    palette[0] = {  0,   0,   0};
    palette[1] = {  0,   6, 136};
    palette[2] = {  0,  19, 198};
    palette[3] = {  0,  32, 239};
    palette[4] = {172, 167, 105};
    palette[5] = {194, 198,  49};
    palette[6] = {225, 228, 107};
    palette[7] = {255, 255,   0};
    palette[8] = {255,  51,   0};

    setcolors();

    img_area = w0 * h0;
    img = new RGBI[img_area];
    memset(img, 0, img_area);
    head_y = 0;

    Speed(NORMAL);
}

WFdisp::~WFdisp()
{
    //delete img;
}

void WFdisp::setcolors(void)
{
    double di;
    int r, g, b;
    for (int i = 0; i < 256; i++) {
        di = sqrt((double)i / 256.0);
        mag2RGBI[i].I = (uchar)(200*di);
    }
    for (int n = 0; n < 8; n++) {
        for (int i = 0; i < 32; i++) {
            r = palette[n].R + (int)(1.0 * i * (palette[n+1].R - palette[n].R) / 32.0);
            g = palette[n].G + (int)(1.0 * i * (palette[n+1].G - palette[n].G) / 32.0);
            b = palette[n].B + (int)(1.0 * i * (palette[n+1].B - palette[n].B) / 32.0);
            mag2RGBI[i + 32*n].R = r;
            mag2RGBI[i + 32*n].G = g;
            mag2RGBI[i + 32*n].B = b;
        }
    }
}

void WFdisp::update(
    const double *sigy_normalized,
    unsigned int first_x,
    unsigned int width)
{
    // TBD - Could keep track of which parts of img are dirty and communicate
    // that to draw() so only part of the image would have to be updated each
    // time.
    static unsigned int         last_first_x = UINT_MAX;
    const double * __restrict__ sigy_norm;
    RGBI         * __restrict__ rgbi;

    if(first_x < last_first_x) {
        // We are on a new row.  Do this at the beginning of this method.
        // draw() needs to know where this method wrote to.
        if(0 == head_y)
            head_y = h() - 1;
        else
            head_y--;
    }
    last_first_x = first_x;

    // WFdisp::img is a circular buffer of line records. The lines are
    // segmented into width wide segments. The very first call to this method
    // fills in one line and that is drawn at the top of the image. Each
    // subsequent call fills in one line above the previous until the top of
    // img is reached. Then we go back to the bottom of img. The WFdisp::draw()
    // function knows how to take apart the circular buffer and draw two
    // seperate images to complete the fill image.
    sigy_norm = sigy_normalized;
    rgbi      = &img[head_y * w() + first_x];
    while(width--) {
        unsigned char sig;
        sig = static_cast<unsigned char>(round(*sigy_norm * 255.0));
        *rgbi = mag2RGBI[sig];
        //printf("%d %d %d %d %d\n", sig, rgbi->R, rgbi->G, rgbi->B, rgbi->I);
        sigy_norm++;
        rgbi++;
    }

    redraw();
}

void WFdisp::draw()
{
    uchar *pixmap_a, *pixmap_b;
    unsigned int disp_x, disp_y, disp_w, disp_h;
    unsigned int a_y, a_h, b_y, b_h;

    disp_x = x();
    disp_y = y();
    disp_w = w();
    disp_h = h();

    pixmap_a = (uchar *)&img[head_y * disp_w];
    pixmap_b = (uchar *)&img[0];

    a_y = disp_y;
    a_h = disp_h - head_y;

    b_y = disp_y + disp_h - head_y;
    b_h = head_y;

    fl_draw_image(pixmap_a, disp_x, a_y, disp_w, a_h, sizeof(RGBI), disp_w * sizeof(RGBI));
    fl_draw_image(pixmap_b, disp_x, b_y, disp_w, b_h, sizeof(RGBI), disp_w * sizeof(RGBI));

    //printf("%p %p %u %u %u %u %u\n", pixmap_a, pixmap_b, head_y, a_y, a_h, b_y, b_h);
}

Scale::Scale(int x0, int y0, int w0, int h0, char *lbl) : Fl_Widget(x0, y0, w0, h0, lbl)
{
    // Could get much closer calculating max_nticks in makeScale() using
    // f_high, knowing it's right justified and the adjacent label might be the
    // same size and will be center justified. But the dilema is, we want to
    // know max_nticks before we decide how many decimal places in each label
    // and we want to know how many decimal places before calculating
    // max_nticks from f_high. This seems like the bigest label we'll ever want.
    const char test_label[] = "66.666";
    const char test_m[]     = "M";
    int tx, ty, label_width;

    fl_font(progdefaults.WaterfallFontnbr, progdefaults.WaterfallFontsize);

    // Determine max_nticks by dividing width by how wide the widest tick label might be.
    fl_text_extents(test_m,     tx, ty, m_width,     text_height);
    fl_text_extents(test_label, tx, ty, label_width, text_height);
    max_nticks = w() / (m_width + label_width);

    printf("Scale::Scale: m_width=%d, label_width=%d, text_height=%d max_nticks=%u\n",
            m_width, label_width, text_height, max_nticks);

    makeScale(10.1, 10.13);
}

// Scale ticks are an aggravating thing to try to figure out. I got some inspiration from these:
// https://stackoverflow.com/questions/326679/choosing-an-attractive-linear-scale-for-a-graphs-y-axis
// https://stackoverflow.com/questions/8506881/nice-label-algorithm-for-charts-with-minimum-ticks/16363437#16363437
// http://austinclemens.com/blog/2016/01/09/an-algorithm-for-creating-a-graphs-axes/
void Scale::makeScale(double f_low, double f_high)
{
    // f_low and f_high are in MHz. The calculations here are split apart into
    // many local variables because they're hard to follow and the scale
    // doesn't change that often, so we can afford to take a little more CPU
    // time for clarity in the code.
    double df_approx;     // Approximate delta frequency between ticks.
    double log_dfa;       // Logarithm base 10 of df_approx.
    double f_ldfa;        // Fractional part of log_dfa.
    double e_ldfa;        // Exponent part of log_dfa.
    double td_rel_approx; // Tick distance, relative, approximate.
    double td_rel;        // Tick distance, relative.

    freq_low  = f_low;
    freq_high = f_high;

    if(1 == max_nticks) {
        puts("No point in max_nticks = 1.");
    } else if(2 == max_nticks) {
        puts("f_low and f_high are the two ticks.");
    }

    // Use logarithms to bring our frequency range to a number between 0.1 and
    // 1.0 and an exponent we can use later to bring it back to what the
    // frequency range really is.
    freq_range = f_high - f_low;
    df_approx  = freq_range / (max_nticks - 1);
    log_dfa    = log10(df_approx);
    f_ldfa     = modf(log_dfa, &e_ldfa);
    while(0.0 > f_ldfa) {
        --e_ldfa;
        f_ldfa += 1.0;
    }
    td_rel_approx = pow(10, f_ldfa - 1.0);
    assert(0.1 <= td_rel_approx && td_rel_approx <= 1.0);

    // Here's where we make the tick distance nice numbers. I think 1, 2, 5,
    // 10, etc. is a nice pattern.
    if(td_rel_approx < (0.1 + EPSILON)) {
        td_rel      = 0.1;
        n_sub_ticks = 5;
    }
    else if(td_rel_approx < (0.2 + EPSILON)) {
        td_rel      = 0.20;
        n_sub_ticks = 4;
    }
    else if(td_rel_approx < (0.5 + EPSILON)) {
        td_rel      = 0.5;
        n_sub_ticks = 5;
    }
    else {
        td_rel      = 1.0;
        n_sub_ticks = 5;
    }

    // Here's where we use that exponent to bring the result back to MHz.
    tick_delta_freq = td_rel * pow(10, (e_ldfa + 1.0));
    sub_tick_delta_freq = tick_delta_freq / n_sub_ticks;

    printf("Scale::makeScale: freq_range=%f, df_approx=%f, log_dfa=%f, f_ldfa=%f, e_ldfa=%f, td_rel_approx=%f, td_rel=%f, tick_delta_freq=%f\n",
            freq_range, df_approx, log_dfa, f_ldfa, e_ldfa, td_rel_approx, td_rel, tick_delta_freq);

    if((0.01 - EPSILON) > tick_delta_freq)
        format_string = "%.3f";
    else if((0.1 - EPSILON) > tick_delta_freq)
        format_string = "%.2f";
    else if((1.0 - EPSILON) > tick_delta_freq)
        format_string = "%.1f";
    else if((10.0 - EPSILON) > tick_delta_freq)
        format_string = "%.0f";
    else
        format_string = "%f";

    redraw();
}

Scale::~Scale()
{
}

void Scale::draw()
{
    static char szFreq[20];
    int tx, ty, tw, th;
    int tick_x, text_x, text_x_drawn, sub_tick_x;
    double f, f_middle, f_done;
    int n;
    int nticks = 0;       // Count of ticks we have made.

    fl_color(65);
    fl_rectf(x(), y(), w(), h());
    fl_color(fl_rgb_color(228));

    printf("<%f< %d\n", freq_low, ++nticks);

    // Start from the left and go halfway. Then start from the right and go
    // back halfway. That way the edge labels can crowd the labels closer to
    // the middle and make room for more labels.
    f_middle = (freq_low + freq_high) / 2.0;

    snprintf(szFreq, sizeof(szFreq), format_string, freq_low);
    fl_text_extents(szFreq, tx, ty, tw, th);
    tick_x = x();
    text_x = tick_x + SCALE_MARGIN; // Left justified.
    text_x_drawn = text_x + tw;
    fl_draw(szFreq, text_x, y() + h() - SCALE_MARGIN);
    fl_line(tick_x, y(), tick_x, y() + h() - text_height - 2 * SCALE_MARGIN - 1);

    for(n = 1; n < n_sub_ticks; n++) {
        sub_tick_x = tick_x + n * sub_tick_delta_freq * (w() - 1) / freq_range;
        fl_line(sub_tick_x, y(), sub_tick_x, y() + (h() - text_height) / SUB_TICK_HEIGH_DIVISOR);
    }

    f = tick_delta_freq * ceil(freq_low / tick_delta_freq - EPSILON) + tick_delta_freq;
    while(f <= f_middle) {
        snprintf(szFreq, sizeof(szFreq), format_string, f);
        fl_text_extents(szFreq, tx, ty, tw, th);
        tick_x = x() + (f - freq_low) * (w() - 1) / freq_range;
        text_x = tick_x - tw / 2; // Center justified.
        if(text_x < text_x_drawn + m_width)
            text_x = text_x_drawn + m_width;
        text_x_drawn = text_x + tw;
        fl_draw(szFreq, text_x, y() + h() - SCALE_MARGIN);
        fl_line(tick_x, y(), tick_x, y() + h() - text_height - 2 * SCALE_MARGIN - 1);

        for(n = 1; n < n_sub_ticks; n++) {
            sub_tick_x = tick_x + n * sub_tick_delta_freq * (w() - 1) / freq_range;
            fl_line(sub_tick_x, y(), sub_tick_x, y() + (h() - text_height) / SUB_TICK_HEIGH_DIVISOR);
        }

        printf("|%f| %d\n", f, ++nticks);

        f_done = f; // Next loop we'll know exactly which tick we finished here.
        f += tick_delta_freq;
    }

    snprintf(szFreq, sizeof(szFreq), format_string, freq_high);
    fl_text_extents(szFreq, tx, ty, tw, th);
    tick_x = x() + w() - 1;
    text_x = tick_x - tw - SCALE_MARGIN; // Right justified.
    text_x_drawn = text_x - 1;
    fl_draw(szFreq, text_x, y() + h() - SCALE_MARGIN);
    fl_line(tick_x, y(), tick_x, y() + h() - text_height - 2 * SCALE_MARGIN - 1);

    printf(">%f> (%f) %d\n", freq_high, f, ++nticks);

    f = freq_high / tick_delta_freq - EPSILON;
    f = ceil(f);
    f *= tick_delta_freq;
    f -= tick_delta_freq;
    while(f > f_done) {
        snprintf(szFreq, sizeof(szFreq), format_string, f);
        fl_text_extents(szFreq, tx, ty, tw, th);
        tick_x = x() + (f - freq_low) * (w() - 1) / freq_range;
        text_x = tick_x - tw / 2; // Center justified.
        if((text_x + tw + m_width) > text_x_drawn)
            text_x = text_x_drawn - tw - m_width;
        text_x_drawn = text_x - 1;
        fl_draw(szFreq, text_x, y() + h() - SCALE_MARGIN);
        fl_line(tick_x, y(), tick_x, y() + h() - text_height - 2 * SCALE_MARGIN - 1);

        for(n = 1; n < n_sub_ticks; n++) {
            sub_tick_x = tick_x + n * sub_tick_delta_freq * (w() - 1) / freq_range;
            fl_line(sub_tick_x, y(), sub_tick_x, y() + (h() - text_height) / SUB_TICK_HEIGH_DIVISOR);
        }

        printf("|%f| %d\n", f, ++nticks);

        f -= tick_delta_freq;
    }
}

// waterfall is an Fl_Group that holds the two signal displays and some
// buttons. The responsibility of the constructor is to arrange the items
// within the group.
waterfall::waterfall(int x0, int y0, int w0, int h0, char *lbl) : Fl_Group(x0, y0, w0, h0, lbl)
{
    // Note, the line is not explicitly drawn for now. It is just the
    // background of the Fl_Group.
    int children_h;
    int child_x, child_w;
    int   fft_y,   fft_h;
    int  line_y;
    int    wf_y,    wf_h;
    int scale_y;

    // Used for button placement.
    const int bwRate    = 45;
    const int bwFftWfOn = 32;
    const int wSpace    =  1;
    const int bwdths    = (wSpace + bwRate + wSpace + bwFftWfOn + wSpace);
    int xpos;
    float ratio;
    int buttonrow;

    children_h = h0 - BTN_HEIGHT - 4;
    buttonrow  = h0 + y0 - BTN_HEIGHT - 1;

    child_x = x0 + BEZEL;
    child_w = w0 - 2 * BEZEL;
    fft_y   = y0 + BEZEL;
    fft_h   = static_cast<int>(RATIO * (double)h0);
    line_y  = fft_y + fft_h;
    wf_y    = line_y + LINE_H;
    wf_h    = children_h - fft_h - LINE_H - SCALE_H - 2 * BEZEL;
    scale_y = wf_y + wf_h;

    ratio     = w0 * 1.0 / bwdths;
    buttonrow = h0 + y0 - BTN_HEIGHT - 1;
    xpos      = x0 + wSpace;

    rs1 = new Fl_Group(x0, y0, w0, children_h);
        rs1->box(FL_DOWN_BOX);
        fftdisp = new FFTdisp(child_x,   fft_y, child_w,   fft_h, (char *)"fftdisp");
        wfdisp  = new WFdisp( child_x,    wf_y, child_w,    wf_h, (char *)"wfdisp");
        scale   = new Scale(  child_x, scale_y, child_w, SCALE_H, (char *)"scale");
        rs1->resizable(fftdisp);
        rs1->resizable(wfdisp);
        rs1->resizable(scale);
    rs1->end();

    wfrate_btn = new Fl_Button(xpos, buttonrow, (int)(bwRate*ratio), BTN_HEIGHT, "Norm");
    wfrate_btn->callback(wfrate_btn_cb, 0);
    wfrate_btn->tooltip("Waterfall drop speed");
    xpos = xpos + (int)(bwRate*ratio) + wSpace;

    scope_on_btn = new Fl_Light_Button(xpos, buttonrow, (int)(bwFftWfOn*ratio), BTN_HEIGHT, "Off");
    scope_on_btn->callback(scope_on_btn_cb, 0);
    scope_on_btn->value(0);
    scope_on_btn->selection_color(FL_GREEN);
    scope_on_btn->tooltip("FFT and Waterfall Data From Radio Activated");
    //xpos = xpos + (int)(bwFftWfOn*ratio) + wSpace;

    off_on = 0;

    wf = this;
}

waterfall::~waterfall()
{
    //delete fftdisp;
    //delete wfdisp;
    //delete scale;
}

// This class that contains the data displayer objects manages the normalized
// signal data and passes it onto the displayer objects.
void waterfall::sig_data(
    unsigned char *sig,
    unsigned int  first_point_n,
    unsigned int  length)
{
    // This stays around because we often need last time's last value for this
    // time's first interpolation.
    static unsigned char scope_data_span[SCOPE_DATA_SPAN_SIZE];

    double sigy_normalized[MAX_CHUNK_WIDTH];
    unsigned int first_sigx;
    unsigned int last_point_n;
    unsigned int last_sigx;
    unsigned int sigx;

    assert((first_point_n + length) <= (sizeof(scope_data_span) / sizeof(scope_data_span[0])));

    memcpy(scope_data_span + first_point_n, sig, length);

    if(0 == first_point_n)
        first_sigx = 0;
    else
        first_sigx = 1 + (first_point_n - 1) * (w() - 1) / (SCOPE_DATA_SPAN_SIZE - 1);

    last_point_n = first_point_n + length - 1;
    last_sigx    = last_point_n * (w() - 1) / (SCOPE_DATA_SPAN_SIZE - 1);

    assert(MAX_CHUNK_WIDTH > (last_sigx - first_sigx));

    // printf("first_point_n=%3d length=%2d last_point_n=%3d first_sigx=%3d last_sigx=%3d\n",
    //         first_point_n,   length,   last_point_n,   first_sigx,   last_sigx);
    // puts("nf nmf npf nmi npi m data_a data_b sigx sigy_normalized[sigx]");

    sigx = first_sigx;
    while(sigx <= last_sigx) {
        double nf;    // n float
        double nmf;   // n minus float
        double npf;   // n plus  float
        int    nmi;   // n minus integer
        int    npi;   // n plus  integer
        double m;     // slope for y = mx + b.
        unsigned char        data_a;
        static unsigned char data_b;

        // nf is the value of n (x axis value of discrete scope values) between
        // the two integer n values for which we actually have scope values.
        nf = static_cast<double>(sigx) * (SCOPE_DATA_SPAN_SIZE - 1.0) / (w() - 1.0);
        nmf = floor(nf);
        npf =  ceil(nf);
        nmi = static_cast<int>(nmf);
        npi = static_cast<int>(npf);
        data_a = scope_data_span[nmi];
        data_b = scope_data_span[npi];

        if(nmi == npi) {
            // No interpolation. scope_data_spany equals scope_data_span[nmi]
            // which is the same as scope_data_span[npi].
            m = 0.0;
        } else {
            m = static_cast<double>(data_b - data_a) / (npf - nmf);
        }

        sigy_normalized[sigx - first_sigx] = (static_cast<double>(data_a) + m * (nf - nmf)) /
                                              static_cast<double>(SCOPE_DATA_MAX_VALUE);

        //printf("%7.3f %7.3f %7.3f %2d %2d %+7.3f %3d %3d %2d %f\n",
        //        nf,   nmf,  npf,  nmi,npi,m, data_a, data_b, sigx, sigy_normalized[sigx - first_sigx]);

        sigx++;
    }

    Fl::lock();
    fftdisp->update(sigy_normalized, first_sigx, last_sigx - first_sigx + 1);
    wfdisp->update( sigy_normalized, first_sigx, last_sigx - first_sigx + 1);
    Fl::unlock();
    Fl::awake();
}

bool waterfall::FakeFlag(void)
{
    return fake_flag;
}

void waterfall::FakeFlag(bool f)
{
    fake_flag = f;
}

int waterfall::Speed(void)
{
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

    wfrate_btn->label(label);
    wfrate_btn->redraw_label();
}

static void wfrate_btn_cb(Fl_Widget *w, void *v)
{
    waterfall* wf = static_cast<waterfall *>(w->parent());
    WFspeed new_speed;

    switch (wf->wfdisp->Speed()) {
    case SLOW:
        new_speed = PAUSE;
        break;
    case NORMAL: default:
        new_speed = SLOW;
        break;
    case FAST:
        new_speed = NORMAL;
        break;
    case PAUSE:
        new_speed = FAST;
        break;
    }

    wf->Speed(new_speed);
}

int waterfall::On(void)
{
    return off_on;
}

void waterfall::On(int off_on)
{
    if(off_on) {
        if(fake_flag) {
            start_scope_waveform_thread();
        } else {
            send_scope_wave_output_on();
        }
        scope_on_btn->label("On");
    } else {
        if(fake_flag) {
            stop_scope_waveform_thread();
        } else {
            send_scope_wave_output_off();
        }
        scope_on_btn->label("Off");
    }
    scope_on_btn->redraw_label();
}

static void scope_on_btn_cb(Fl_Widget *w, void *vi)
{
    int v;
    waterfall* wf;
    v  = reinterpret_cast<Fl_Light_Button *>(w)->value();
    wf = static_cast<waterfall *>(w->parent());
    wf->On(v);
}
