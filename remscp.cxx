#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#include "waterfall.h"
#include "serial.h"
#include "progdefaults.h"

static void print_usage_exit(void);
static void do_fltk(void);

int main(int argc, char **argv)
{
    int c;
    int retval;

    static const struct option long_options[] =
    {
        {"device", required_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    const char *devStr = "/dev/ttyUSB0";

    /* getopt_long stores the option index here. */
    int option_index = 0;

    while (-1 != (c = getopt_long(argc, argv, "d:", long_options, &option_index)))
    {
        switch (c)
        {
            case 'd':
              printf ("option -d with value `%s'\n", optarg);
              devStr = optarg;
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
    do_fltk();
    send_scope_wave_output_off();
    sleep(1);
    retval = serial_close();

    return retval;
}

#define DEFAULT_SW 16
void do_fltk(void)
{
    waterfall *wf = (waterfall *)0;
    int run_value;

    int Hwfall = 125;
    int Wwfall = 800 - 2 * DEFAULT_SW;

	Fl::lock();  // start the gui thread!!
	Fl::visual(FL_RGB); // insure 24 bit color operation
	Fl::scheme("gtk+");

    Fl_Window win(Wwfall + 30, Hwfall + 30); // make a window
    wf = new waterfall(10, 10, Wwfall + 10, Hwfall + 10);
    wf->end();
    win.show();

    run_value = Fl::run();

    printf("do_fltk: run_value=%d\n", run_value);
}

void print_usage_exit(void)
{
    puts("usage: remscp [-d|--device /dev/ttyUSBx] [-s|--serial] TBD TBD TBD TBD TBD");
    puts("  default device is /dev/ttyUSB0.");
    exit(1);
}
