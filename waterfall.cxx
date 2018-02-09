// ----------------------------------------------------------------------------
// waterfall.cxx - Waterfall Spectrum Analyzer Widget
//
// Copyright (C) 2006-2010
//      Dave Freese, W1HKJ
// Copyright (C) 2007-2010
//      Stelios Bounanos, M0GLD
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <assert.h>
#include <queue>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <FL/names.h>
//DW #include <FL/Fl_Repeat_Button.H>
//DW #include <FL/Fl_Light_Button.H>
//DW #include <FL/Fl_Menu_Button.H>
//DW #include <FL/Fl_Group.H>
//DW #include <FL/Fl_Box.H>
//DW #include <FL/Fl_Counter.H>
//DW #include <FL/Enumerations.H>

#include <fl_lock.h>
#include "waterfall.h"
//DW #include "qrunner.h"
//DW #include "threads.h"

//#include "fldigi-config.h"
#include "flslider2.h"

#include "progdefaults.h"
#include "ic7300.h"
#include "scope_waveform_data.h"
#include "serial.h"

using namespace std;

#define bwFFT       30
#define cwRef       50
#define bwX1        25
#define bwMov       18
#define bwRate      45
#define cwCnt       92
#define bwQsy       32
#define bwXmtLock   32
#define bwRev       32
#define bwMem       40
#define bwXmtRcv    40
#define wSpace      1

#define bwdths  (wSpace + bwFFT + wSpace + cwRef + wSpace + cwRef + wSpace + bwX1 + \
                wSpace + 3*bwMov + wSpace + bwRate + wSpace + \
                cwCnt + wSpace + bwQsy + wSpace + bwMem + wSpace + \
                bwXmtLock + wSpace + bwRev + wSpace + bwXmtRcv + wSpace)

//DW extern modem *active_modem;

static  RGB RGByellow   = {254,254,0};
//static    RGB RGBgreen    = {0,254,0};
//static    RGB RGBdkgreen  = {0,128,0};
//static    RGB RGBblue     = {0,0,255};
static  RGB RGBred      = {254,0,0};
//static    RGB RGBwhite    = {254,254,254};
//static    RGB RGBblack    = {0,0,0};
//static RGB RGBmagenta = {196,0,196};
//static RGB RGBblack   = {0,0,0};

// RGBI is a structure consisting of the values RED, GREEN, BLUE, INTENSITY
// each value can range from 0 (extinguished) to 255 (full on)
// the INTENSITY value is used for the grayscale waterfall display

RGBI    mag2RGBI[256];
RGB     palette[9];

short int *tmp_fft_db;

// Sort of a singelton.
waterfall *waterfall::wf = NULL;
waterfall *waterfall::get_waterfall(void)
{
    return wf;
}

//DW static pthread_mutex_t waterfall_mutex = PTHREAD_MUTEX_INITIALIZER;

WFdisp::WFdisp (int x0, int y0, int w0, int h0, char *lbl) :
              Fl_Widget(x0,y0,w0,h0,"") {
    disp_width = w();
    scale_width = disp_width * 2;
    image_height = h() - WFTEXT - WFSCALE - WFMARKER;
    image_area      = disp_width * image_height;
    sig_image_area  = disp_width * h();
    RGBsize         = sizeof(RGB);
    RGBwidth        = RGBsize * scale_width;
    fft_img         = new RGBI[image_area];
    markerimage     = new RGB[scale_width * WFMARKER];
    scaleimage      = new uchar[scale_width * WFSCALE];
    scline          = new uchar[scale_width];
    fft_sig_img     = new uchar[image_area];
    sig_img         = new uchar[sig_image_area];

    mag = 1;
    step = 4;
    offset = 0;
    sigoffset = 0;
    ampspan = 75;
//  reflevel = -10;
    initmaps();
    bandwidth = 32;
    RGBmarker = RGBred;
    RGBcursor = RGByellow;
    RGBInotch.I = progdefaults.notchRGBI.I;
    RGBInotch.R = progdefaults.notchRGBI.R;
    RGBInotch.G = progdefaults.notchRGBI.G;
    RGBInotch.B = progdefaults.notchRGBI.B;
//  mode = WATERFALL;
    mode = SCOPE;
    centercarrier = false;
    overload = false;
    peakaudio = 0.0;
    rfc = 0L;
    usb = true;
    wfspeed = NORMAL;
    srate = 8000;
    wfspdcnt = 0;
//DW    dispcnt = 1.0 * WFBLOCKSIZE / SC_SMPLRATE;
//DW    dispdec = 1.0 * WFBLOCKSIZE / srate;
    wantcursor = false;
    cursormoved = false;

    carrier(100);

    oldcarrier = newcarrier = 0;
    tmp_carrier = false;

    for (int i = 0; i < 256; i++)
        mag2RGBI[i].I = mag2RGBI[i].R = mag2RGBI[i].G = mag2RGBI[i].B = 0;
}

WFdisp::~WFdisp() {
    delete [] scaleimage;
    delete [] markerimage;
    delete [] sig_img;
    delete [] scline;
}

void WFdisp::initMarkers() {
    uchar *c1 = (uchar *)markerimage,
          *c2 = c1 + RGBwidth * (WFMARKER - 1);
    memset(c1, 196, RGBwidth);
    memset(c2, 196, RGBwidth);
}

// draw a marker of specified width and colour centred at freq and clrM
inline void WFdisp::makeMarker_(int width, const RGB* color, int freq, const RGB* clrMin, RGB* clrM, const RGB* clrMax)
{
//DW    if (!active_modem) return;
//DW    trx_mode marker_mode = active_modem->get_mode();
//DW    if (marker_mode == MODE_RTTY) {
//DW    // rtty has two bandwidth indicators on the waterfall
//DW    // upper and lower frequency
//DW        int shift = static_cast<int>(
//DW            (progdefaults.rtty_shift < rtty::numshifts ?
//DW                rtty::SHIFT[progdefaults.rtty_shift] :
//DW                progdefaults.rtty_custom_shift));
//DW        int bw_limit_hi = (int)(shift / 2 + progdefaults.RTTY_BW / 2.0);
//DW        int bw_limit_lo = (int)(shift / 2 - progdefaults.RTTY_BW / 2.0);
//DW        int bw_freq = static_cast<int>(freq + 0.5);
//DW        int bw_lower1 = -bw_limit_hi;
//DW        int bw_upper1 = -bw_limit_lo;
//DW        int bw_lower2 = bw_limit_lo;
//DW        int bw_upper2 = bw_limit_hi;
//DW        if (bw_lower1 + bw_freq < 0)
//DW            bw_lower1 -= bw_lower1 + bw_freq;
//DW        if (bw_upper1 + bw_freq < 0)
//DW            bw_lower2 -= bw_lower2 + bw_freq;
//DW        if (bw_upper2 + bw_freq > scale_width)
//DW            bw_upper2 -= bw_upper2 + bw_freq - scale_width;
//DW        if (bw_lower2 + bw_freq > scale_width)
//DW            bw_lower2 -= bw_lower2 + bw_freq - scale_width;
//DW    // draw it
//DW        RGB* clrPos;
//DW        for (int y = 0; y < WFMARKER - 2; y++) {
//DW            for (int x = bw_lower1; x < bw_upper1; x++) {
//DW                clrPos = clrM + x + y * scale_width;
//DW                if (clrPos > clrMin && clrPos < clrMax)
//DW                    *clrPos = *color;
//DW            }
//DW            for (int x = bw_lower2; x < bw_upper2; x++) {
//DW                clrPos = clrM + x + y * scale_width;
//DW                if (clrPos > clrMin && clrPos < clrMax)
//DW                    *clrPos = *color;
//DW            }
//DW        }
//DW        return;
//DW    }
//DW 
//DW    int bw_lower = -width, bw_upper = width;
//DW 
//DW    if (marker_mode >= MODE_MT63_500S && marker_mode <= MODE_MT63_2000L)
//DW            bw_upper = (int)(width * 31 / 32);
//DW 
//DW    if (marker_mode == MODE_FSQ || marker_mode == MODE_IFKP)
//DW            bw_upper = (int)(width * 32 / 33);
//DW 
//DW    if (bw_lower + static_cast<int>(freq+0.5) < 0)
//DW        bw_lower -= bw_lower + static_cast<int>(freq+0.5);
//DW 
//DW    if (bw_upper + static_cast<int>(freq+0.5) > scale_width)
//DW        bw_upper -= bw_upper + static_cast<int>(freq+0.5) - scale_width;
//DW 
//DW    // draw it
//DW    RGB* clrPos;
//DW    for (int y = 0; y < WFMARKER - 2; y++) {
//DW        for (int x = bw_lower; x < bw_upper; x++) {
//DW            clrPos = clrM + x + y * scale_width;
//DW            if (clrPos > clrMin && clrPos < clrMax)
//DW                *clrPos = *color;
//DW        }
//DW    }
}

