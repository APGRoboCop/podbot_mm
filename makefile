# Console command(s) for compiling on Cygwin/MinGW:
#   Example Cygwin: make HOST=cygwin
# 	http://www.xake.dk/target/tools.html

MODNAME = podbot_mm
SRCFILES = bot.cpp bot_chat.cpp bot_client.cpp bot_combat.cpp bot_globals.cpp \
bot_sounds.cpp compress.cpp dll.cpp engine.cpp util.cpp bot_navigate.cpp waypoint.cpp

RESFILE = podbot_mm.rc

EXTRA_LIBS_LINUX =
EXTRA_LIBS_WIN32 =
EXTRA_LIBDIRS_LINUX = -Lextra/lib_linux
EXTRA_LIBDIRS_WIN32 = -Lextra/lib_win32

#EXTRA_FLAGS = -Dstrcmpi=strcasecmp

SDKTOP=../hlsdk-2.3-p4
SDKSRC=$(SDKTOP)/multiplayer
METADIR=../metamod-p/metamod
SRCDIR=.

OBJDIR_LINUX=obj.linux
OBJDIR_WIN32=obj.win32

ifeq "$(shell uname | cut -d _ -f1)" "CYGWIN"
	HOST=cygwin
endif

#for linux platform - need to start it so for cygwin - to generate win32 dll under linux
MCPU=-march

ifdef ComSpec
	ifeq "$(HOST)" "cygwin"
		OS=LINUX
#		MCPU=-mcpu
		MCPU=-mtune
		PATH_WIN=/usr/local/cross-tools/i386-mingw32msvc/bin
		CLEAN=clean_linux
	else
		OS=WIN32
		MCPU=-mtune
		PATH_WIN=/mingw/bin
		CLEAN=clean_win32
	endif
else
	ifdef COMSPEC
		ifeq "$(HOST)" "cygwin"
			OS=LINUX
#			MCPU=-mcpu
			MCPU=-mtune
			PATH_WIN=/usr/local/cross-tools/i386-mingw32msvc/bin
			CLEAN=clean_linux
		else
			OS=WIN32
			MCPU=-mtune
			PATH_WIN=/mingw/bin
			CLEAN=clean_msys_w32
		endif
	else
		OS=LINUX
		PATH_WIN=/usr/local/cross-tools/i386-mingw32msvc/bin
		CLEAN=clean_linux
#		MCPU=-mcpu
		MCPU=-mtune
	endif
endif

ifeq "$(HOST)" "cygwin"
	ifeq "$(ARCH)" "amd64"
		CC_LINUX=g++-linux-x86_64
	else
		CC_LINUX=g++-linux-4.1
#		CC_LINUX=g++-linux
	endif
else
	ifeq "$(ARCH)" "amd64"
		CC_LINUX=g++-4.8-x86_64
	else
		CC_LINUX=g++-4.8
	endif
endif

ifeq "$(OS)" "LINUX"
	INSTALL=install -m 644
	LD_WINDLL= PATH=$(PATH_WIN) $(PATH_WIN)/dllwrap
	DEFAULT=linux
#	CLEAN=clean_linux
else	# WIN32
	INSTALL=cp
	CC_WIN32=g++-4.8
	LD_WINDLL= dllwrap
	DEFAULT=win32
#	CLEAN=clean_win32
endif

RES_WIN32=windres

# Just call everything i386 unless its a 64bit binary!
ifeq "$(ARCH)" "amd64"
	LIBFILE_LINUX = $(MODNAME)_amd64.so
else
	LIBFILE_LINUX = $(MODNAME).so
endif

LIBFILE_WIN32 = $(MODNAME).dll
TARGET_LINUX = $(OBJDIR_LINUX)/$(LIBFILE_LINUX)
TARGET_WIN32 = $(OBJDIR_WIN32)/$(LIBFILE_WIN32)

FILES_ALL = *.cpp *.h [A-Z]* *.rc
ifeq "$(OS)" "LINUX"
#	ASRCFILES := $(shell ls -t $(SRCFILES))
else
#	ASRCFILES := $(shell dir /b)
endif

OBJ_LINUX := $(SRCFILES:%.cpp=$(OBJDIR_LINUX)/%.o)
OBJ_WIN32 := $(SRCFILES:%.cpp=$(OBJDIR_WIN32)/%.o)
RES_OBJ_WIN32 := $(RESFILE:%.rc=$(OBJDIR_WIN32)/%.o)

# optimisation level; overridden for certain problematic files
CCO = -w -O2 -fomit-frame-pointer -s -pipe -mmmx -msse -msse2 -mfpmath=sse

#use this one for debug
#CCO = -ggdb

