#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>  /* File Control Definitions          */
#include <termios.h>/* POSIX Terminal Control Definitions*/
#include <unistd.h> /* UNIX Standard Definitions         */
#include <sys/ioctl.h>
#include <pthread.h>
#include <assert.h>

static void serial_port_init(struct termios *ptio);
static int  serial_send(const unsigned char *buf, int n);
static void *serial_listen_thread_loop(void *pfd);

/* These are per the "Data format" section of the Full Manual. */
const unsigned char PREAMBLE    = 0xFE;
const unsigned char XCVR_ADDR   = 0x94; /* Transceiver's default address */
const unsigned char CONT_ADDR   = 0xE0; /* Controller's default address */
const unsigned char OK_CODE     = 0xFB;
const unsigned char NG_CODE     = 0xFA;
const unsigned char END_MESSAGE = 0xFD;

static int            f_fd = 0;
static struct termios f_tio_orig;

static pthread_t listen_thread;
static bool      f_done = false;

//const unsigned char READ_SCOPE_WAVEFORM_DATA = {0x27, 0x00, 

//#pragma pack(1)
//struct scope_waveform_data_tag {
//    unsigned char zero;
//    unsigned char current_division;
//    unsigned char max_division;
//    union {
//        struct extra_waveform_data_tag {
//            unsigned char fixed_flag;
//            unsigned char waveform_info;
//            unsigned char out_of_range_flag;
//        } extra_waveform_data_tag;
//        unsigned char waveform_data[4];
//    };
//} x;

int serial_init(const char *devStr)
{
    struct termios tio;
    int rc;

//    printf("sizeof x = %d\n", sizeof(x));
//    exit(1);

    f_fd = open(devStr, O_RDWR | O_NOCTTY);
    if(f_fd == 1) {
        printf("Error Opening %s\n", devStr);
        f_fd = 0;
        return 1;
    }

    /* Get the current serial port configuration and save it off for
     * restoration later. */
    rc = tcgetattr(f_fd, &f_tio_orig);
    if(rc < 0) {
        printf("main: failed to get attr: %d, %s\n", rc, strerror(errno));
    }

    tio = f_tio_orig;

    /* Modify the current configuration for our use here. */
    serial_port_init(&tio);

    // Start the listen thread.
    pthread_create(&listen_thread, NULL, serial_listen_thread_loop, static_cast<void *>(&f_fd));

    return 0;
}

int serial_close(void)
{
    int rc;

    f_done = true;
    puts("Joining Listen Thread");
    pthread_join(listen_thread, NULL);
    puts("Joined");

    rc = tcsetattr(f_fd, TCSADRAIN, &f_tio_orig);
    if(rc < 0) {
        printf("main: failed to restore attr: %d, %s\n", rc, strerror(errno));
    }

    return close(f_fd);
}

void send_scope_on(void)
{
    unsigned char buf[20];
    int nsent;
    int n;

    n = 0;
    buf[n++] = PREAMBLE;
    buf[n++] = XCVR_ADDR;
    buf[n++] = CONT_ADDR;
    buf[n++] = 0x27;
    buf[n++] = 0x10;
    buf[n++] = 0x01;
    buf[n++] = END_MESSAGE;

    nsent = serial_send(buf, n);

    printf("send_scope_on: sent %d of %d bytes.\n", nsent, n);

    //nsent = read(f_fd, buf, sizeof(buf));
    //printf("send_scope_on: read %d bytes:", nsent);
    //for(n = 0; n < nsent; n++) {
        //printf(" %02X", buf[n]);
    //}
    //putchar('\n');

    // Check for correct response.
    //puts( (CONT_ADDR   == buf[nsent - 4] &&
           //XCVR_ADDR   == buf[nsent - 3] &&
           //OK_CODE     == buf[nsent - 2] &&
           //END_MESSAGE == buf[nsent - 1]) ? "OK" : "No Good");
}

void send_scope_off(void)
{
    unsigned char buf[20];
    int nsent;
    int n;

    assert(f_fd);

    n = 0;
    buf[n++] = PREAMBLE;
    buf[n++] = XCVR_ADDR;
    buf[n++] = CONT_ADDR;
    buf[n++] = 0x27;
    buf[n++] = 0x10;
    buf[n++] = 0x00;
    buf[n++] = END_MESSAGE;

    nsent = serial_send(buf, n);

    printf("send_scope_off: sent %d of %d bytes.\n", nsent, n);

    nsent = read(f_fd, buf, sizeof(buf));
    printf("send_scope_off: read %d bytes:", nsent);
    for(n = 0; n < nsent; n++) {
        printf(" %02X", buf[n]);
    }
    putchar('\n');

    // Check for correct response.
    puts( (CONT_ADDR   == buf[nsent - 4] &&
           XCVR_ADDR   == buf[nsent - 3] &&
           OK_CODE     == buf[nsent - 2] &&
           END_MESSAGE == buf[nsent - 1]) ? "OK" : "No Good");
}

void send_scope_wave_output_on(void)
{
    unsigned char buf[20];
    int nsent;
    int n;

    n = 0;
    buf[n++] = PREAMBLE;
    buf[n++] = XCVR_ADDR;
    buf[n++] = CONT_ADDR;
    buf[n++] = 0x27;
    buf[n++] = 0x11;
    buf[n++] = 0x01;
    buf[n++] = END_MESSAGE;

    nsent = serial_send(buf, n);

    printf("send_scope_wave_output_on: sent %d of %d bytes.\n", nsent, n);

    //nsent = read(f_fd, buf, sizeof(buf));
    //printf("send_scope_on: read %d bytes:", nsent);
    //for(n = 0; n < nsent; n++) {
        //printf(" %02X", buf[n]);
    //}
    //putchar('\n');

    // Check for correct response.
    //puts( (CONT_ADDR   == buf[nsent - 4] &&
           //XCVR_ADDR   == buf[nsent - 3] &&
           //OK_CODE     == buf[nsent - 2] &&
           //END_MESSAGE == buf[nsent - 1]) ? "OK" : "No Good");
}

