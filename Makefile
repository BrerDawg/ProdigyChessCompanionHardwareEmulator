# A Makefile for both Linux and Windows, 10-nov-2016

#define all executables here
app_name= prodigy_app


all: ${app_name}


#define compiler options	
CC=g++

ifneq ($(OS),Windows_NT)			#linux?
#	CFLAGS=-g -O0 -fno-inline -Dbuild_date="\"`date +%Y\ %b\ %d`\"" #-Dbuild_date="\"2016 Mar 23\""
#	LIBS=-L/usr/X11/lib -L/usr/local/lib -lfltk_images /usr/local/lib/libfltk.a -lpng -lz -ljpeg -lrt -lm -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lpthread -ldl -lX11 -lfftw3 #-ljack
#	INCLUDE= -I/usr/local/include
	CFLAGS=-g -O0 -fno-inline
	CFLAGS=-g -O0 -Wno-deprecated-declarations -Wno-format-security -Wno-int-to-pointer-cast -fpermissive -fno-inline -Dbuild_date="\"`date +%Y-%b-%d`\"" #-Dbuild_date="\"2016-Mar-23\""			#64 bit
	LIBS=-lfltk -lfltk_images -lfltk_gl -lX11 -lpng -lz -ljpeg -lrt -lm -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lXrender -lpthread -ldl -lX11 #64 bit
else								#windows?
	CFLAGS=-g -O0 -fno-inline -DWIN32 -mms-bitfields -Dcompile_for_windows -Dbuild_date="\"`date +%Y\ %b\ %d`\""
LIBS= -L/usr/local/lib -static -mwindows -lfltk_images -lfltk -lfltk_png -lfltk_z -lfltk_jpeg -lole32 -luuid -lcomctl32 -lwsock32 -lWs2_32 -lm -lfftw3 -lwinmm
	INCLUDE= -I/usr/local/include
endif



#define object files for each executable, see dependancy list at bottom
obj1= prodigy_main.o GCProfile.o pref.o GCLed.o GCCol.o gclog.o gcthrd.o gcpipe.o mos6502.o chss.o seg7.o
#obj2= backprop.o layer.o



#linker definition
prodigy_app: $(obj1)
	$(CC) $(CFLAGS) -o $@ $(obj1) $(LIBS)


#linker definition
#backprop: $(obj2)
#	$(CC) $(CFLAGS) -o $@ $(obj2) $(LIBS)



#compile definition for all cpp files to be complied into .o files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

%.o: %.cxx
	$(CC) $(CFLAGS) $(INCLUDE) -c $<



#dependancy list per each .o file
prodigy_main.o: prodigy_main.h globals.h GCProfile.h pref.h GCCol.h GCLed.h gclog.h gcthrd.h gcpipe.h mos6502.h chss.h seg7.h
GCProfile.o: GCProfile.h
pref.o: pref.h GCCol.h GCLed.h
GCCol.o:  GCCol.h
GCLed.o: GCLed.h
gclog.o:  gclog.h GCProfile.h
gcthrd.o: gcthrd.h GCProfile.h gclog.h
gcpipe.o: gcpipe.h GCProfile.h
mos6502.o: mos6502.h
chss.o: chss.h globals.h GCProfile.h pref.h GCCol.h gclog.h gcthrd.h gcpipe.h #my_colour.h
seg7: seg7.h  GCProfile.h GCCol.h GCLed.h 
#layer.o: layer.h


.PHONY : clean
clean : 
		-rm $(obj1)					#remove obj files
ifneq ($(OS),Windows_NT)
		-rm ${app_name}				#remove linux exec
else
		-rm ${app_name}.exe			#remove windows exec
endif


