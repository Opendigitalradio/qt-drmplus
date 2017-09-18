
EXTRA_DIST = libdrmplus.sym

lib_LTLIBRARIES = libdrmplus.la

libdrmplus_la_CFLAGS = @FFTW3_INCFLAGS@ -I$(top_srcdir)/include
libdrmplus_la_LDFLAGS = -export-symbols libdrmplus.sym -no-undefined -version-info 1:0:1 @FFTW3_LDFLAGS@ @FFTW3_LIBS@ -lfaad2_drm

libdrmplusdir = $(includedir)/libdrmplus

libdrmplus_la_SOURCES = drmplus.c \
	sig_sync.c sig_proc.c \
	parser_fac.c parser_sdc.c parser_msc.c parser_str.c audiotext.c \
	rom_tables.c utils.c midiviterbi.c

noinst_HEADERS = drmplus_internal.h audiotext.h
