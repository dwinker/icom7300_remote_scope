/* src/config.h.  Generated from config.h.in by configure.  */
/* src/config.h.in.  Generated from configure.ac by autoheader.  */


#ifndef CONFIG_H_
#define CONFIG_H_


/* Defined if we are building for benchmarking */
#define BENCHMARK_MODE 0

/* Build platform */
#define BUILD_BUILD_PLATFORM "x86_64-pc-linux-gnu"

/* Compiler */
#define BUILD_COMPILER "gcc version 7.2.1 20170915 (Red Hat 7.2.1-2) (GCC) "

/* Configure arguments */
#define BUILD_CONFIGURE_ARGS "'--enable-debug' '--with-pulseaudio'"

/* Build date */
#define BUILD_DATE "Tue Jan 23 13:30:38 CST 2018"

/* Build host */
#define BUILD_HOST "gozer.localdomain"

/* Host platform */
#define BUILD_HOST_PLATFORM "x86_64-pc-linux-gnu"

/* Target platform */
#define BUILD_TARGET_PLATFORM "x86_64-pc-linux-gnu"

/* Build user */
#define BUILD_USER "dwinker"

/* Define to 1 if the `closedir' function returns void instead of `int'. */
/* #undef CLOSEDIR_VOID */

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#define ENABLE_NLS 1

/* Flarq authors */
#define FLARQ_AUTHORS "Dave Freese, Robert Stiles, and others"

/* Flarq compiler flags */
#define FLARQ_BUILD_CXXFLAGS "-I$(srcdir) -I$(srcdir)/include -I$(srcdir)/fileselector -I$(srcdir)/flarq-src -I$(srcdir)/flarq-src/include -I$(srcdir)/xmlrpcpp -pipe -Wall -fexceptions -O2 -O0 -fno-inline-functions -ggdb3 -Wall -I$(srcdir)/xmlrpcpp -UNDEBUG"

/* Flarq linker flags */
#define FLARQ_BUILD_LDFLAGS "-rdynamic -lfltk_images -lfltk -lX11 -ldl -lpthread"

/* Flarq Docs index */
#define FLARQ_DOCS "http://www.w1hkj.com/FlarqHelpFiles/flarq.html"

/* Flarq version string */
#define FLARQ_VERSION "4.3.6"

/* Flarq major version number */
#define FLARQ_VERSION_MAJOR 4

/* Flarq minor version number */
#define FLARQ_VERSION_MINOR 3

/* Flarq patch/alpha version string */
#define FLARQ_VERSION_PATCH ".6"

/* Fldigi authors */
#define FLDIGI_AUTHORS "Dave Freese, Stelios Bounanos, and others"

/* Fldigi compiler flags */
#define FLDIGI_BUILD_CXXFLAGS "-I$(srcdir) -I$(srcdir)/include -I$(srcdir)/irrxml -I$(srcdir)/libtiniconv -I$(srcdir)/fileselector -I$(srcdir)/xmlrpcpp -pthread -I/usr/include/alsa -D_REENTRANT -I/usr/include/libpng16 -pipe -Wall -fexceptions -O2 -O0 -fno-inline-functions -ggdb3 -Wall --param=max-vartrack-size=0 -I$(srcdir)/xmlrpcpp --param=max-vartrack-size=0 -UNDEBUG"

/* Fldigi linker flags */
#define FLDIGI_BUILD_LDFLAGS "-rdynamic -lportaudio -lm -lpthread -lasound -lfltk_images -lfltk -lX11 -lsndfile -lsamplerate -lpulse-simple -lpulse -lpng16 -lz -ldl -lpthread"

/* FLTK API major version */
#define FLDIGI_FLTK_API_MAJOR 1

/* FLTK API minor version */
#define FLDIGI_FLTK_API_MINOR 3

/* FLTK API patch */
#define FLDIGI_FLTK_API_PATCH 4

/* Fldigi version string */
#define FLDIGI_VERSION "4.0.15"

/* Fldigi major version number */
#define FLDIGI_VERSION_MAJOR 4

/* Fldigi minor version number */
#define FLDIGI_VERSION_MINOR 0

/* Fldigi patch/alpha version string */
#define FLDIGI_VERSION_PATCH ".15"

/* FLTK version */
#define FLTK_BUILD_VERSION "1.3.4"

/* flxmlrpc version */
#define FLXMLRPC_BUILD_VERSION ""

/* Define to 0 if gcc < 4.1.0 */
#define GCC_VER_OK 1

/* hamlib version */
#define HAMLIB_BUILD_VERSION ""

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the Mac OS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
/* #undef HAVE_CFLOCALECOPYCURRENT */

/* Define to 1 if you have the Mac OS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
/* #undef HAVE_CFPREFERENCESCOPYAPPVALUE */

/* Define to 1 if we have clock_gettime */
#define HAVE_CLOCK_GETTIME 1

/* define if the compiler supports basic C++11 syntax */
#define HAVE_CXX11 1

/* Define to 0 if not using dbg::stack */
#define HAVE_DBG_STACK 0

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#define HAVE_DCGETTEXT 1

/* Define to 1 if you have the <dev/ppbus/ppi.h> header file. */
/* #undef HAVE_DEV_PPBUS_PPI_H */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if we have dlopen */
#define HAVE_DLOPEN 1

/* Define to 1 if you have the <execinfo.h> header file. */
#define HAVE_EXECINFO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `getaddrinfo' function. */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `gethostbyname' function. */
#define HAVE_GETHOSTBYNAME 1

/* Define if the GNU gettext() function is already present or preinstalled. */
#define HAVE_GETTEXT 1

/* Define to 1 if you have the `gmtime_r' function. */
#define HAVE_GMTIME_R 1