void WFdisp::makeMarker()
{
//DW    if (unlikely(!active_modem))
//DW        return;
//DW 
//DW    RGB *clrMin, *clrMax, *clrM;
//DW    clrMin = markerimage + scale_width;
//DW    clrMax = clrMin + (WFMARKER - 2) * scale_width;
//DW    memset(clrMin, 0, RGBwidth * (WFMARKER - 2));
//DW    clrM = clrMin + (int)((double)carrierfreq + 0.5);
//DW 
//DW    int marker_width = bandwidth;
//DW    int mode = active_modem->get_mode();
//DW    if (mode >= MODE_PSK_FIRST && mode <= MODE_PSK_LAST)
//DW        marker_width += mailserver ? progdefaults.ServerOffset :
//DW            progdefaults.SearchRange;
//DW    else if (mode >= MODE_FELDHELL && mode <= MODE_HELL80)
//DW        marker_width = (int)progdefaults.HELL_BW;
//DW    else if (mode == MODE_RTTY)
//DW        marker_width = static_cast<int>((progdefaults.rtty_shift < rtty::numshifts ?
//DW                  rtty::SHIFT[progdefaults.rtty_shift] :
//DW                  progdefaults.rtty_custom_shift));
//DW    marker_width = (int)(marker_width / 2.0 + 1);
//DW 
//DW    RGBmarker.R = progdefaults.bwTrackRGBI.R;
//DW    RGBmarker.G = progdefaults.bwTrackRGBI.G;
//DW    RGBmarker.B = progdefaults.bwTrackRGBI.B;
//DW 
//DW    makeMarker_(marker_width, &RGBmarker, carrierfreq, clrMin, clrM, clrMax);
//DW 
//DW    if (unlikely(active_modem->freqlocked() || mode == MODE_FSQ)) {
//DW        int txfreq = static_cast<int>(active_modem->get_txfreq());
//DW        adjust_color_inv(RGBmarker.R, RGBmarker.G, RGBmarker.B, FL_BLACK, FL_RED);
//DW        makeMarker_( static_cast<int>(bandwidth / 2.0 + 1),
//DW                     &RGBmarker, txfreq,
//DW                     clrMin, clrMin + (int)((double)txfreq + 0.5), clrMax);
//DW    }
//DW 
//DW    if (!wantcursor) return;
//DW 
//DW    if (cursorpos > progdefaults.HighFreqCutoff - bandwidth / 2 / step)
//DW        cursorpos = progdefaults.HighFreqCutoff - bandwidth / 2 / step;
//DW    if (cursorpos >= (progdefaults.HighFreqCutoff - offset - bandwidth/2)/step)
//DW        cursorpos = (progdefaults.HighFreqCutoff - offset - bandwidth/2)/step;
//DW    if (cursorpos < (progdefaults.LowFreqCutoff + bandwidth / 2) / step)
//DW        cursorpos = (progdefaults.LowFreqCutoff + bandwidth / 2) / step;
//DW 
//DW // Create the cursor marker
//DW    double xp = offset + step * cursorpos;
//DW    if (xp < bandwidth / 2.0 || xp > (progdefaults.HighFreqCutoff - bandwidth / 2.0))
//DW        return;
//DW    clrM = markerimage + scale_width + (int)(xp + 0.5);
//DW    RGBcursor.R = progdefaults.cursorLineRGBI.R;
//DW    RGBcursor.G = progdefaults.cursorLineRGBI.G;
//DW    RGBcursor.B = progdefaults.cursorLineRGBI.B;
//DW 
//DW    int bw_lo = marker_width;
//DW    int bw_hi = marker_width;
//DW    if (mode >= MODE_MT63_500S && mode <= MODE_MT63_2000L)
//DW        bw_hi = bw_hi * 31 / 32;
//DW    if (mode == MODE_FSQ || mode == MODE_IFKP) bw_hi = bw_hi * 32 / 33;
//DW 
//DW    for (int y = 0; y < WFMARKER - 2; y++) {
//DW        int incr = y * scale_width;
//DW        int msize = (WFMARKER - 2 - y)*RGBsize*step/4;
//DW        *(clrM + incr - 1)  =
//DW        *(clrM + incr)      =
//DW        *(clrM + incr + 1)  = RGBcursor;
//DW 
//DW        if (xp - (bw_lo + msize) > 0)
//DW            for (int i = bw_lo - msize; i <= bw_lo + msize; i++)
//DW                *(clrM - i + incr) = RGBcursor;
//DW 
//DW        if (xp + (bw_hi + msize) < scale_width)
//DW            for (int i = bw_hi - msize; i <= bw_hi + msize; i++)
//DW                *(clrM + i + incr) = RGBcursor;
//DW    }
}

void WFdisp::makeScale() {
    uchar *gmap = scaleimage;
    int hwidth = step / 2;
    memset(scline, 0, scale_width);

    for (int tic = 500; tic < scale_width; tic += 500) {
        if (hwidth)
            for (int ticn = -hwidth; ticn < hwidth; ticn++)
                scline[tic + ticn] = 255;
        else
            scline[tic] = 255;
    }
    for (int i = 0; i < WFSCALE - 5; i++) {
        memcpy(gmap, scline, scale_width);
        gmap += (scale_width);
    }

    for (int tic = 100; tic < scale_width ; tic += 100) {
        if (hwidth)
            for (int ticn = -hwidth; ticn < hwidth; ticn++)
                scline[tic + ticn] = 255;
        else
            scline[tic] = 255;
    }
    for (int i = 0; i < 5; i++) {
        memcpy(gmap, scline, scale_width);
        gmap += (scale_width);
    }
}

