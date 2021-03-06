#include <stdio.h>
#include <string.h>
#include <stdlib.h> // For exit();
#include <assert.h>
#include "ic7300.h"
#include "IC7300_waterfall.h"

//struct scope_waveform_data_tag x;

const unsigned char SCOPE_CMD = 0x27;

const unsigned char SCOPE_WAVEFORM_DATA_SUBCMD        = 0x00;
const unsigned char SCOPE_OFF_ON_SUBCMD               = 0x10;
const unsigned char SCOPE_WAVEFORM_DATA_OUTPUT_SUBCMD = 0x11;
const unsigned char SCOPE_MAIN_OR_SUB_SUBCMD          = 0x12;
const unsigned char SCOPE_SINGLE_OR_DUAL_SUBCMD       = 0x13;
const unsigned char SCOPE_CENTER_OR_FIXED_SUBCMD      = 0x14;
const unsigned char SCOPE_CENTER_SPAN_SUBCMD          = 0x15;
const unsigned char SCOPE_FIXED_EDGE_SUBCMD           = 0x16;
const unsigned char SCOPE_HOLD_SUBCMD                 = 0x17;
const unsigned char SCOPE_REF_LEVEL_SUBCMD            = 0x19;
const unsigned char SCOPE_SPEED_SUBCMD                = 0x1A;
const unsigned char SCOPE_CENTER_TX_INDICATOR_SUBCMD  = 0x1B;
const unsigned char SCOPE_CENTER_MODE_SUBCMD          = 0x1C;
const unsigned char SCOPE_VBW_SUBCMD                  = 0x1D;
const unsigned char SCOPE_FIXED_EDGE_FREQS_SUBCMD     = 0x1E;

static void ProcessScopeCommand(const unsigned char *buf, int length);
static void ProcessScopeDataSubcmd(const unsigned char *buf, int length);

int process_cmd_from_radio(const unsigned char *buf, int nread)
{
    // Spin past the 0xFEs.
    while(nread && PREAMBLE == *buf) {
        buf++;
        nread--;
    }

    // Controller address is going to be our address or 0. We really couldn't
    // be getting a message from anything other than the radio.
    if(nread && (CONT_ADDR == *buf || 0 == *buf)) {
        buf++;
        nread--;
    } else {
        printf("Bad controller address: %02X\n", *buf);
        return -1;
    }

    // Next will be the radio address.
    if(nread && XCVR_ADDR == *buf) {
        buf++;
        nread--;
    } else {
        printf("Bad transceiver address: %02X\n", *buf);
        return -1;
    }
    
    // Nothing to do if no more data.
    if(nread) {
        // Next byte is the command.
        --nread;
        switch(*buf++) {
            case SCOPE_CMD:
                ProcessScopeCommand(buf, nread);
                break;
            default:
                break;
        }
    }

    return 0;
}

static void ProcessScopeCommand(const unsigned char *buf, int length)
{
    // Nothing to do if no sub command.
    if(0 == length)
        return;

    --length;
    switch(*buf++) {
        case SCOPE_WAVEFORM_DATA_SUBCMD:
            ProcessScopeDataSubcmd(buf, length);
            break;
        case SCOPE_OFF_ON_SUBCMD:
            puts("SCOPE_OFF_ON_SUBCMD unhandled");
            break;
        case SCOPE_WAVEFORM_DATA_OUTPUT_SUBCMD:
            puts("SCOPE_WAVEFORM_DATA_OUTPUT_SUBCMD unhandled");
            break;
        case SCOPE_MAIN_OR_SUB_SUBCMD:
            puts("SCOPE_MAIN_OR_SUB_SUBCMD unhandled");
            break;
        case SCOPE_SINGLE_OR_DUAL_SUBCMD:
            puts("SCOPE_SINGLE_OR_DUAL_SUBCMD unhandled");
            break;
        case SCOPE_CENTER_OR_FIXED_SUBCMD:
            puts("SCOPE_CENTER_OR_FIXED_SUBCMD unhandled");
            break;
        case SCOPE_CENTER_SPAN_SUBCMD:
            puts("SCOPE_CENTER_SPAN_SUBCMD unhandled");
            break;
        case SCOPE_FIXED_EDGE_SUBCMD:
            puts("SCOPE_FIXED_EDGE_SUBCMD unhandled");
            break;
        case SCOPE_HOLD_SUBCMD:
            puts("SCOPE_HOLD_SUBCMD unhandled");
            break;
        case SCOPE_REF_LEVEL_SUBCMD:
            puts("SCOPE_REF_LEVEL_SUBCMD unhandled");
            break;
        case SCOPE_SPEED_SUBCMD:
            puts("SCOPE_SPEED_SUBCMD unhandled");
            break;
        case SCOPE_CENTER_TX_INDICATOR_SUBCMD:
            puts("SCOPE_CENTER_TX_INDICATOR_SUBCMD unhandled");
            break;
        case SCOPE_CENTER_MODE_SUBCMD:
            puts("SCOPE_CENTER_MODE_SUBCMD unhandled");
            break;
        case SCOPE_VBW_SUBCMD:
            puts("SCOPE_VBW_SUBCMD unhandled");
            break;
        case SCOPE_FIXED_EDGE_FREQS_SUBCMD:
            puts("SCOPE_FIXED_EDGE_FREQS_SUBCMD unhandled");
            break;
        default:
            printf("Unknown scope sub command: %02X\n", buf[-1]);
            break;
    }
}