/* Define to 1 if you have the `hstrerror' function. */
#define HAVE_HSTRERROR 1

/* Define if you have the iconv() function and it works. */
/* #undef HAVE_ICONV */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <linux/ppdev.h> header file. */
#define HAVE_LINUX_PPDEV_H 1

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the `mkdir' function. */
#define HAVE_MKDIR 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have O_CLOEXEC defined */
#define HAVE_O_CLOEXEC 1

/* Define to 1 if you have the <regex.h> header file. */
#define HAVE_REGEX_H 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* Define to 1 if we have sem_timedwait */
#define HAVE_SEM_TIMEDWAIT 1

/* Define to 1 if we have sem_unlink */
#define HAVE_SEM_UNLINK 1

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the `socket' function. */
#define HAVE_SOCKET 1

/* Define to 1 if you have the `socketpair' function. */
#define HAVE_SOCKETPAIR 1

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Defined 0 if we do not have std::bind in <functional> */
#define HAVE_STD_BIND 1

/* Defined 0 if we do not have std::bind in <unordered_map> */
#define HAVE_STD_HASH 1

/* Defined 0 if we do not have std::tr1::bind in <tr1/functional> */
/* #undef HAVE_STD_TR1_BIND */

/* Defined 0 if we do not have std::tr1::hash in <tr1/unordered_map> */
/* #undef HAVE_STD_TR1_HASH */

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strcasestr' function. */
#define HAVE_STRCASESTR 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcpy' function. */
/* #undef HAVE_STRLCPY */

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP 1

/* Define to 1 if you have the `strncmp' function. */
#define HAVE_STRNCMP 1

/* Define to 1 if you have the `strncpy' function. */
#define HAVE_STRNCPY 1

/* Define to 1 if you have the `strnlen' function. */
#define HAVE_STRNLEN 1

/* Define to 1 if you have the `strrchr' function. */
#define HAVE_STRRCHR 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the `strtol' function. */
#define HAVE_STRTOL 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/utsname.h> header file. */
#define HAVE_SYS_UTSNAME_H 1

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* Define to 1 if you have the `uname' function. */
#define HAVE_UNAME 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `unsetenv' function. */
#define HAVE_UNSETENV 1

/* Define to 1 if you have the <values.h> header file. */
#define HAVE_VALUES_H 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* Name of package */
#define PACKAGE "fldigi"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "fldigi-devel@sourceforge.net"

/* Download page */
#define PACKAGE_DL "http://www.w1hkj.com/download.html"

/* Docs index */
#define PACKAGE_DOCS "http://www.w1hkj.com/FldigiHelp/index.html"

/* Beginners guide */
#define PACKAGE_GUIDE "http://www.w1hkj.com/beginners.html"

/* Home page */
#define PACKAGE_HOME "http://www.w1hkj.com/Fldigi.html"

/* Define to the full name of this package. */
#define PACKAGE_NAME "fldigi"

/* Trac new ticket page */
#define PACKAGE_NEWBUG "https://fedorahosted.org/fldigi/newticket"

/* Sourceforge page */
#define PACKAGE_PROJ "http://sourceforge.net/projects/fldigi/files/"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "fldigi 4.0.15"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "fldigi"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "4.0.15"

/* png version */
#define PNG_BUILD_VERSION "1.6.31"

/* portaudio version */
#define PORTAUDIO_BUILD_VERSION "19"

/* pulseaudio version */
#define PULSEAUDIO_BUILD_VERSION "11.1-rebootstrapped"

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* samplerate version */
#define SAMPLERATE_BUILD_VERSION "0.1.8"

/* Define to the type of arg 1 for `select'. */
#define SELECT_TYPE_ARG1 int

/* Define to the type of args 2, 3 and 4 for `select'. */
#define SELECT_TYPE_ARG234 (fd_set *)

/* Define to the type of arg 5 for `select'. */
#define SELECT_TYPE_ARG5 (struct timeval *)

/* sndfile version */
#define SNDFILE_BUILD_VERSION "1.0.28"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Define to 1 if we are using flxmlrpc */
#define USE_FLXMLRPC 0

/* Define to 1 if we are using hamlib */
#define USE_HAMLIB 0

/* Defined if we are using OSS */
#define USE_OSS 1

/* Define to 1 if we are using png */
#define USE_PNG 1

/* Define to 1 if we are using portaudio */
#define USE_PORTAUDIO 1

/* Define to 1 if we are using pulseaudio */
#define USE_PULSEAUDIO 1

/* Define to 1 if we are using samplerate */
#define USE_SAMPLERATE 1

/* Define to 1 if we are using sndfile */
#define USE_SNDFILE 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Defined if we are using TLS */
#define USE_TLS 1

/* Define to 1 if we are using x */
#define USE_X 1

/* Version number of package */
#define VERSION "4.0.15"

/* x version */
#define X_BUILD_VERSION "1.6.5"

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* Define to 1 if we are building on cygwin or mingw */
/* #undef _WINDOWS */

/* Define to 1 if we are building on cygwin or mingw */
/* #undef __MINGW32__ */

/* Define to 1 if we are building on cygwin or mingw */
/* #undef __WOE32__ */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int16_t */

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int64_t */

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#define restrict __restrict
/* Work around a bug in Sun C++: it does not support _Restrict or
   __restrict__, even though the corresponding Sun C compiler ends up with
   "#define restrict _Restrict" or "#define restrict __restrict__" in the
   previous line.  Perhaps some future version of Sun C++ will work with
   restrict; if so, hopefully it defines __RESTRICT like Sun C does.  */
#if defined __SUNPRO_CC && !defined __RESTRICT
# define _Restrict
# define __restrict__
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */


#include "pkg.h"
#include "util.h"
#endif /* CONFIG_H_ */

