#ifndef IC7300_H
#define IC7300_H

// From page 19-13 of IC-7300 FULL MANUAL. PARSE_FIXED_EDGE_FREQUENCY() will
// convert the BCD encoded frequency into an unsigned long of frequency in Hz.
#define BCD(x) (((x >> 4) * 10) + (x & 0x0F))
#define PARSE_FIXED_EDGE_FREQUENCY(x3, x4, x5, x6, x7) \
    (1ul*BCD(x3) + 100ul*BCD(x4) + 10000ul*BCD(x5) + 1000000ul*BCD(x6) + 100000000ul*BCD(x7))

// These are per the "Data format" section of the Full Manual.
const unsigned char PREAMBLE    = 0xFE;
const unsigned char XCVR_ADDR   = 0x94; // Transceiver's default address
const unsigned char CONT_ADDR   = 0xE0; // Controller's default address
const unsigned char OK_CODE     = 0xFB;
const unsigned char NG_CODE     = 0xFA;
const unsigned char END_MESSAGE = 0xFD;

// IC-7300 FULL MANUAL page 19-7 says of '27 00':
//      • Only when “Scope ON/OFF status”
//      (Command: 27 10) and “Scope data output”
//      (Command: 27 20) are set to “ON,” outputs
//      the waveform data to the controller.
// There is no '27 20' command that I can find. It seems the scope must be
// turned on at the radio or with '27 10 01' and then '27 11 01' must be sent
// to start the data stream.

const int SCOPE_DATA_ARRAY_SIZE      = 50;
const int SCOPE_DATA_ARRAY_SIZE_LAST = 25;
const int SCOPE_DATA_SPAN_SIZE       = 475;
const int SCOPE_DATA_MAX_VALUE       = 160;  // Per IC-7300_ENG_CD_0.pdf.
const int FAKE_SCOPE_DATA_ARRAY_SIZE = 792;
const int LAST_DIVISION_NUMBER       = 0x11;

#pragma pack(push, 1)
struct scope_waveform_data_tag {
    unsigned char fixed;
    unsigned char division_number_bcd;
    unsigned char division_number_bcd_max;
    union extended_info_or_data {
        struct extended_info_tag {
            unsigned char  center_or_fixed;
            unsigned char  info_03;
            unsigned char  info_04;
            unsigned char  info_05;
            unsigned char  info_06;
            unsigned char  info_07;
            unsigned char  info_08;
            unsigned char  info_09;
            unsigned char  info_10;
            unsigned char  info_11;
            unsigned char  info_12;
            unsigned char  out_of_range;
        } extended_info;
        unsigned char data[SCOPE_DATA_ARRAY_SIZE];
    } extended_info_or_data;
};
#pragma pack(pop)

int process_cmd_from_radio(const unsigned char *buf, int nread);

#endif // IC7300_H
