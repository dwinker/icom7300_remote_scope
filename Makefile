CC=g++
CFLAGS=-g -c -Wall -std=c++11
FL_CFLAGS=$(CFLAGS) `fltk-config --cxxflags --use-images`
LIBS=`fltk-config --ldflags` -lfltk -lfltk_images -lstdc++ -lXfixes -lXext -lpthread -ldl -lm -lX11

remscp: remscp.o serial.o ic7300.o scope_waveform_data.o flslider2.o progdefaults.o scope_waveform_data.o waterfall.o
	$(CC) -o remscp remscp.o serial.o ic7300.o scope_waveform_data.o flslider2.o progdefaults.o waterfall.o $(LIBS)

remscp.o: remscp.cxx

remscp.o: remscp.cxx waterfall.h include/fldigi-config.h \
 include/digiscope.h include/complex.h include/flslider2.h serial.h \
 progdefaults.h
	$(CC) $(FL_CFLAGS) -MMD -MP -MF .deps/remscp.Tpo -c -Iinclude remscp.cxx

serial.o: serial.cxx serial.h ic7300.h
	$(CC) $(CFLAGS) -MMD -MP -MF .deps/serial.Tpo -c serial.cxx

ic7300.o: ic7300.cxx ic7300.h waterfall.h include/fldigi-config.h \
 include/digiscope.h include/complex.h include/flslider2.h
	$(CC) $(CFLAGS) -MMD -MP -MF .deps/ic7300.Tpo -c -I. -I./include ic7300.cxx

scope_waveform_data.o: scope_waveform_data.cxx scope_waveform_data.h \
 waterfall.h include/fldigi-config.h include/digiscope.h \
 include/complex.h include/flslider2.h ic7300.h
	$(CC) $(CFLAGS) -MMD -MP -MF .deps/scope_waveform_data.Tpo -c -I. -I./include scope_waveform_data.cxx

flslider2.o: flslider2.cxx include/config.h include/pkg.h include/util.h include/flslider2.h
	g++ -DHAVE_CONFIG_H -I. -DBUILD_FLDIGI -DLOCALEDIR=\"/usr/local/share/locale\" -I./include -I./irrxml -I./libtiniconv -I./fileselector -I./xmlrpcpp -DPKGDATADIR=\"/usr/local/share/fldigi\" -pthread -I/usr/include/alsa -D_REENTRANT -I/usr/include/libpng16 -pipe -Wall -fexceptions -O2 -O0 -fno-inline-functions -ggdb3 -Wall --param=max-vartrack-size=0 -I./xmlrpcpp --param=max-vartrack-size=0 -UNDEBUG -MT flslider2.o -MMD -MP -MF .deps/flslider2.Tpo -c -o flslider2.o flslider2.cxx

progdefaults.o: progdefaults.cxx
	g++ -DHAVE_CONFIG_H -I. -DBUILD_FLDIGI -DLOCALEDIR=\"/usr/local/share/locale\" -I./include -I./irrxml -I./libtiniconv -I./fileselector -I./xmlrpcpp -DPKGDATADIR=\"/usr/local/share/fldigi\" -pthread -I/usr/include/alsa -D_REENTRANT -I/usr/include/libpng16 -pipe -Wall -fexceptions -O2 -O0 -fno-inline-functions -ggdb3 -Wall --param=max-vartrack-size=0 -I./xmlrpcpp --param=max-vartrack-size=0 -UNDEBUG -MT flslider2.o -MMD -MP -MF .deps/progdefaults.Tpo -c -o progdefaults.o progdefaults.cxx

waterfall.o: waterfall.cxx include/fl_lock.h include/stacktrace.h \
 include/debug.h include/util.h include/config.h include/pkg.h \
 include/config.h waterfall.h include/digiscope.h \
 include/complex.h include/flslider2.h progdefaults.h \
 scope_waveform_data.h
	g++ -DHAVE_CONFIG_H -I. -DBUILD_FLDIGI -DLOCALEDIR=\"/usr/local/share/locale\" -I./include -I./irrxml -I./libtiniconv -I./fileselector -I./xmlrpcpp -DPKGDATADIR=\"/usr/local/share/fldigi\" -pthread -I/usr/include/alsa -D_REENTRANT -I/usr/include/libpng16 -pipe -Wall -fexceptions -O2 -O0 -fno-inline-functions -ggdb3 -Wall --param=max-vartrack-size=0 -I./xmlrpcpp --param=max-vartrack-size=0 -UNDEBUG -MT waterfall.o -MMD -MP -MF .deps/waterfall.Tpo -c -o waterfall.o waterfall.cxx

.PHONY: all
all: remscp

.PHONY: clean
clean:
	rm -f *~ *.o remscp a.out