static void ProcessScopeDataSubcmd(const unsigned char *buf, int length)
{
    const struct  scope_waveform_data_tag *swd;
    unsigned char division_number;
    unsigned int  first_point_n;
    int           n_points;

    // Magic number, but a short message shorter than this is pretty useless.
    // If it's a division 1 there isn't enough there to decode the
    // extended_info. If it's a data message there may be a few bytes of data,
    // but not much. Seems reasonable to drop it.
    if(16 > length)
        return;

    swd = reinterpret_cast<const struct scope_waveform_data_tag *>(buf);

    division_number = BCD(swd->division_number_bcd);

    // Nothing to do if we don't understand division number.
    if(11 < division_number)
        return;

    first_point_n = 2 <= division_number ? 50 * (division_number - 2) : 0;

    // printf("SCOPE_WAVEFORM_DATA_SUBCMD: %02X %02X %02X ... %02X\n",
    //         buf[0], buf[1], buf[2], buf[length - 1]);
    // printf("  fixed=%02X division_number_bcd=%02X division_number_bcd_max=%02X\n",
    //         swd->fixed, swd->division_number_bcd, swd->division_number_bcd_max);
    if(1 == division_number) {

        static unsigned long lower_edge_last      = 0;
        static unsigned long higher_edge_last     = 0;
        static unsigned char center_or_fixed_last = 255;
        static unsigned char out_of_range_last    = 255;
        // TBD add handling of changing center_or_fixed or out_of_range.

        unsigned long lower_edge = PARSE_FIXED_EDGE_FREQUENCY(
                swd->extended_info_or_data.extended_info.info_03,
                swd->extended_info_or_data.extended_info.info_04,
                swd->extended_info_or_data.extended_info.info_05,
                swd->extended_info_or_data.extended_info.info_06,
                swd->extended_info_or_data.extended_info.info_07);

        unsigned long higher_edge = PARSE_FIXED_EDGE_FREQUENCY(
                swd->extended_info_or_data.extended_info.info_08,
                swd->extended_info_or_data.extended_info.info_09,
                swd->extended_info_or_data.extended_info.info_10,
                swd->extended_info_or_data.extended_info.info_11,
                swd->extended_info_or_data.extended_info.info_12);

        if(lower_edge_last      != lower_edge  ||
           higher_edge_last     != higher_edge ||
           center_or_fixed_last != swd->extended_info_or_data.extended_info.center_or_fixed ||
           out_of_range_last    != swd->extended_info_or_data.extended_info.out_of_range)
        {

            printf("  center_or_fixed=%02X lower_edge=%lu higher_edge=%lu out_of_range=%02X\n",
                swd->extended_info_or_data.extended_info.center_or_fixed,
                lower_edge,
                higher_edge,
                swd->extended_info_or_data.extended_info.out_of_range);

            waterfall::get_waterfall()->scale->makeScale(
                    static_cast<double>(lower_edge)  / 1000000.0,
                    static_cast<double>(higher_edge) / 1000000.0);

            lower_edge_last      = lower_edge;
            higher_edge_last     = higher_edge;
            center_or_fixed_last = swd->extended_info_or_data.extended_info.center_or_fixed;
            out_of_range_last    = swd->extended_info_or_data.extended_info.out_of_range;
        }

    } else {
        n_points = length - (sizeof(*swd) - (sizeof(swd->extended_info_or_data.data)) + 1);
        // printf("  first_point_n=%u n_points=%u\n", first_point_n, n_points);
        if(0 < n_points)
            waterfall::get_waterfall()->sig_data(
                const_cast<unsigned char *>(swd->extended_info_or_data.data),
                first_point_n,
                n_points);
    }
}
