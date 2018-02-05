#ifndef _SCOPE_WAVEFORM_DATA_H
#define _SCOPE_WAVEFORM_DATA_H

#define SCOPE_DATA_ARRAY_SIZE       50
#define SCOPE_DATA_ARRAY_SIZE_LAST  25
#define SCOPE_DATA_SPAN_SIZE       475
#define SCOPE_DATA_MAX_VALUE       160  // Per IC-7300_ENG_CD_0.pdf.
#define FAKE_SCOPE_DATA_ARRAY_SIZE 792
#define LAST_DIVISION_NUMBER      0x11

#pragma pack(push, 1)
struct scope_info_tag {
    unsigned char fixed;
    unsigned char division_number;
    unsigned char division_number_max;
    union extended_info_or_data {
        struct extended_info_tag {
            /* TBD - Guess for now. */
            unsigned char  fixed_mode_flag;
            unsigned long  waveform_info_1;
            unsigned long  waveform_info_2;
            unsigned char  out_of_range_flag;
        } extended_info;
        unsigned char data[SCOPE_DATA_ARRAY_SIZE];
    } extended_info_or_data;
};
#pragma pack(pop)

void start_scope_waveform_thread();
void stop_scope_waveform_thread();

extern const struct scope_info_tag fake_scope_data[FAKE_SCOPE_DATA_ARRAY_SIZE];

#endif // _SCOPE_WAVEFORM_DATA_H