# architecture tuning by arch type
ifeq "$(ARCH)" "amd64"
#	CCOPT_ARCH = -D__amd64__=1
	CCOPT_ARCH =
else
	CCOPT_ARCH = -m32 -march=i686 $(MCPU)=generic
endif

BASEFLAGS = -Dstricmp=strcasecmp -Dstrcmpi=strcasecmp
#BASEFLAGS =

CCOPT = $(CCO) $(CCOPT_ARCH) -fno-exceptions -fno-rtti -DNDEBUG
#CCOPT = $(CCO) $(CCOPT_ARCH) -DNDEBUG -D_DEBUG


INCLUDEDIRS= -I$(SRCDIR) -I$(METADIR) -I$(SDKSRC)/engine -I$(SDKSRC)/common -I$(SDKSRC)/pm_shared -I$(SDKSRC)/dlls -I$(SDKSRC) $(EXTRA_INCLUDEDIRS)

CFLAGS=$(BASEFLAGS) -Wall -Wno-unknown-pragmas
#CFLAGS=$(BASEFLAGS)

ODEF = -DOPT_TYPE=\"optimized\"
#ODEF =
CFLAGS:=$(CCOPT) $(CFLAGS) $(ODEF)

DO_CC_LINUX=$(CC_LINUX) $(CFLAGS) -fPIC $(INCLUDEDIRS) -DLINUX -o $@ -c $<
#DO_CC_LINUX=$(CC_LINUX) $(CFLAGS) -fPIC $(INCLUDEDIRS) -DLINUX -D__linux__=1 -o $@ -c $<
DO_CC_WIN32=$(CC_WIN32) $(CFLAGS) $(INCLUDEDIRS) -DWIN32 -o $@ -c $<

DO_RES_WIN32=$(RES_WIN32) -I$(SRCDIR) -I$(METADIR) -i $< -O coff -o $@

#LINK_LINUX=$(CC_LINUX) $(CFLAGS) -shared -ldl -lm -static-libgcc $(OBJ_LINUX) $(EXTRA_LIBDIRS_LINUX) $(EXTRA_LIBS_LINUX) -s -o $@
LINK_LINUX=$(CC_LINUX) $(CFLAGS) -shared -ldl -lm -static-libgcc -L. $(OBJ_LINUX) $(EXTRA_LIBDIRS_LINUX) $(EXTRA_LIBS_LINUX) -s -o $@
LINK_WIN32=$(CC_WIN32) -mdll -Xlinker --add-stdcall-alias $(OBJ_WIN32) $(RES_OBJ_WIN32) $(EXTRA_LIBDIRS_WIN32) $(EXTRA_LIBS_WIN32) -s -o $@
#for debug use 2 below
#LINK_LINUX=$(CC_LINUX) $(CFLAGS) -shared -ldl -lm -static-libgcc $(OBJ_LINUX) $(EXTRA_LIBDIRS_LINUX) $(EXTRA_LIBS_LINUX) -o $@
#LINK_WIN32=$(CC_WIN32) -mdll -Xlinker --add-stdcall-alias $(OBJ_WIN32) $(RES_OBJ_WIN32) $(EXTRA_LIBDIRS_WIN32) $(EXTRA_LIBS_WIN32) -o $@

$(OBJDIR_LINUX)/%.o: $(SRCDIR)/%.cpp
	$(DO_CC_LINUX)

$(OBJDIR_WIN32)/%.o: $(SRCDIR)/%.cpp
	$(DO_CC_WIN32)

# compiling windows resource file
$(OBJDIR_WIN32)/%.o: $(SRCDIR)/%.rc $(INFOFILES)
	$(DO_RES_WIN) 

# compiling windows resource file
$(OBJDIR_WIN32)/%.o: $(SRCDIR)/%.rc
	$(DO_RES_WIN32)

default: $(DEFAULT)

$(TARGET_LINUX): $(OBJDIR_LINUX) $(OBJ_LINUX)
	$(LINK_LINUX)

$(TARGET_WIN32): $(OBJDIR_WIN32) $(OBJ_WIN32) $(RES_OBJ_WIN32)
	$(LINK_WIN32)

$(OBJDIR_LINUX):
	mkdir $@

$(OBJDIR_WIN32):
	mkdir $@

win32: $(TARGET_WIN32)

linux: $(TARGET_LINUX)
	
clean: $(CLEAN)

clean_linux:
	test -n "$(OBJDIR_LINUX)"
	-rm -f $(OBJDIR_LINUX)/*.o

clean_win32:
	-if exist $(OBJDIR_WIN32)\*.o del /q $(OBJDIR_WIN32)\*.o
	
clean_msys_w32:
	test -n "$(OBJDIR_WIN32)"
	-rm -f $(OBJDIR_WIN32)/*.o
	
clean_both: clean_linux clean_win32
