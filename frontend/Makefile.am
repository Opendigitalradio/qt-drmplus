bin_PROGRAMS = drmplusdemod drmplusgraph
man_MANS = ../docs/drmplusdemod.1

drmplusdemod_SOURCES = drmplusdemod.c fac_display.c sdc_display.c

drmplusgraph_SOURCES = drmplusgraph.c fac_display.c sdc_display.c graphs_callbacks.c gnuplot_i.c

drmplusdemod_CFLAGS = -I$(top_srcdir)/include  -I$(top_srcdir)/src
drmplusgraph_CFLAGS = -I$(top_srcdir)/include  -I$(top_srcdir)/src
LDADD = $(top_builddir)/src/libdrmplus.la -lm

drmplusdemod_LDFLAGS = @FFTW3_LDFLAGS@ -lfaad2_drm
drmplusgraph_LDFLAGS = @FFTW3_LDFLAGS@ -lfaad2_drm
