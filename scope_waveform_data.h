#ifndef _SCOPE_WAVEFORM_DATA_H
#define _SCOPE_WAVEFORM_DATA_H

//#define SCOPE_DATA_ARRAY_SIZE       50
//#define SCOPE_DATA_ARRAY_SIZE_LAST  25
//#define SCOPE_DATA_SPAN_SIZE       475
//#define SCOPE_DATA_MAX_VALUE       160  // Per IC-7300_ENG_CD_0.pdf.
//#define FAKE_SCOPE_DATA_ARRAY_SIZE 792
//#define LAST_DIVISION_NUMBER      0x11

void start_scope_waveform_thread();
void stop_scope_waveform_thread();

#endif // _SCOPE_WAVEFORM_DATA_H