void WFdisp::setcolors() {
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

void WFdisp::initmaps() {
    memset (scaleimage, 0, scale_width * WFSCALE);
    memset (markerimage, 0, disp_width * WFMARKER);
    memset (sig_img, 0, sig_image_area);

    memset (mag2RGBI, 0, sizeof(mag2RGBI));
    initMarkers();
    makeScale();
    setcolors();
}

void WFdisp::process_analog (double *sigy_normalized,
                             unsigned int  first_x,
                             unsigned int  width)
{
    unsigned int h0, ynext;
    static unsigned int ylast;
    unsigned int sigx, sigpixel;
    int white, graylevel;

    h0 = h();
    white     = 255;
    graylevel = 255;    // Set to 100 to see data points brighter than lines.

    if(0 == first_x)
        ylast = h0;

    sigx = first_x;
    for(unsigned int i = 0; i < width; i++) {

        // Clear this column.
        for(unsigned int y = 0; y < h0; y++) {
            sig_img[sigx + y * disp_width] = 0;
        }

        assert(1.0 >= sigy_normalized[i]);
        ynext = static_cast<unsigned int>((h0 * (1.0 - sigy_normalized[i])));

        while(ylast < ynext) {
            sigpixel = sigx + ylast * disp_width;
            sig_img[sigpixel] = graylevel;
            ylast++;
        }

        while(ylast > ynext) {
            sigpixel = sigx + ylast * disp_width;
            sig_img[sigpixel] = graylevel;
            ylast--;
        }

        sigpixel = sigx + ynext * disp_width;
        sig_img[sigpixel] = white;
        sigx++;
    }
    redraw();
}

void WFdisp::redrawCursor()
{
    redraw();
//  cursormoved = true;
}

//DW extern state_t trx_state;

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// XXXXX This was a try at using queues. May be needed later.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// XXXXX static pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
// XXXXX struct scope_data_block_tag {
// XXXXX    unsigned char sig[SCOPE_DATA_ARRAY_SIZE];
// XXXXX    unsigned int  first_point_n;
// XXXXX    unsigned int  length;
// XXXXX };
// XXXXX queue<scope_data_block_tag> scope_data_blocks;
// XXXXX 
// XXXXX void WFdisp::sig_data (unsigned char *sig,
// XXXXX                        unsigned int  first_point_n,
// XXXXX                        unsigned int  length)
// XXXXX {
// XXXXX     struct scope_data_block_tag sdb;
// XXXXX 
// XXXXX     assert(length <= sizeof(sdb.sig));
// XXXXX 
// XXXXX     memcpy(sdb.sig, sig, length);
// XXXXX     sdb.first_point_n = first_point_n;
// XXXXX     sdb.length        = length;
// XXXXX 
// XXXXX    pthread_mutex_lock(&data_mutex);
// XXXXX     scope_data_blocks.push(sdb);
// XXXXX    pthread_mutex_unlock(&data_mutex);
// XXXXX 
// XXXXX    REQ(&WFdisp::handle_sig_data, this);
// XXXXX }
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// This function stretches the data in the y direction to fill a display wider
// in pixels than the number of data points we have, and this function
// normalizes the values from 0.0 to 1.0;
#define MAX_CHUNK_WIDTH 256
void WFdisp::sig_data(const unsigned char *sig,
                      unsigned int first_point_n,
                      unsigned int length)
{
    // This stays around because we often need last time's last value for this
    // time's first interpolation.
    static unsigned char scope_data_span[SCOPE_DATA_SPAN_SIZE];

    double sigy_normalized[MAX_CHUNK_WIDTH];
    static unsigned int first_sigx = 0;
    unsigned int  last_point_n;
    unsigned int  last_sigx;
    unsigned int  sigx;

    if(first_point_n == 0)
        first_sigx = 0;

    assert((first_point_n + length) <= (sizeof(scope_data_span) / sizeof(scope_data_span[0])));

    memcpy(scope_data_span + first_point_n, sig, length);

    last_point_n = first_point_n + length - 1;
    last_sigx    = last_point_n * (disp_width - 1) / (SCOPE_DATA_SPAN_SIZE - 1);

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
        nf = static_cast<double>(sigx) * (SCOPE_DATA_SPAN_SIZE - 1.0) / (disp_width - 1.0);
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

    process_analog(sigy_normalized, first_sigx, last_sigx - first_sigx + 1);

    Fl::unlock();
    Fl::awake();

    // Ready for next time.
    first_sigx = sigx;
}

//DW void WFdisp::handle_sig_data()
//DW {
//DW    guard_lock data_lock(&data_mutex);
//DW 
//DW    while (!audio_blocks.empty()) {
//DW 
//DW    // if sound card sampling rate changed reset the waterfall buffer
//DW    if (srate != audio_blocks.front().sr) {
//DW        srate = audio_blocks.front().sr;
//DW        memset(circbuff, 0, FFT_LEN * sizeof(*circbuff));
//DW        ptrCB = 0;
//DW    }
//DW 
//DW    memmove((void*)circbuff,
//DW            (void*)(circbuff + WFBLOCKSIZE),
//DW            (size_t)((FFT_LEN - WFBLOCKSIZE)*sizeof(wf_fft_type)));
//DW 
//DW    {
//DW        double gain = pow(10, progdefaults.wfRefLevel / -20.0);
//DW        int insertptr = FFT_LEN - WFBLOCKSIZE;
//DW        overload = false;
//DW        double overval, peak = 0.0;
//DW        for (int i = 0; i < WFBLOCKSIZE; i++) {
//DW            overval = fabs(audio_blocks.front().sig[i]);
//DW            circbuff[insertptr + i] = gain * audio_blocks.front().sig[i];
//DW            if (overval > peak) peak = overval;
//DW        }
//DW        peakaudio = 0.1 * peak + 0.9 * peakaudio;
//DW    }
//DW    audio_blocks.pop();
//DW 
//DW    if (mode == SCOPE)
//DW        process_analog(circbuff, FFT_LEN);
//DW    else
//DW        processFFT();
//DW 
//DW    put_WARNstatus(peakaudio);
//DW 
//DW    static char szFrequency[14];
//DW    if (active_modem && rfc != 0) { // use a boolean for the waterfall
//DW        int offset = 0;
//DW        double afreq = active_modem->get_txfreq();
//DW        trx_mode mode = active_modem->get_mode();
//DW        if (mode == MODE_RTTY && progdefaults.useMARKfreq) {
//DW            offset = (progdefaults.rtty_shift < rtty::numshifts ?
//DW                  rtty::SHIFT[progdefaults.rtty_shift] :
//DW                  progdefaults.rtty_custom_shift);
//DW            offset /= 2;
//DW            if (active_modem->get_reverse()) offset *= -1;
//DW        }
//DW        string testmode = qso_opMODE->value();
//DW        usb = !ModeIsLSB(testmode);
//DW        if (testmode.find("CW") != string::npos)
//DW            afreq = 0;//-progdefaults.CWsweetspot;
//DW        if (mode == MODE_ANALYSIS) {
//DW            dfreq = 0;
//DW        } else {
//DW            if (usb)
//DW                dfreq = rfc + afreq + offset;
//DW            else
//DW                dfreq = rfc - afreq - offset;
//DW        }
//DW        snprintf(szFrequency, sizeof(szFrequency), "%-.3f", dfreq / 1000.0);
//DW    } else {
//DW        dfreq = active_modem->get_txfreq();
//DW        snprintf(szFrequency, sizeof(szFrequency), "%-.0f", dfreq);
//DW    }
//DW    inpFreq->value(szFrequency);
//DW 
//DW    }
//DW }

// Check the display offset & limit to 0 to max IMAGE_WIDTH displayed
void WFdisp::checkoffset() {
//DW     if (mode == SCOPE) {
//DW         if (sigoffset < 0)
//DW             sigoffset = 0;
//DW         if (sigoffset > (disp_width - disp_width))
//DW             sigoffset = disp_width - disp_width;
//DW     } else {
//DW         if (offset > (int)(progdefaults.HighFreqCutoff - step * disp_width))
//DW             offset = (int)(progdefaults.HighFreqCutoff - step * disp_width);
//DW         if (offset < 0)
//DW             offset = 0;
//DW     }
}

void WFdisp::setOffset(int v) {
    offset = v;
    checkoffset();
}

void WFdisp::slew(int dir) {
    if (mode == SCOPE)
        sigoffset += dir;
    else
        offset += dir;
    checkoffset();
}

void WFdisp::movetocenter() {
    if (mode == SCOPE)
        sigoffset = disp_width / 2;
    else
        offset = carrierfreq - (disp_width * step / 2);
    checkoffset();
}

void WFdisp::carrier(int cf) {
    if (cf >= bandwidth / 2 && cf < (disp_width - bandwidth / 2)) {
        carrierfreq = cf;
        makeMarker();
        redrawCursor();
    }
}

int WFdisp::carrier() {
    return carrierfreq;
}

void WFdisp::checkWidth()
{
//DW     disp_width = w();
//DW     if (mag == MAG_1) step = 4;
//DW     if (mag == MAG_1 && disp_width > progdefaults.HighFreqCutoff/4)
//DW         disp_width = progdefaults.HighFreqCutoff/4;
//DW     if (mag == MAG_2) step = 2;
//DW     if (mag == MAG_2 && disp_width > progdefaults.HighFreqCutoff/2)
//DW         disp_width = progdefaults.HighFreqCutoff/2;
//DW     if (mag == MAG_4) step = 1;
}

int WFdisp::checkMag()
{
    checkWidth();
    makeScale();
    return mag;
}

int WFdisp::setMag(int m)
{
    int mid = offset + (disp_width * step / 2);
    mag = m;
    checkMag();
    if (centercarrier || Fl::event_shift()) {
        offset = mid - (disp_width * step / 2);
    }
    else {
        movetocenter();
    }
    return mag;
}

int WFdisp::wfmag() {
    int mid = offset + (disp_width * step / 2);
    if (mag == MAG_1) mag = MAG_2;
    else if (mag == MAG_2) mag = MAG_4;
    else mag = MAG_1;
    checkMag();
    if (centercarrier || Fl::event_shift()) {
        offset = mid - (disp_width * step / 2);
    }
    else {
        movetocenter();
    }
    return mag;
}


void WFdisp::drawScale() {
    int fw = 60, xoff;
    static char szFreq[20];
    double fr;
    uchar *pixmap;

    if (progdefaults.wf_audioscale)
        pixmap = (scaleimage + (int)offset);
    else if (usb || !rfc)
        pixmap = (scaleimage +  (int)((rfc % 1000 + offset)) );
    else
        pixmap = (scaleimage + (int)((1000 - rfc % 1000 + offset)));

    fl_draw_image_mono(
        pixmap,
        x(), y() + WFTEXT,
        w(), WFSCALE,
        step, scale_width);

    fl_color(fl_rgb_color(228));
    fl_font(progdefaults.WaterfallFontnbr, progdefaults.WaterfallFontsize);
    for (int i = 1; ; i++) {
        if (progdefaults.wf_audioscale)
            fr = 500.0 * i;
        else {
            int cwoffset = 0;
//DW            string testmode = qso_opMODE->value();
//DW            usb = !ModeIsLSB(testmode);
//DW            if (testmode.find("CW") != string::npos)
//DW                cwoffset = progdefaults.CWsweetspot;
            if (usb)
                fr = (rfc - (rfc%500))/1000.0 + 0.5*i - cwoffset/1000.0;
            else
                fr = (rfc - (rfc %500))/1000.0 + 0.5 - 0.5*i + cwoffset/1000.0;
        }
        if (progdefaults.wf_audioscale)
            snprintf(szFreq, sizeof(szFreq), "%7.0f", fr);
        else
            snprintf(szFreq, sizeof(szFreq), "%7.1f", fr);
        fw = (int)fl_width(szFreq);
        if (progdefaults.wf_audioscale)
            xoff = (int) (( (1000.0/step) * i - fw) / 2.0 - offset /step );
        else if (usb)
            xoff = (int) ( ( (1000.0/step) * i - fw) / 2.0 -
                            (offset + rfc % 500) /step );
        else
            xoff = (int) ( ( (1000.0/step) * i - fw) / 2.0 -
                            (offset + 500 - rfc % 500) /step );
        if (xoff > 0 && xoff < w() - fw)
            fl_draw(szFreq, x() + xoff, y() + 10 );
        if (xoff > w() - fw) break;
    }
}

void WFdisp::drawMarker() {
    if (mode == SCOPE) return;
    uchar *pixmap = (uchar *)(markerimage + (int)(offset));
    fl_draw_image(
        pixmap,
        x(), y() + WFSCALE + WFTEXT,
        w(), WFMARKER,
        step * RGBsize, RGBwidth);
}

void WFdisp::update_waterfall() {
// transfer the fft history data into the WF image
//DW    short int * __restrict__ p1, * __restrict__ p2;
//DW    RGBI * __restrict__ p3, * __restrict__ p4;
//DW    p1 = tmp_fft_db + offset + step/2;
//DW    p2 = p1;
//DW    p3 = fft_img;
//DW    p4 = p3;
//DW 
//DW    short*  __restrict__ limit = tmp_fft_db + image_area - step + 1;
//DW 
#define UPD_LOOP( Step, Operation ) \
case Step: for (int row = 0; row < image_height; row++) { \
        p2 = p1; \
        p4 = p3; \
        for ( const short *  __restrict__ last_p2 = std::min( p2 + Step * disp_width, limit +1 ); p2 < last_p2; p2 += Step ) { \
            *(p4++) = mag2RGBI[ Operation ]; \
        } \
        p1 += IMAGE_WIDTH; \
        p3 += disp_width; \
    }; break

//DW    if (progdefaults.WFaveraging) {
//DW        switch(step) {
//DW            UPD_LOOP( 4, (*p2 + *(p2+1) + *(p2+2) + *(p2-1) + *(p2-1))/5 );
//DW            UPD_LOOP( 2, (*p2 + *(p2+1) + *(p2-1))/3 );
//DW            UPD_LOOP( 1, *p2 );
//DW            default:;
//DW        }
//DW    } else {
//DW        switch(step) {
//DW            UPD_LOOP( 4, MAX( MAX( MAX ( MAX ( *p2, *(p2+1) ), *(p2+2) ), *(p2-2) ), *(p2-1) ) );
//DW            UPD_LOOP( 2, MAX( MAX( *p2, *(p2+1) ), *(p2-1) ) );
//DW            UPD_LOOP( 1, *p2 );
//DW            default:;
//DW        }
//DW    }
//DW #undef UPD_LOOP
//DW 
//DW    if (active_modem && progdefaults.UseBWTracks) {
//DW        int bw_lo = bandwidth / 2;
//DW        int bw_hi = bandwidth / 2;
//DW        trx_mode mode = active_modem->get_mode();
//DW        if (mode >= MODE_MT63_500S && mode <= MODE_MT63_2000L)
//DW            bw_hi = bw_hi * 31 / 32;
//DW        if (mode == MODE_FSQ || mode == MODE_IFKP) {
//DW            bw_hi = bw_lo = 69 * bandwidth / 100;
//DW        }
//DW        RGBI  *pos1 = fft_img + (carrierfreq - offset - bw_lo) / step;
//DW        RGBI  *pos2 = fft_img + (carrierfreq - offset + bw_hi) / step;
//DW        if (unlikely(pos2 == fft_img + disp_width))
//DW            pos2--;
//DW        if (likely(pos1 >= fft_img && pos2 < fft_img + disp_width)) {
//DW            RGBI rgbi1, rgbi2 ;
//DW 
//DW            if (mode == MODE_RTTY && progdefaults.useMARKfreq) {
//DW                if (active_modem->get_reverse()) {
//DW                    rgbi1 = progdefaults.rttymarkRGBI;
//DW                    rgbi2 = progdefaults.bwTrackRGBI;
//DW                } else {
//DW                    rgbi1 = progdefaults.bwTrackRGBI;
//DW                    rgbi2 = progdefaults.rttymarkRGBI;
//DW                }
//DW            } else {
//DW                rgbi1 = progdefaults.bwTrackRGBI;
//DW                rgbi2 = progdefaults.bwTrackRGBI;
//DW            }
//DW            if (progdefaults.UseWideTracks) {
//DW                for (int y = 0; y < image_height; y ++) {
//DW                    *(pos1 + 1) = *pos1 = rgbi1;
//DW                    *(pos2 - 1) = *pos2 = rgbi2;
//DW                    pos1 += disp_width;
//DW                    pos2 += disp_width;
//DW                }
//DW            } else {
//DW                for (int y = 0; y < image_height; y ++) {
//DW                    *pos1 = rgbi1;
//DW                    *pos2 = rgbi2;
//DW                    pos1 += disp_width;
//DW                    pos2 += disp_width;
//DW                }
//DW            }
//DW        }
//DW    }
//DW 
//DW // draw notch
//DW    if ((notch_frequency > 1) && (notch_frequency < progdefaults.HighFreqCutoff - 1)) {
//DW        RGBInotch.I = progdefaults.notchRGBI.I;
//DW        RGBInotch.R = progdefaults.notchRGBI.R;
//DW        RGBInotch.G = progdefaults.notchRGBI.G;
//DW        RGBInotch.B = progdefaults.notchRGBI.B;
//DW        RGBI  *notch = fft_img + (notch_frequency - offset) / step;
//DW        int dash = 0;
//DW        for (int y = 0; y < image_height; y++) {
//DW            dash = (dash + 1) % 6;
//DW            if (dash == 0 || dash == 1 || dash == 2)
//DW                *(notch-1) = *notch = *(notch+1) = RGBInotch;
//DW            notch += disp_width;
//DW        }
//DW    }
//DW 
//DW    if (progdefaults.show_psm_btn &&
//DW        progStatus.kpsql_enabled && 
//DW        (trx_state == STATE_RX))
//DW        signal_psm();
}

void WFdisp::drawcolorWF() {
    uchar *pixmap = (uchar *)fft_img;

    update_waterfall();

//DW    if (active_modem && wantcursor &&
//DW        (progdefaults.UseCursorLines || progdefaults.UseCursorCenterLine) ) {
//DW        trx_mode mode = active_modem->get_mode();
//DW        int bw_lo = bandwidth / 2;
//DW        int bw_hi = bandwidth / 2;
//DW        if (mode >= MODE_MT63_500S && mode <= MODE_MT63_2000L)
//DW            bw_hi = bw_hi * 31 / 32;
//DW        if (mode == MODE_FSQ || mode == MODE_IFKP) bw_hi = bw_hi * 32 / 33;
//DW        RGBI  *pos0 = (fft_img + cursorpos);
//DW        RGBI  *pos1 = (fft_img + cursorpos - bw_lo/step);
//DW        RGBI  *pos2 = (fft_img + cursorpos + bw_hi/step);
//DW        if (pos1 >= fft_img && pos2 < fft_img + disp_width)
//DW            for (int y = 0; y < image_height; y ++) {
//DW                if (progdefaults.UseCursorLines) {
//DW                    *pos1 = *pos2 = progdefaults.cursorLineRGBI;
//DW                    if (progdefaults.UseWideCursor)
//DW                        *(pos1 + 1) = *(pos2 - 1) = *pos1;
//DW                }
//DW                if (progdefaults.UseCursorCenterLine) {
//DW                    *pos0 = progdefaults.cursorCenterRGBI;
//DW                    if (progdefaults.UseWideCenter)
//DW                        *(pos0 - 1) = *(pos0 + 1) = *pos0;
//DW                }
//DW                pos0 += disp_width;
//DW                pos1 += disp_width;
//DW                pos2 += disp_width;
//DW            }
//DW    }

    fl_color(FL_BLACK);
    fl_rectf(x(), y(), w(), WFSCALE + WFMARKER + WFTEXT);
    fl_color(fl_rgb_color(palette[0].R, palette[0].G, palette[0].B));
    fl_rectf(x(), y() + WFSCALE + WFMARKER + WFTEXT, w(), image_height);
    fl_draw_image(
        pixmap, x(), y() + WFSCALE + WFMARKER + WFTEXT,
        disp_width, image_height,
        sizeof(RGBI), disp_width * sizeof(RGBI) );
    drawScale();
}

void WFdisp::drawspectrum() {
    int sig;
    long offset_idx = 0;
    long ynext,
        h1 = image_height - 1,
        ffty = 0,
        fftpixel = disp_width * h1,
        graylevel = 220;
    uchar *pixmap = (uchar *)fft_sig_img + offset / step;

    memset (fft_sig_img, 0, image_area);

    fftpixel /= step;
    for (int c = 0; c < disp_width; c += step) {
        sig = tmp_fft_db[c];
        if (step == 1)
            sig = tmp_fft_db[c];
        else if (step == 2)
            sig = MAX(tmp_fft_db[c], tmp_fft_db[c+1]);
        else
            sig = MAX( MAX ( MAX ( tmp_fft_db[c], tmp_fft_db[c+1] ), tmp_fft_db[c+2] ), tmp_fft_db[c+3]);
        ynext = h1 * sig / 256;
        offset_idx = (disp_width/step);
        while ((ffty < ynext)) {
            fft_sig_img[fftpixel -= offset_idx] = graylevel; 
            ffty++;
            if (fftpixel < offset_idx) {
                cout << "corrupt index 1\n";
                break;
            }
        }
        while ((ffty > ynext)) {
            fft_sig_img[fftpixel += offset_idx] = graylevel; 
            ffty--;
            if (fftpixel >= (image_area - 1)) {
                cout << "corrupt index 2\n";
                break;
            }
        }
        if (fftpixel >= 0 && fftpixel <= image_area)
            fft_sig_img[fftpixel++] = graylevel;
        else
            cout << "fft_sig_image index out of bounds: " << fftpixel << endl;
    }

//DW    if (progdefaults.UseBWTracks) {
//DW        uchar  *pos1 = pixmap + (carrierfreq - offset - bandwidth/2) / step;
//DW        uchar  *pos2 = pixmap + (carrierfreq - offset + bandwidth/2) / step;
//DW        if (pos1 >= pixmap &&
//DW            pos2 < pixmap + disp_width)
//DW            for (int y = 0; y < image_height; y ++) {
//DW                *pos1 = *pos2 = 255;
//DW                if (progdefaults.UseWideTracks) {
//DW                    *(pos1 + 1) = 255;
//DW                    *(pos2 - 1) = 255;
//DW                }
//DW                pos1 += IMAGE_WIDTH/step;
//DW                pos2 += IMAGE_WIDTH/step;
//DW            }
//DW    }
//DW    if (active_modem && wantcursor &&
//DW        (progdefaults.UseCursorLines || progdefaults.UseCursorCenterLine)) {
//DW        trx_mode mode = active_modem->get_mode();
//DW        int bw_lo = bandwidth / 2;
//DW        int bw_hi = bandwidth / 2;
//DW        if (mode >= MODE_MT63_500S && mode <= MODE_MT63_2000L)
//DW            bw_hi = bw_hi * 31 / 32;
//DW        if (mode == MODE_FSQ || mode == MODE_IFKP) bw_hi = bw_hi * 32 / 33;
//DW        uchar  *pos0 = pixmap + cursorpos;
//DW        uchar  *pos1 = (pixmap + cursorpos - bw_lo/step);
//DW        uchar  *pos2 = (pixmap + cursorpos + bw_hi/step);
//DW        for (int y = 0; y < h1; y ++) {
//DW            if (progdefaults.UseCursorLines) {
//DW                *pos1 = *pos2 = 255;
//DW                if (progdefaults.UseWideCursor)
//DW                    *(pos1 + 1) = *(pos2 - 1) = *pos1;
//DW            }
//DW            if (progdefaults.UseCursorCenterLine) {
//DW                *pos0 = 255;
//DW                if (progdefaults.UseWideCenter) *(pos0-1) = *(pos0+1) = *(pos0);
//DW            }
//DW            pos0 += IMAGE_WIDTH/step;
//DW            pos1 += IMAGE_WIDTH/step;
//DW            pos2 += IMAGE_WIDTH/step;
//DW        }
//DW    }

//DW // draw notch
//DW    if ((notch_frequency > 1) && (notch_frequency < progdefaults.HighFreqCutoff - 1)) {
//DW        uchar  *notch = pixmap + (notch_frequency - offset) / step;
//DW        int dash = 0;
//DW        for (int y = 0; y < image_height; y++) {
//DW            dash = (dash + 1) % 6;
//DW            if (dash == 0 || dash == 1 || dash == 2)
//DW                *(notch-1) = *notch = *(notch+1) = 255;
//DW            notch += IMAGE_WIDTH/step;
//DW        }
//DW    }

    fl_color(FL_BLACK);
    fl_rectf(x(), y(), w(), WFSCALE + WFMARKER + WFTEXT + image_height);

    fl_draw_image_mono(
        pixmap,
        x(), y() + WFSCALE + WFMARKER + WFTEXT,
        disp_width, image_height,
        1, disp_width / step);
    drawScale();
}

void WFdisp::drawsignal() {
    uchar *pixmap = (uchar *)(sig_img + sigoffset);

    fl_color(FL_BLACK);
    fl_rectf(x() + disp_width, y(), w() - disp_width, h());
    fl_draw_image_mono(pixmap, x(), y(), disp_width, h(), 1, disp_width);
}

void WFdisp::draw() {

    checkoffset();
    checkWidth();

//DW    if (progdefaults.show_psm_btn && progStatus.kpsql_enabled) {
//DW        drawcolorWF();
//DW        drawMarker();
//DW        return;
//DW    }

    switch (mode) {
    case SPECTRUM :
        drawspectrum();
        drawMarker();
        break;
    case SCOPE :
        drawsignal();
        break;
    case WATERFALL :
    default:
        drawcolorWF();
        drawMarker();
    }
}

//=======================================================================
// waterfall
//=======================================================================

void x1_cb(Fl_Widget *w, void* v) {
    waterfall *wf = (waterfall *)w->parent();
    int m = wf->wfdisp->wfmag();
    if (m == MAG_1) w->label("x1");
    if (m == MAG_2) w->label("x2");
    if (m == MAG_4) w->label("x4");
//DW    restoreFocus();
}

void slew_left(Fl_Widget *w, void * v) {
    waterfall *wf = (waterfall *)w->parent();
    wf->wfdisp->slew(-100);
//DW    restoreFocus();
}

void slew_right(Fl_Widget *w, void * v) {
    waterfall *wf = (waterfall *)w->parent();
    wf->wfdisp->slew(100);
//DW    restoreFocus();
}


void center_cb(Fl_Widget *w, void *v) {
    waterfall *wf = (waterfall *)w->parent();
    wf->wfdisp->movetocenter();
//DW    restoreFocus();
}

void carrier_cb(Fl_Widget *w, void *v) {
    Fl_Counter *cntr = (Fl_Counter *)w;
    waterfall *wf = (waterfall *)w->parent();
    int selfreq = (int) cntr->value();
//DW    if (selfreq > progdefaults.HighFreqCutoff) selfreq = progdefaults.HighFreqCutoff - wf->wfdisp->Bandwidth() / 2;
//DW    stopMacroTimer();
//DW    if (active_modem) active_modem->set_freq(selfreq);
    wf->wfdisp->carrier(selfreq);
//DW    restoreFocus();
}

void do_qsy(bool dir)
{
//DW    if (!active_modem) return;
//DW    static vector<qrg_mode_t> qsy_stack;
//DW    qrg_mode_t m;
//DW 
//DW    wf->xmtlock->value(0);
//DW    wf->xmtlock->do_callback();
//DW 
//DW    if (dir) {
//DW // store
//DW        m.rfcarrier = wf->rfcarrier();
//DW        int wfc = m.carrier = active_modem->get_freq();
//DW        qsy_stack.push_back(m);
//DW        m.rmode = qso_opMODE->value();
//DW        trx_mode md = active_modem->get_mode();
//DW 
//DW // qsy to the sweet spot frequency that is the center of the PBF in the rig
//DW        switch (md) {
//DW            case MODE_CW:
//DW                m.carrier = (long long)progdefaults.CWsweetspot;
//DW                break;
//DW            case MODE_RTTY:
//DW                m.carrier = (long long)progdefaults.RTTYsweetspot;
//DW                break;
//DW            default:
//DW                m.carrier = (long long)progdefaults.PSKsweetspot;
//DW                break;
//DW        }
//DW        if (m.rmode.find("CW") != string::npos) {
//DW            if (wf->USB())
//DW                m.rfcarrier += (wfc - m.carrier);
//DW            else
//DW                m.rfcarrier -= (wfc - m.carrier);
//DW        }
//DW        else if (wf->USB())
//DW            m.rfcarrier += (wf->carrier() - m.carrier);
//DW        else
//DW            m.rfcarrier -= (wf->carrier() - m.carrier);
//DW    }
//DW    else { // qsy to top of stack
//DW        if (qsy_stack.size()) {
//DW            m = qsy_stack.back();
//DW            qsy_stack.pop_back();
//DW        }
//DW    }
//DW 
//DW    if (m.carrier > 0)
//DW        qsy(m.rfcarrier, m.carrier);
}

void qsy_cb(Fl_Widget *w, void *v)
{
    if (Fl::event_button() == FL_LEFT_MOUSE) {
        //send_scope_on();
        send_scope_wave_output_on();
    } else if (Fl::event_button() == FL_RIGHT_MOUSE) {
        start_scope_waveform_thread();
    } else if (Fl::event_button() == FL_MIDDLE_MOUSE) {
        stop_scope_waveform_thread();
    }

//DW    if (Fl::event_button() != FL_RIGHT_MOUSE)
//DW        do_qsy(true);
//DW    else
//DW        do_qsy(false);
//DW    restoreFocus();
}

void rate_cb(Fl_Widget *w, void *v) {
    waterfall* wf = static_cast<waterfall*>(w->parent());
    WFspeed new_speed;

    switch (wf->wfdisp->Speed()) {
    case SLOW:
        new_speed = NORMAL;
        break;
    case NORMAL: default:
        new_speed = FAST;
        break;
    case FAST:
        new_speed = PAUSE;
        break;
    case PAUSE:
        new_speed = SLOW;
        break;
    }

    wf->Speed(new_speed);
//DW    restoreFocus();
}

//extern void reset_xmlchars();

void xmtrcv_cb(Fl_Widget *w, void *vi)
{
//DW    if (!active_modem) return;
//DW    FL_LOCK_D();
//DW    Fl_Light_Button *b = (Fl_Light_Button *)w;
//DW    int v = b->value();
//DW    FL_UNLOCK_D();
//DW    if (!(active_modem->get_cap() & modem::CAP_TX)) {
//DW        b->value(0);
//DW        restoreFocus();     return;
//DW    }
//DW    if (v == 1) {
//DW        stopMacroTimer();
//DW        active_modem->set_stopflag(false);
//DW 
//DW        if (progdefaults.show_psm_btn && progStatus.kpsql_enabled)
//DW            set_xmtrcv_selection_color_pending();
//DW        trx_transmit();
//DW    } else {
//DW        if (progdefaults.show_psm_btn && progStatus.kpsql_enabled) {
//DW            psm_transmit_ended(PSM_ABORT);
//DW            xmtrcv_selection_color(progdefaults.XmtColor);
//DW        }
//DW 
//DW        if (btnTune->value()) {
//DW            btnTune->value(0);
//DW            btnTune->do_callback();
//DW        }
//DW        else {
//DW            TransmitText->clear();
//DW            if (active_modem->get_mode() == MODE_FSQ && fsq_tx_text)
//DW                fsq_tx_text->clear();
//DW            else if (active_modem->get_mode() == MODE_IFKP && ifkp_tx_text)
//DW                ifkp_tx_text->clear();
//DW 
//DW            if (arq_text_available)
//DW                AbortARQ();
//DW 
//DW            if(xmltest_char_available)
//DW                reset_xmlchars();
//DW 
//DW            if(kiss_text_available)
//DW                flush_kiss_tx_buffer();
//DW 
//DW            if (progStatus.timer)
//DW                progStatus.timer = 0;
//DW 
//DW            queue_reset();
//DW            active_modem->set_stopflag(true);
//DW        }
//DW    }
//DW    restoreFocus();
}

void xmtlock_cb(Fl_Widget *w, void *vi)
{
//DW    if (!active_modem) return;
//DW    FL_LOCK_D();
//DW    Fl_Light_Button *b = (Fl_Light_Button *)w;
//DW    int v = b->value();
//DW    FL_UNLOCK_D();
//DW    active_modem->set_freqlock(v ? true : false );
//DW    restoreFocus();
}

void waterfall::set_XmtRcvBtn(bool val)
{
    FL_LOCK_D();
    xmtrcv->value(val);
//DW    if (!val && btnTune->value()) {
//DW        btnTune->value(0);
//DW        btnTune->labelcolor(FL_FOREGROUND_COLOR);
//DW    }
    FL_UNLOCK_D();
}

void set_wf_mode(Fl_Widget *w)
{
    static const char* names[NUM_WF_MODES] = { "WF", "FFT", "SIG" };
    waterfall *wf = (waterfall *)w->parent();
    int m = 0;

    m = wf->wfdisp->Mode() + (Fl::event_button() == FL_LEFT_MOUSE ? 1 : -1);
    m = WCLAMP(m, WATERFALL, NUM_WF_MODES-1);

    if (m == SCOPE)
        wf->x1->deactivate();
    else
        wf->x1->activate();

    wf->wfdisp->Mode(static_cast<WFmode>(m));
    wf->mode->label(names[m]);
//DW    restoreFocus();
}

void mode_cb(Fl_Widget* w, void*)
{
    set_wf_mode(w);
}

void reflevel_cb(Fl_Widget *w, void *v) {
    FL_LOCK_D();
    waterfall *wf = (waterfall *)w->parent();
    double val = wf->wfRefLevel->value();
    FL_UNLOCK_D();
    progdefaults.wfRefLevel = val;
//DW    restoreFocus();
}

void ampspan_cb(Fl_Widget *w, void *v) {
    FL_LOCK_D();
    waterfall *wf = (waterfall *)w->parent();
    double val = wf->wfAmpSpan->value();
    FL_UNLOCK_D();
    wf->wfdisp->Ampspan(val);
    progdefaults.wfAmpSpan = val;
//DW    restoreFocus();
}

void btnRev_cb(Fl_Widget *w, void *v)
{
//DW    if (!active_modem) return;
//DW    FL_LOCK_D();
//DW    waterfall *wf = (waterfall *)w->parent();
//DW    Fl_Light_Button *b = (Fl_Light_Button *)w;
//DW    wf->Reverse(b->value());
//DW    FL_UNLOCK_D();
//DW    active_modem->set_reverse(wf->Reverse());
//DW    progdefaults.rtty_reverse = b->value();
//DW    progdefaults.changed = true;
//DW    restoreFocus();
}

void btnMem_cb(Fl_Widget *, void *menu_event)
{
//DW    if (!active_modem) return;
//DW    static std::vector<qrg_mode_t> qrg_list;
//DW    enum { SELECT, APPEND, REPLACE, REMOVE, CLEAR };
//DW    int op = SELECT, elem = 0;
//DW 
//DW    if (menu_event) { // event on popup menu
//DW        elem = wf->mbtnMem->value();
//DW 
//DW        switch (Fl::event_button()) {
//DW            case FL_MIDDLE_MOUSE:
//DW                op = REPLACE;
//DW                break;
//DW            case FL_LEFT_MOUSE: case FL_RIGHT_MOUSE: default:
//DW                op = (Fl::event_state() & FL_SHIFT) ? REMOVE : SELECT;
//DW                break;
//DW        }
//DW    }
//DW    else { // button press
//DW        switch (Fl::event_button()) {
//DW            case FL_RIGHT_MOUSE:
//DW                return;
//DW            case FL_MIDDLE_MOUSE: // select last
//DW                if ((elem = qrg_list.size() - 1) < 0)
//DW                    return;
//DW                op = SELECT;
//DW                break;
//DW            case FL_LEFT_MOUSE: default:
//DW                op = (Fl::event_state() & FL_SHIFT) ? CLEAR : APPEND;
//DW                break;
//DW            }
//DW    }
//DW 
//DW    qrg_mode_t m;
//DW    switch (op) {
//DW        case SELECT:
//DW            m = qrg_list[elem];
//DW            if (active_modem != *mode_info[m.mode].modem)
//DW                init_modem_sync(m.mode);
//DW            if (m.rfcarrier && m.rfcarrier != wf->rfcarrier())
//DW                qsy(m.rfcarrier, m.carrier);
//DW            else
//DW                active_modem->set_freq(m.carrier);
//DW            break;
//DW        case REMOVE:
//DW            wf->mbtnMem->remove(elem);
//DW            qrg_list.erase(qrg_list.begin() + elem);
//DW            break;
//DW        case CLEAR:
//DW            wf->mbtnMem->clear();
//DW            qrg_list.clear();
//DW            break;
//DW        case APPEND: case REPLACE:
//DW            m.rfcarrier = wf->rfcarrier();
//DW            m.carrier = active_modem->get_freq();
//DW            m.mode = active_modem->get_mode();
//DW            if (op == APPEND) {
//DW                if (find(qrg_list.begin(), qrg_list.end(), m) == qrg_list.end())
//DW                    qrg_list.push_back(m);
//DW            else
//DW                break;
//DW            }
//DW            else
//DW                qrg_list[elem] = m;
//DW // write the menu item text
//DW            {
//DW                ostringstream o;
//DW                o << mode_info[m.mode].sname << " @@ ";
//DW                if (m.rfcarrier > 0) { // write 1000s separators
//DW                    char s[20], *p = s + sizeof(s) - 1;
//DW                    int i = 0;
//DW 
//DW                    *p = '\0';
//DW                    do {
//DW                        if (i % 3 == 0 && i)
//DW                            *--p = '.';
//DW                        *--p = '0' + m.rfcarrier % 10;
//DW                        ++i;
//DW                    } while ((m.rfcarrier /= 10) && p > s);
//DW 
//DW                    o << p << (wf->USB() ? " + " : " - ");
//DW                }
//DW                o << m.carrier;
//DW                if (op == APPEND)
//DW                    wf->mbtnMem->add(o.str().c_str());
//DW                else
//DW                    wf->mbtnMem->replace(elem, o.str().c_str());
//DW            }
//DW            break;
//DW    }
//DW 
//DW    restoreFocus();
}

void waterfall::opmode() {
//DW    if (!active_modem) return;
//DW    int val = (int)active_modem->get_bandwidth();
//DW 
//DW    wfdisp->carrier((int)CLAMP(
//DW        wfdisp->carrier(),
//DW        progdefaults.LowFreqCutoff + val / 2,
//DW        progdefaults.HighFreqCutoff - val / 2));
//DW 
//DW    wfdisp->Bandwidth( val );
//DW    FL_LOCK_D();
//DW    wfcarrier->range(progdefaults.LowFreqCutoff + val/2, progdefaults.HighFreqCutoff - val/2);
//DW    FL_UNLOCK_D();
}

void waterfall::carrier(int f) {
    wfdisp->carrier(f);
    FL_LOCK_D();
    wfcarrier->value(f);
    wfcarrier->damage(FL_DAMAGE_ALL);
    FL_UNLOCK_D();
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

int waterfall::Mag() {
    return wfdisp->Mag();
}

void waterfall::Mag(int m) {
    FL_LOCK_D();
    wfdisp->Mag(m);
    if (m == MAG_1) x1->label("x1");
    if (m == MAG_2) x1->label("x2");
    if (m == MAG_4) x1->label("x4");
    x1->redraw_label();
    FL_UNLOCK_D();
}

int waterfall::Offset() {
    return wfdisp->Offset();
}

void waterfall::Offset(int v) {
    FL_LOCK_D();
    wfdisp->Offset(v);
    FL_UNLOCK_D();
}

int waterfall::Carrier()
{
    return wfdisp->carrier();
}

void waterfall::Carrier(int f)
{
//DW    if (active_modem) active_modem->set_freq(f);
}

void waterfall::rfcarrier(long long cf) {
    wfdisp->rfcarrier(cf);
}

long long waterfall::rfcarrier() {
    return wfdisp->rfcarrier();
}

void waterfall::setRefLevel() {
    FL_LOCK_D();
    wfRefLevel->value(progdefaults.wfRefLevel);
    FL_UNLOCK_D();
}

void waterfall::setAmpSpan() {
    FL_LOCK_D();
    wfAmpSpan->value(progdefaults.wfAmpSpan);
    wfdisp->Ampspan(progdefaults.wfAmpSpan);
    FL_UNLOCK_D();
}

void waterfall::USB(bool b) {
//DW    if (wfdisp->USB() == b)
//DW        return;
//DW    wfdisp->USB(b);
//DW    if (active_modem) active_modem->set_reverse(reverse);
//DW    REQ(&viewer_redraw);
}

bool waterfall::USB() {
    return wfdisp->USB();
}

void waterfall::show_scope(bool on)
{
    if (on) {
        wfscope->show();
//DW        wfscope->position(wf->x() + wf->w() - wf_dim - BEZEL, wf->y());
//DW        wfdisp->size( wf->w() - 2 * BEZEL - wf_dim, wf_dim - 2 * BEZEL);
        rs1->init_sizes();
    } else {
        wfscope->hide();
//DW        wfscope->position(wf->x() + wf->w(), wf->y());
//DW        wfdisp->size( wf->w() - 2 * BEZEL, wf_dim - 2 * BEZEL);
        rs1->init_sizes();
    }
    wfscope->redraw();
}

waterfall::waterfall(int x0, int y0, int w0, int h0, char *lbl) :
    Fl_Group(x0,y0,w0,h0,lbl) {
    int xpos;
    float ratio;
    ratio = w0 * 1.0 / bwdths;

    wf_dim = h() - BTN_HEIGHT - 4;

    buttonrow = h() + y() - BTN_HEIGHT - 1;

    rs1 = new Fl_Group(x(), y(), w(), wf_dim);
        rs1->box(FL_DOWN_BOX);
        wfdisp = new WFdisp(
            x() + BEZEL,
            y() + BEZEL,
            w() - 2 * BEZEL,
            wf_dim - 2 * BEZEL);
//DW        wfscope = new Digiscope (x() + w(), y(), wf_dim, wf_dim);
        rs1->resizable(wfdisp);
    rs1->end();
//DW    wfscope->hide();

    xpos = x() + wSpace;

    mode = new Fl_Button(xpos, buttonrow, (int)(bwFFT*ratio), BTN_HEIGHT, "SIG");
    mode->callback(mode_cb, 0);
    mode->tooltip("Waterfall / FFT / Scope");

    xpos = xpos + (int)(bwFFT*ratio) + wSpace;
    wfRefLevel = new Fl_Counter2(xpos, buttonrow, (int)(cwRef*ratio), BTN_HEIGHT );
    wfRefLevel->callback(reflevel_cb, 0);
    wfRefLevel->step(1.0);
    wfRefLevel->precision(0);
    wfRefLevel->range(-40.0, 0.0);
    wfRefLevel->value(-20.0);
    wfRefLevel->tooltip("Upper signal level (dB)");
    wfRefLevel->type(FL_SIMPLE_COUNTER);

    xpos = xpos + (int)(cwRef*ratio) + wSpace;
    wfAmpSpan = new Fl_Counter2(xpos, buttonrow, (int)(cwRef*ratio), BTN_HEIGHT );
    wfAmpSpan->callback(ampspan_cb, 0);
    wfAmpSpan->step(1.0);
    wfAmpSpan->precision(0);
    wfAmpSpan->range(6.0, 90.0);
    wfAmpSpan->value(70.0);
    wfdisp->Ampspan(70.0);
    wfAmpSpan->tooltip("Signal range (dB)");
    wfAmpSpan->type(FL_SIMPLE_COUNTER);

    xpos = xpos + (int)(cwRef*ratio) + wSpace;
    x1 = new Fl_Button(xpos, buttonrow, (int)(bwX1*ratio), BTN_HEIGHT, "x1");
    x1->callback(x1_cb, 0);
    x1->tooltip("Change waterfall scale");

    xpos = xpos + (int)(bwX1*ratio) + wSpace;
    left = new Fl_Repeat_Button(xpos, buttonrow, (int)(bwMov*ratio), BTN_HEIGHT, "@<");
    left->callback(slew_left, 0);
    left->tooltip("Slew display lower in frequency");

    xpos = xpos + (int)(bwMov*ratio);
    center = new Fl_Button(xpos, buttonrow, (int)(bwMov*ratio), BTN_HEIGHT, "@||");
    center->callback(center_cb, 0);
    center->tooltip("Center display on signal");

    xpos = xpos + (int)(bwMov*ratio);
    right = new Fl_Repeat_Button(xpos, buttonrow, (int)(bwMov*ratio), BTN_HEIGHT, "@>");
    right->callback(slew_right, 0);
    right->tooltip("Slew display higher in frequency");

    xpos = xpos + (int)(bwMov*ratio) + wSpace;
    wfrate = new Fl_Button(xpos, buttonrow, (int)(bwRate*ratio), BTN_HEIGHT, "Norm");
    wfrate->callback(rate_cb, 0);
    wfrate->tooltip("Waterfall drop speed");

    xpos = xpos + (int)(bwRate*ratio) + wSpace;
    wfcarrier = new Fl_Counter2(xpos, buttonrow, (int)(cwCnt*ratio), BTN_HEIGHT );
    wfcarrier->callback(carrier_cb, 0);
    wfcarrier->step(1.0);
    wfcarrier->lstep(10.0);
    wfcarrier->precision(0);
    wfcarrier->range(16.0, progdefaults.HighFreqCutoff - 16.0);
    wfcarrier->value(wfdisp->carrier());
    wfcarrier->tooltip("Adjust cursor frequency");

    xpos = xpos + (int)(cwCnt*ratio) + wSpace;
    qsy = new Fl_Button(xpos, buttonrow, (int)(bwQsy*ratio), BTN_HEIGHT, "QSY");
    qsy->callback(qsy_cb, 0);
    qsy->tooltip("Center in passband\nRight click to undo");
//DW    qsy->deactivate();

    xpos = xpos + (int)(bwQsy*ratio) + wSpace;
    btnMem = new Fl_Button(xpos, buttonrow, (int)(bwMem*ratio), BTN_HEIGHT, "Store");
    btnMem->callback(btnMem_cb, 0);
    btnMem->tooltip("Store mode and frequency\nRight click for list");
    mbtnMem = new Fl_Menu_Button(btnMem->x(), btnMem->y(), btnMem->w(), btnMem->h(), 0);
    mbtnMem->callback(btnMem->callback(), mbtnMem);
    mbtnMem->type(Fl_Menu_Button::POPUP3);

    xpos = xpos + (int)(bwMem*ratio) + wSpace;
    xmtlock = new Fl_Light_Button(xpos, buttonrow, (int)(bwXmtLock*ratio), BTN_HEIGHT, "Lk");
    xmtlock->callback(xmtlock_cb, 0);
    xmtlock->value(0);
    xmtlock->selection_color(progdefaults.LkColor);
    xmtlock->tooltip("Lock transmit frequency");

    /// We save this flag which is used by rtty decoding.
    xpos = xpos + (int)(bwXmtLock*ratio) + wSpace;
    btnRev = new Fl_Light_Button(xpos, buttonrow, (int)(bwRev*ratio), BTN_HEIGHT, "Rv");
    btnRev->callback(btnRev_cb, 0);
    reverse = progdefaults.rtty_reverse;
    btnRev->value(reverse);
    btnRev->selection_color(progdefaults.RevColor);
    btnRev->tooltip("Reverse");

    xpos = w() - (int)(bwXmtRcv*ratio) - wSpace;
    xmtrcv = new Fl_Light_Button(xpos, buttonrow, (int)(bwXmtRcv*ratio) - BEZEL, BTN_HEIGHT, "T/R");
    xmtrcv->callback(xmtrcv_cb, 0);
    xmtrcv->selection_color(progdefaults.XmtColor);
    xmtrcv->value(0);
    xmtrcv->tooltip("Transmit/Receive");
    end();

    if(wf) {
        puts("waterfall::waterfall: error more than one instance created.");
        exit(1);
    }

    wf = this;
}

void waterfall::UI_select(bool on) {
    if (on) {
        if (!progdefaults.WF_UIrev)
            btnRev->hide(); else btnRev->show();
        if (!progdefaults.WF_UIwfcarrier)
            wfcarrier->hide(); else wfcarrier->show();
        if (!progdefaults.WF_UIwfreflevel)
            wfRefLevel->hide(); else wfRefLevel->show();
        if (!progdefaults.WF_UIwfampspan)
            wfAmpSpan->hide(); else wfAmpSpan->show();
        if (!progdefaults.WF_UIxmtlock)
            xmtlock->hide(); else xmtlock->show();
        if (!progdefaults.WF_UIqsy)
            qsy->hide(); else qsy->show();
        if (!progdefaults.WF_UIwfmode)
            mode->hide(); else mode->show();
        if (!progdefaults.WF_UIx1)
            x1->hide(); else x1->show();
        if (!progdefaults.WF_UIwfshift) {
            left->hide();
            center->hide();
            right->hide();
        } else {
            left->show();
            center->show();
            right->show();
        }
        if (!progdefaults.WF_UIwfdrop)
            wfrate->hide(); else wfrate->show();
        if (!progdefaults.WF_UIwfstore) {
            btnMem->hide();
            mbtnMem->hide();
        } else {
            btnMem->show();
            mbtnMem->show();
        }
//if (noUI) xmtrcv->hide();
    } else {
        btnRev->show();
        wfcarrier->show();
        wfRefLevel->show();
        wfAmpSpan->show();
        xmtlock->show();
        qsy->show();
        mode->show();
        x1->show();
        left->show();
        center->show();
        right->show();
        wfrate->show();
        btnMem->show();
        mbtnMem->show();
    }
    btnRev->redraw();
    wfcarrier->redraw();
    wfRefLevel->redraw();
    wfAmpSpan->redraw();
    xmtlock->redraw();
    qsy->redraw();
    mode->redraw();
    x1->redraw();
    left->redraw();
    center->redraw();
    right->redraw();
    wfrate->redraw();
    btnMem->redraw();
    mbtnMem->redraw();
}

int waterfall::handle(int event)
{
    //printf("waterfall::handle: event %s (%d)\n", fl_eventnames[event], event);

    if (event != FL_MOUSEWHEEL || Fl::event_inside(wfdisp))
        return Fl_Group::handle(event);

    int d;
    if ( !((d = Fl::event_dy()) || (d = Fl::event_dx())) )
        return 1;

//DW    // this does not belong here, but we don't have access to this widget's
//DW    // handle method (or its parent's)
//DW    if (active_modem && Fl::event_inside(MODEstatus)) {
//DW        trx_mode mode = active_modem->get_mode();
//DW        for (;;) {
//DW            mode = WCLAMP(mode + d, 0, NUM_MODES - 1);
//DW            if ((mode >= NUM_RXTX_MODES && mode < NUM_MODES) ||
//DW                progdefaults.visible_modes.test(mode))
//DW                break;
//DW        }
//DW        init_modem(mode);
//DW        return 1;
//DW    }
//DW    // as above; handle wheel events for the macro bar
//DW    extern void altmacro_cb(Fl_Widget *w, void *v);
//DW    if (progdefaults.macro_wheel) {
//DW        if (progdefaults.mbar_scheme > MACRO_SINGLE_BAR_MAX) {
//DW            if (Fl::event_inside(macroFrame2)) {
//DW                altmacro_cb(btnAltMacros2, reinterpret_cast<void *>(d));
//DW                return 1;
//DW            }
//DW        } else {
//DW            if (Fl::event_inside(macroFrame1)) {
//DW                altmacro_cb(btnAltMacros1, reinterpret_cast<void *>(d));
//DW                return 1;
//DW            }
//DW        }
//DW    }
//DW 
    return Fl_Group::handle(event);
}

static Fl_Cursor cursor = FL_CURSOR_DEFAULT;

static void hide_cursor(void *w)
{
    if (cursor != FL_CURSOR_NONE)
        reinterpret_cast<Fl_Widget *>(w)->window()->cursor(cursor = FL_CURSOR_NONE);
}

void waterfall::insert_text(bool check)
{
//DW    if (active_modem && check) {
//DW        qrg_mode_t m;
//DW        m.rfcarrier = wf->rfcarrier();
//DW        m.carrier = active_modem->get_freq();
//DW        m.mode = active_modem->get_mode();
//DW        extern qrg_mode_t last_marked_qrg;
//DW        if (last_marked_qrg.mode == m.mode && last_marked_qrg.rfcarrier == m.rfcarrier &&
//DW            abs(last_marked_qrg.carrier - m.carrier) <= 16)
//DW            return;
//DW        last_marked_qrg = m;
//DW    }
//DW 
//DW    string::size_type i;
//DW    if ((i = progdefaults.WaterfallClickText.find("<FREQ>")) != string::npos) {
//DW        string s = progdefaults.WaterfallClickText;
//DW        s[i] = '\0';
//DW        ReceiveText->addstr(s);
//DW        note_qrg(false);
//DW //     ReceiveText->addstr(s);
//DW //     ReceiveText->addstr(s.c_str() + i + strlen("<FREQ>"));
//DW    }
//DW    else
//DW        ReceiveText->addstr(progdefaults.WaterfallClickText, FTextView::SKIP);
}

static void find_signal_text(void)
{
//DW    if (!active_modem) return;
//DW    int freq = active_modem->get_freq();
//DW    trx_mode mode = active_modem->get_mode();
//DW 
//DW    extern map<string, qrg_mode_t> qrg_marks;
//DW    map<string, qrg_mode_t>::const_iterator i;
//DW    for (i = qrg_marks.begin(); i != qrg_marks.end(); ++i)
//DW        if (i->second.mode == mode && abs(i->second.carrier - freq) <= 20)
//DW            break;
//DW    if (i != qrg_marks.end()) {
//DW        // Search backward from the current text cursor position, then
//DW        // try the other direction
//DW        int pos = ReceiveText->insert_position();
//DW        if (ReceiveText->buffer()->search_backward(pos, i->first.c_str(), &pos, 1) ||
//DW            ReceiveText->buffer()->search_forward(pos, i->first.c_str(), &pos, 1)) {
//DW            ReceiveText->insert_position(pos);
//DW            ReceiveText->show_insert_position();
//DW        }
//DW    }
}

int WFdisp::handle(int event)
{
    //printf("WFdisp::handle: event %s (%d)\n", fl_eventnames[event], event);
//DW    static int pxpos, push;
    if (!(event == FL_LEAVE || Fl::event_inside(this))) {
//DW        if (event == FL_RELEASE)
//DW            push = 0;
        return 0;
    }

//DW    if (trx_state != STATE_RX)
//DW        return 1;
    int xpos = Fl::event_x() - x();
//DW    int ypos = Fl::event_y() - y();
    int eb;

    switch (event) {
    case FL_MOVE:
//DW        if (progdefaults.WaterfallQSY && ypos < WFTEXT + WFSCALE) {
//DW            Fl::remove_timeout(hide_cursor, this);
//DW            if (cursor != FL_CURSOR_WE)
//DW                window()->cursor(cursor = FL_CURSOR_WE);
//DW            if (wantcursor) {
//DW                wantcursor = false;
//DW                makeMarker();
//DW            }
//DW            break;
//DW        }
        if (cursor != FL_CURSOR_DEFAULT)
            window()->cursor(cursor = FL_CURSOR_DEFAULT);
        if (!Fl::has_timeout(hide_cursor, this))
            Fl::add_timeout(1, hide_cursor, this);
        wantcursor = true;
        cursorpos = xpos;
        makeMarker();
        redrawCursor();
        break;
    case FL_DRAG: case FL_PUSH:
//DW        stopMacroTimer();

        switch (eb = Fl::event_button()) {
        case FL_RIGHT_MOUSE:
            wantcursor = false;
            if (event == FL_PUSH) {
                tmp_carrier = true;
                oldcarrier = carrier();
//DW                if (progdefaults.WaterfallHistoryDefault)
//DW                    bHistory = true;
            }
            goto lrclick;
        case FL_LEFT_MOUSE:
            if ((Fl::event_state() & (FL_ALT | FL_CTRL)) == (FL_ALT | FL_CTRL)) {
//DW                if (notch_frequency)
//DW                    notch_off();
//DW                else
//DW                    notch_on(cursorFreq(xpos));
                return 1;
            }
            if (event == FL_PUSH) {
//DW                push = ypos;
//DW                pxpos = xpos;
                if (Fl::event_clicks())
                    return 1;
            }
//DW            if (progdefaults.WaterfallQSY && push < WFTEXT + WFSCALE) {
//DW                long long newrfc = (pxpos - xpos) * step;
//DW                if (!USB())
//DW                    newrfc = -newrfc;
//DW                newrfc += rfcarrier();
//DW                qsy(newrfc, active_modem ? active_modem->get_freq() : 1500);
//DW                pxpos = xpos;
//DW                return 1;
//DW            }
        lrclick:
            if (Fl::event_state() & FL_CTRL) {
                if (event == FL_DRAG)
                    break;
//DW                if (!progdefaults.WaterfallHistoryDefault)
//DW                    bHistory = true;
//DW                if (eb == FL_LEFT_MOUSE) {
//DW                       restoreFocus();
//DW                       break;
//DW                }
            }
//DW            if (progdefaults.WaterfallHistoryDefault)
//DW                bHistory = true;
            newcarrier = cursorFreq(xpos);
//DW            if (active_modem) {
//DW                newcarrier = (int)CLAMP(
//DW                    newcarrier,
//DW                    progdefaults.LowFreqCutoff + active_modem->get_bandwidth() / 2,
//DW                    progdefaults.HighFreqCutoff - active_modem->get_bandwidth() / 2);
//DW                active_modem->set_freq(newcarrier);
//DW                viewer_paste_freq(newcarrier);
//DW                if (!(Fl::event_state() & FL_SHIFT))
//DW                    active_modem->set_sigsearch(SIGSEARCH);
//DW            }
            redrawCursor();
//DW            restoreFocus();
            break;
        case FL_MIDDLE_MOUSE:
            if (event == FL_DRAG)
                break;
//          if (Fl::event_state() & FL_CTRL)
//              viewer_paste_freq(cursorFreq(xpos));
//          else {
//DW                btnAFC->value(!btnAFC->value());
//DW                btnAFC->do_callback();
//          }
        }
        break;
    case FL_RELEASE:
//DW        switch (eb = Fl::event_button()) {
//DW        case FL_RIGHT_MOUSE:
//DW            tmp_carrier = false;
//DW            if (active_modem) active_modem->set_freq(oldcarrier);
//DW            redrawCursor();
//DW            restoreFocus();
//DW            // fall through
//DW        case FL_LEFT_MOUSE:
//DW            push = 0;
//DW            oldcarrier = newcarrier;
//DW            if (eb != FL_LEFT_MOUSE || !ReceiveText->visible())
//DW                break;
//DW            if (eb == FL_LEFT_MOUSE)
//DW                recenter_spectrum_viewer();
//DW            if (!(Fl::event_state() & (FL_CTRL | FL_META | FL_ALT | FL_SHIFT))) {
//DW                if (Fl::event_clicks() == 1)
//DW                    note_qrg(true, "\n", "\n");
//DW                else
//DW                    if (progdefaults.WaterfallClickInsert)
//DW                        wf->insert_text(true);
//DW            }
//DW            else if (Fl::event_state() & (FL_META | FL_ALT))
                find_signal_text();
//DW            break;
//DW        }
        break;

//DW    case FL_MOUSEWHEEL:
//DW    {
//DW        stopMacroTimer();
//DW 
//DW        int d;
//DW        if ( !((d = Fl::event_dy()) || (d = Fl::event_dx())) )
//DW            break;
//DW        int state = Fl::event_state();
//DW        if (state & FL_CTRL)
//DW            wf->handle_mouse_wheel(waterfall::WF_AFC_BW, d);
//DW        else if (state & (FL_META | FL_ALT))
//DW            wf->handle_mouse_wheel(waterfall::WF_SIGNAL_SEARCH, d);
//DW        else if (state & FL_SHIFT)
//DW            wf->handle_mouse_wheel(waterfall::WF_SQUELCH, d);
//DW        else {
//DW            if (progdefaults.WaterfallQSY && Fl::event_inside(x(), y(), w(), WFTEXT+WFSCALE+WFMARKER))
//DW                qsy(wf->rfcarrier() - 500*d);
//DW            else
//DW                wf->handle_mouse_wheel(progdefaults.WaterfallWheelAction, d);
//DW        }
//DW        return handle(FL_MOVE);
//DW    }
//DW    case FL_SHORTCUT:
//DW        if (Fl::event_inside(this))
//DW            take_focus();
//DW        break;
//DW    case FL_KEYBOARD:
//DW    {
//DW        stopMacroTimer();
//DW 
//DW        int d = (Fl::event_state() & FL_CTRL) ? 10 : 1;
//DW        int k = Fl::event_key();
//DW        switch (k) {
//DW        case FL_Left: case FL_Right:
//DW            if (k == FL_Left)
//DW                d = -d;
//DW            if (active_modem) {
//DW                oldcarrier = newcarrier = (int)CLAMP(
//DW                    carrier() + d,
//DW                    progdefaults.LowFreqCutoff + active_modem->get_bandwidth() / 2,
//DW                    progdefaults.HighFreqCutoff - active_modem->get_bandwidth() / 2);
//DW                active_modem->set_freq(newcarrier);
//DW            }
//DW            redrawCursor();
//DW            break;
//DW        case FL_Tab:
//DW            restoreFocus();
//DW            break;
//DW        default:
//DW            restoreFocus();
//DW            return TransmitText->handle(event);
//DW        }
//DW        break;
//DW    }
    case FL_KEYUP:
    {
//DW        if (Fl::event_inside(this)) {
//DW            int k = Fl::event_key();
//DW            if (k == FL_Shift_L || k == FL_Shift_R || k == FL_Control_L ||
//DW                k == FL_Control_R || k == FL_Meta_L || k == FL_Meta_R ||
//DW                k == FL_Alt_L || k == FL_Alt_R)
//DW                restoreFocus();
//DW        }
        break;
    }

    case FL_LEAVE:
        Fl::remove_timeout(hide_cursor, this);
//DW        if (cursor != FL_CURSOR_DEFAULT)
//DW            window()->cursor(cursor = FL_CURSOR_DEFAULT);
//DW        wantcursor = false;
//DW        makeMarker();
//DW        // restoreFocus();
        break;

    }

    return 1;
}

void waterfall::handle_mouse_wheel(int what, int d)
{
//DW    if (d == 0)
//DW        return;
//DW 
//DW    Fl_Valuator *val = 0;
//DW    const char* msg_fmt = 0, *msg_label = 0;
//DW 
//DW    switch (what) {
//DW    case WF_NOP:
//DW        return;
//DW    case WF_AFC_BW:
//DW    {
//DW        if (active_modem) {
//DW            trx_mode m = active_modem->get_mode();
//DW            if (m >= MODE_PSK_FIRST && m <= MODE_PSK_LAST) {
//DW                val = mailserver ? cntServerOffset : cntSearchRange;
//DW                msg_label = "Srch Rng";
//DW            }
//DW            else if (m >= MODE_HELL_FIRST && m <= MODE_HELL_LAST) {
//DW                val = sldrHellBW;
//DW                msg_label = "BW";
//DW            }
//DW            else if (m == MODE_CW) {
//DW                val = sldrCWbandwidth;
//DW                msg_label = "BW";
//DW            }
//DW            else
//DW                return;
//DW            msg_fmt = "%s: %2.0f Hz";
//DW        }
//DW        break;
//DW    }
//DW    case WF_SIGNAL_SEARCH:
//DW        if (d > 0) {
//DW            if (active_modem) active_modem->searchDown();
//DW        } else {
//DW            if (active_modem) active_modem->searchUp();
//DW        }
//DW        return;
//DW    case WF_SQUELCH:
//DW        val = sldrSquelch;
//DW        d = -d;
//DW        msg_fmt = "%s = %2.0f %%";
//DW        msg_label = "Squelch";
//DW        break;
//DW    case WF_CARRIER:
//DW        val = wfcarrier;
//DW        break;
//DW    case WF_MODEM:
//DW        init_modem(d > 0 ? MODE_NEXT : MODE_PREV);
//DW        return;
//DW    case WF_SCROLL:
//DW        (d > 0 ? right : left)->do_callback();
//DW        return;
//DW    }
//DW 
//DW    val->value(val->clamp(val->increment(val->value(), -d)));
//DW    bool changed_save = progdefaults.changed;
//DW    val->do_callback();
//DW    progdefaults.changed = changed_save;
//DW    if (val == cntServerOffset || val == cntSearchRange) {
//DW        if (active_modem) active_modem->set_sigsearch(SIGSEARCH);
//DW    } else if (val == sldrSquelch) { // sldrSquelch gives focus to TransmitText
//DW        take_focus();
//DW    }
//DW    if (msg_fmt) {
//DW        char msg[60];
//DW        snprintf(msg, sizeof(msg), msg_fmt, msg_label, val->value());
//DW        put_status(msg, 2.0);
//DW    }
}

const char* waterfall::wf_wheel_action[] = {
//DW    _("None"), _("AFC range or BW"),
//DW    _("Signal search"), _("Squelch level"),
//DW    _("Modem carrier"), _("Modem"), _("Scroll")
};
