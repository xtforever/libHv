
.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) +z $*.c

#---------------------------------------------------------------------
# Define the C compiler
#---------------------------------------------------------------------

        CC = cc -Ae

#---------------------------------------------------------------------
# Define rm  & mv so as not to return errors
#---------------------------------------------------------------------

        RM =  -rm -f
        MV =  mv -f

#---------------------------------------------------------------------
# Define (relative to the source dir) the object dir
#---------------------------------------------------------------------

        OBJDIR = .

#---------------------------------------------------------------------
# Put all the libraries together with their paths
#---------------------------------------------------------------------

HV_XMLIBDIR=-L/usr/lib/Motif1.2 -lXm
HV_XLIBS=-L/usr/lib/X11R6 -lXt -lX11 
HV_LIBDIR=../../../src
HV_STANDARDLIBS= -lm

        HVLIB = $(HV_LIBDIR)/libHv.a
        LIBS = $(HVLIB) -L$(HV_XMLIBDIR) $(HV_XMLIBS)\
	   -L$(HV_XLIBDIR) $(HV_XLIBS)\
	   $(HV_STANDARDLIBS)

#---------------------------------------------------
# Program and library names
#---------------------------------------------------

        PROGRAM = simple

#---------------------------------------------------------------------
# The source files that make up the application
#---------------------------------------------------------------------

        SRCS = main.c init.c feedback.c setup.c
        OBJS = main.o init.o feedback.o setup.o

#---------------------------------------------------------------------
# This variable lists the subdirectories to search for include files.
#---------------------------------------------------------------------

XINCLUDEDIR   = /usr/include/X11R6
XMINCLUDEDIR  = /usr/include/Motif1.2
HV_INCLUDEDIR = -I.  -I../../../src

INCLUDES = -I$(HV_INCLUDEDIR)  -I$(HV_XINCLUDEDIR) -I$(HV_XMINCLUDEDIR)



all: $(PROGRAM)
	touch gbgtmp.o
#	$(RM) *.o
#	touch $(OBJDIR)/tmpfle
#	cp -p $(OBJDIR)/* .
#	$(RM) $(OBJDIR)/tmpfle
#	make $(PROGRAM)
#	$(RM) *.o

#---------------------------------------------------------------------
# This rule generates the executable using the object files and libraries.
#---------------------------------------------------------------------

     $(PROGRAM): $(OBJS)
	$(RM) $(PROGRAM)
	$(CC) -o $@ $(OBJS) $(LIBS)

#---------------------------------------------------------------------
# This rule cleans up the build directory (type "make clean").
#---------------------------------------------------------------------

     clean:
	$(RM) *.o core simple

#---------------------------------------------------------------------
# This rule makes the help file.
#---------------------------------------------------------------------

     help:
	$(RM) $(PROGRAM).help
	nroff beforenroff.help > $(PROGRAM).help

#Where to put the bins
HPBINDIR = /opt/Hv/bin/X11

install: all
	@echo "Installing EXEC files ...."
	if [ -d $(HPBINDIR) ]; then set +x; \
	else (set -x; mkdirhier $(HPBINDIR)); fi
	bsdinst -c -s simple $(HPBINDIR)

#---------------------------------------------------------------------
#additional dependencies
#---------------------------------------------------------------------

$(PROGRAM):      $(HVLIB)

main.o:     simple.h
init.o:     simple.h
feedback.o: simple.h
setup.o:    simple.h
