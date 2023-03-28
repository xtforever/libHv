#---------------------------------------------------------------------
# Define the C compiler
#---------------------------------------------------------------------

        CC = $(HV_CC)

#---------------------------------------------------------------------
# Define rm  & mv so as not to return errors
#---------------------------------------------------------------------

        RM =  rm -f
        MV =  mv -f

#---------------------------------------------------------------------
# Define (relative to the source dir) the object dir
#---------------------------------------------------------------------

        OBJDIR = ../$(HV_HOSTOS)

#---------------------------------------------------------------------
# Put all the libraries together with their paths
#---------------------------------------------------------------------

        HVLIB = $(HV_LIBDIR)/libHv.a
        LIBS = $(HVLIB) -L$(HV_XMLIBDIR) $(HV_XMLIBS)\
	   -L$(HV_XLIBDIR) $(HV_XLIBS)\
	   $(HV_STANDARDLIBS)

#---------------------------------------------------
# Program and library names
#---------------------------------------------------

        PROGRAM = hvplot

#---------------------------------------------------------------------
# The source files that make up the application
#---------------------------------------------------------------------

        SRCS = main.c init.c setup.c

#---------------------------------------------------------------------
# The object files (via macro substitution)
#---------------------------------------------------------------------

        OBJS = ${SRCS:.c=.o}

#---------------------------------------------------------------------
# This variable lists the subdirectories to search for include files.
#---------------------------------------------------------------------

      INCLUDES = -I$(HV_INCLUDEDIR)\
        -I$(HV_XINCLUDEDIR)\
        -I$(HV_XMINCLUDEDIR)

#---------------------------------------------------------------------
# How to make a .o file from a .c file 
#---------------------------------------------------------------------


.c.o :
	$(CC) $(HV_CFLAGS) $(INCLUDES) $<


#--------------------------------------------------------
# Default target

#the tmpfle business is just so mv does not
#give an error if no files are there to move

#--------------------------------------------------------

     all:
	touch gbgtmp.o
	$(RM) *.o
	$(RM) $(PROGRAM)
	touch $(OBJDIR)/tmpfle
	$(MV) $(OBJDIR)/* .
	$(RM) $(OBJDIR)/tmpfle
	make $(PROGRAM)
	$(MV) *.o $(OBJDIR)


#---------------------------------------------------------------------
# This rule generates the executable using the object files and libraries.
#---------------------------------------------------------------------

     $(PROGRAM): $(OBJS)
	$(RM) $(PROGRAM)
	$(CC) -o $@ $(OBJS) $(LIBS)
	cp $(PROGRAM) $(OBJDIR)

#---------------------------------------------------------------------
# This rule cleans up the build directory (type "make clean").
#---------------------------------------------------------------------

     clean:
	$(RM) *.o core

#---------------------------------------------------------------------
# This rule makes the help file.
#---------------------------------------------------------------------

     help:
	$(RM) hvplot.help
	nroff beforenroff.help > hvplot.help

#---------------------------------------------------------------------
#additional dependencies
#---------------------------------------------------------------------

$(PROGRAM):      $(HVLIB)

main.o:      plot.h
setup.o:     plot.h
init.o:      plot.h
