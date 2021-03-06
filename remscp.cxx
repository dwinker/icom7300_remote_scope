#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <X11/Xlib.h>   // For XParseGeometry()

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#include "complex.h"    // TBD - Has to come before one of these other includes. Weird.
#include "IC7300_waterfall.h"
#include "serial.h"
#include "progdefaults.h"
#include "fldigi-config.h"

static void print_usage_exit(void);
static void do_fltk(int width, int height);

#define BORDER     10
#define MAX_WIDTH  2048
#define MAX_HEIGHT 2048

int main(int argc, char **argv)
{
    int c;
    int retval = 0;

    static const struct option long_options[] =
    {
        {"device",   required_argument, 0, 'd'},
        {"geometry", optional_argument, 0, 'g'},
        {0, 0, 0, 0}
    };

    const char *devStr = "/dev/ttyUSB0";
    int x_opt = 0;
    int y_opt = 0;
    unsigned int width_opt  = 30u + 800u - 2u * DEFAULT_SW;
    unsigned int height_opt = 30u + 125u;

    /* getopt_long stores the option index here. */
    int option_index = 0;

    while (-1 != (c = getopt_long(argc, argv, "d:fg:", long_options, &option_index)))
    {
        switch (c)
        {
            case 'd':
              printf ("option -d with value '%s'\n", optarg);
              devStr = optarg;
              break;
            case 'g':
              printf ("option -g with value '%s'\n", optarg);
              XParseGeometry(optarg, &x_opt, &y_opt, &width_opt, &height_opt);
              if(0 != x_opt || 0 != y_opt)
                  puts("warning: x and y offsets from geometry not used.");
              break;
            case '?':
              /* getopt_long already printed an error message. */
              print_usage_exit();
              break;
            default:
              abort ();
        }
    }

    (void)serial_init(devStr);

    init_progdefaults();
    do_fltk(width_opt, height_opt);

    send_scope_wave_output_off();
    sleep(1);
    retval = serial_close();

    return retval;
}

static void do_fltk(int width, int height)
{
    waterfall *wf = (waterfall *)0;
    int run_value;

    if(0 > width || MAX_WIDTH < width)
        width = MAX_WIDTH;

    if(0 > height || MAX_HEIGHT < height)
        height = MAX_HEIGHT;

    Fl::lock();  // start the gui thread!!
    Fl::visual(FL_RGB); // insure 24 bit color operation
    Fl::scheme("gtk+");

    Fl_Window win(width, height, "ICOM-7300 Remote Scope"); // make a window
    wf = new waterfall(BORDER, BORDER, width - 2*BORDER, height - 2*BORDER);
    wf->end();
    win.show();

    run_value = Fl::run();

    printf("do_fltk: run_value=%d\n", run_value);
}

void print_usage_exit(void)
{
    puts("usage: remscp [-d|--device[=]/dev/ttyUSBx] [-g|--geometry[=]<width>{xX}<height>]  [-s|--serial] TBD TBD TBD TBD TBD");
    puts("  default device is /dev/ttyUSB0.");
    exit(1);
}
