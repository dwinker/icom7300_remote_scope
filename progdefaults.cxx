#include "progdefaults.h"

struct progdefaults_tag progdefaults;

void init_progdefaults(void)
{
    progdefaults.bwTrackRGBI          = {255,   0,   0, 255};
    progdefaults.cursorCenterRGBI     = {255, 255, 255, 255};
    progdefaults.notchRGBI            = {255, 255, 255, 255};
    progdefaults.rttymarkRGBI         = {255, 120,   0, 255};
    progdefaults.changed              = false;
    progdefaults.QRZchanged           = false;
    progdefaults.CWsweetspot          = 1000;
    progdefaults.drop_speed           = 8;
    progdefaults.HELL_BW              = 245.0;
    progdefaults.HELL_BW_FH           = 245.0;
    progdefaults.HELL_BW_SH           = 30.0;
    progdefaults.HELL_BW_X5           = 1225.;
    progdefaults.HELL_BW_X9           = 2205.0;
    progdefaults.HELL_BW_FSK          = 180.0;
    progdefaults.HELL_BW_FSK105       = 100.0;
    progdefaults.HELL_BW_HELL80       = 450.0;
    progdefaults. HighFreqCutoff      = 3000;
    progdefaults.LkColor              = FL_GREEN;
    progdefaults.LowFreqCutoff        = 0;
    progdefaults.macro_wheel          = false;
    progdefaults.mbar_scheme          = 1;
    progdefaults.PSKsweetspot         = 1000;
    progdefaults.RevColor             = FL_GREEN;
    progdefaults.RTTY_BW              = 68.0;
    progdefaults.rtty_custom_shift    = 450;
    progdefaults.rtty_reverse         = false;
    progdefaults.rtty_shift           = 3;
    progdefaults.RTTYsweetspot        = 1000;
    progdefaults.SearchRange          = 50;
    progdefaults.ServerOffset         = 50;
    progdefaults.show_psm_btn         = false;
    progdefaults.UseBWTracks          = true;
    progdefaults.UseCursorCenterLine  = true;
    progdefaults.UseCursorLines       = true;
    progdefaults.useMARKfreq          = true;
    progdefaults.UseWideCenter        = false;
    progdefaults.UseWideCursor        = false;
    progdefaults.UseWideTracks        = false;
    progdefaults.WaterfallClickInsert = false;
//DW     progdefaults.WaterfallClickText   = "\n<FREQ>\n";
    progdefaults.WaterfallFontnbr     = FL_HELVETICA;
    progdefaults.WaterfallFontsize    = 12;
    progdefaults.wfAmpSpan            = 60.0;
    progdefaults.wf_audioscale        = true;
    progdefaults.WFaveraging          = false;
    progdefaults.wf_latency           = 8;
    progdefaults.wfPreFilter          = 1;
    progdefaults.wfRefLevel           = 0.0;
    progdefaults.WF_UIqsy             = false;
    progdefaults.WF_UIrev             = false;
    progdefaults.WF_UIwfampspan       = false;
    progdefaults.WF_UIwfcarrier       = false;
    progdefaults.WF_UIwfdrop          = false;
    progdefaults.WF_UIwfmode          = false;
    progdefaults.WF_UIwfreflevel      = false;
    progdefaults.WF_UIwfshift         = false;
    progdefaults.WF_UIwfstore         = false;
    progdefaults.WF_UIx1              = false;
    progdefaults.WF_UIxmtlock         = false;
    progdefaults.XmtColor             = FL_RED;
}
