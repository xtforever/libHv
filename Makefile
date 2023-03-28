# I'v only changed the Makfiles in ./src and ./demos/simple/src
# If you want to build hpdraw etc it should be easy to follow 
# my example.  (Gmake users: copy Makefile.cln back onto Makefile.)
#
# Steph.


all:
	cd src; make
	cd demos/simple/src; make

clean:
	cd src; make clean
	cd demos/simple/src; make clean

install:
	cd src; make install
	cd demos/simple/src; make install



exe:
	@echo "Hv only builds as a library + a demo"
