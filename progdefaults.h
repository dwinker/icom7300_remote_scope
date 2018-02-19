#ifndef _PROGDEFAULTS_H
#define _PROGDEFAULTS_H

#include <FL/Enumerations.H>
#include "waterfall.h"

struct progdefaults_tag {
    RGBI        bwTrackRGBI;
    RGBI        cursorCenterRGBI;
    RGBI        notchRGBI;
    RGBI        rttymarkRGBI;
    bool        changed;
    bool        QRZchanged;
    double      CWsweetspot;
    int         drop_speed;
    double      HELL_BW;
    double      HELL_BW_FH;
    double      HELL_BW_SH;
    double      HELL_BW_X5;
    double      HELL_BW_X9;
    double      HELL_BW_FSK;
    double      HELL_BW_FSK105;
    double      HELL_BW_HELL80;
    int         HighFreqCutoff;
    Fl_Color    LkColor;
    int         LowFreqCutoff;
    bool        macro_wheel;
    int         mbar_scheme;
    double      PSKsweetspot;
    Fl_Color    RevColor;
    double      RTTY_BW;
    int         rtty_custom_shift;
    bool        rtty_reverse;
    int         rtty_shift;
    double      RTTYsweetspot;
    int         SearchRange;
    int         ServerOffset;
    bool        show_psm_btn;
    bool        UseBWTracks;
    bool        UseCursorCenterLine;
    bool        UseCursorLines;
    bool        useMARKfreq;
    bool        UseWideCenter;
    bool        UseWideCursor;
    bool        UseWideTracks;
    bool        WaterfallClickInsert;
//DW     std::string WaterfallClickText;
    Fl_Font     WaterfallFontnbr;
    int         WaterfallFontsize;
    double      wfAmpSpan;
    int         wfPreFilter;
    bool        wf_audioscale;
    bool        WFaveraging;
    int         wf_latency;
    double      wfRefLevel;
    bool        WF_UIqsy;
    bool        WF_UIrev;
    bool        WF_UIwfampspan;
    bool        WF_UIwfcarrier;
    bool        WF_UIwfdrop;
    bool        WF_UIwfmode;
    bool        WF_UIwfreflevel;
    bool        WF_UIwfshift;
    bool        WF_UIwfstore;
    bool        WF_UIx1;
    bool        WF_UIxmtlock;
    Fl_Color    XmtColor;
};

extern struct progdefaults_tag progdefaults;

void init_progdefaults(void);

#endif /* _PROGDEFAULTS_H */