void send_scope_wave_output_off(void)
{
    unsigned char buf[20];
    int nsent;
    int n;

    n = 0;
    buf[n++] = PREAMBLE;
    buf[n++] = XCVR_ADDR;
    buf[n++] = CONT_ADDR;
    buf[n++] = 0x27;
    buf[n++] = 0x11;
    buf[n++] = 0x00;
    buf[n++] = END_MESSAGE;

    nsent = serial_send(buf, n);

    printf("send_scope_wave_output_on: sent %d of %d bytes.\n", nsent, n);

    //nsent = read(f_fd, buf, sizeof(buf));
    //printf("send_scope_on: read %d bytes:", nsent);
    //for(n = 0; n < nsent; n++) {
        //printf(" %02X", buf[n]);
    //}
    //putchar('\n');

    // Check for correct response.
    //puts( (CONT_ADDR   == buf[nsent - 4] &&
           //XCVR_ADDR   == buf[nsent - 3] &&
           //OK_CODE     == buf[nsent - 2] &&
           //END_MESSAGE == buf[nsent - 1]) ? "OK" : "No Good");
}


void get_scope_waveform_data(void)
{
    unsigned char buf[200];
    int nsent;
    int n;
    int i;

    assert(f_fd);

    for(i = 0; i < 10; i++) {
        nsent = read(f_fd, buf, sizeof(buf));
        printf("get_scope_waveform_data: try %d read %d bytes:", i, nsent);
        for(n = 0; n < nsent; n++) {
            printf(" %02X", buf[n]);
        }
        putchar('\n');
    }

    // Check for correct response.
    puts( (CONT_ADDR   == buf[nsent - 4] &&
           XCVR_ADDR   == buf[nsent - 3] &&
           OK_CODE     == buf[nsent - 2] &&
           END_MESSAGE == buf[nsent - 1]) ? "OK" : "No Good");
}

// Mostly sets the serial port to raw. 
static void serial_port_init(struct termios *ptio)
{
    int rc;

    assert(f_fd);

    // Baudrate, fast
    cfsetispeed(ptio, B115200);
    cfsetospeed(ptio, B115200);

    // This is almost exactly like raw mode. We're going to set ICANON so we can detect
    // end of message by 0xFD from the ICOM radio though.
    ptio->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    ptio->c_oflag &= ~OPOST;
    ptio->c_lflag &= ~(ECHO | ECHONL | ISIG | IEXTEN);
    ptio->c_cflag &= ~(CSIZE | PARENB);
    ptio->c_cflag |= CS8;

    // Setting ICANON.
    // IEXTEN not set means these have no effect: EOL2, LNEXT, REPRINT, WERASE, IUCLC.
    memset(ptio->c_cc, 0, sizeof(ptio->c_cc));

//#define CANON
#ifdef  CANON
    ptio->c_lflag |= ICANON;
    ptio->c_cc[VEOL] = 0xFD;
#else
    // Wait 1 tenth second before returing read.  This makes a purely timed
    // read. read(f_fd...) will always take about 1/10 second.
    ptio->c_lflag &= ~ICANON;
    ptio->c_cc[VMIN]  = 0;
    ptio->c_cc[VTIME] = 1;
#endif

    // Number of Stop bits = 1, so we clear the CSTOPB bit.
    ptio->c_cflag &= ~CSTOPB; //Stop bits = 1

    // Turn off hardware based flow control (RTS/CTS).
    ptio->c_cflag &= ~CRTSCTS;

    // Turn on the receiver of the serial port (CREAD).
    ptio->c_cflag |= (CREAD | CLOCAL);

    rc = tcsetattr(f_fd, TCSANOW, ptio);
    if(rc < 0) {
        printf("init_serial: failed to set attr: %d, %s\n", rc, strerror(errno));
        exit(1);
    }
}

static int serial_send(const unsigned char *buf, int n)
{
    int nsent;

    assert(f_fd);
    printf("serial_send: sending %d bytes:", n);
    for(int i = 0; i < n; i++) {
        printf(" %02X", buf[i]);
    }

    nsent = write(f_fd, buf, n);

    if(nsent == n) {
        puts(": success");
    } else {
        printf(": ERROR: sent only %d bytes.\n", nsent);
    }
    putchar('\n');

    return nsent;
}

static void *serial_listen_thread_loop(void *pfd)
{
    // Measured approximately 29 '27 00 001 blocks in approximately 599 mS. So,
    // we're getting a block about once every 20 mS.  Most of these blocks are
    // 59 bytes. 115,200 baud is approximately 11,520 bytes per second. So, it
    // takes about 5 mS to transmit a block.
    //
    // When doing 100 mS timed reads we can easily end up with six ~60 byte
    // '27 00 00' blocks + a few other responses in the receive buffer at any
    // time. 1024 is more than double what should be necessary, and doesn't
    // seem too big.
    int n, nread;
    unsigned char buf[1024];

    while(!f_done) {
        nread = read(f_fd, buf, sizeof(buf));
        if(nread) {
            printf("serial_listen_thread_loop: read %d bytes:", nread);
            for(n = 0; n < nread; n++) {
                printf(" %02X", buf[n]);
            }
            putchar('\n');
        }
    }

    return NULL;
}
