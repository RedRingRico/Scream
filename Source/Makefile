## SCREAM Dreamcast Makefile
# Compiler: Hitachi SH Cross Compiler
# Assembler: Hitachi SH Cross Assembler
# Linker: Hitachi SH Cross Linker
# Graphics library: Kamui 2
# System library: Shinobi

# N.B.
# GNU Make with the Katana SDK does not work well with this makefile, a newer
# version of GNU Make is required (download from GnuWin32)

## Ensure the Katana SDK is set
ifeq ($(strip $(KATANA_ROOT)),)
$(error "Please set the KATANA_ROOT path or run the SETPATHS.BAT file from the Katana SDK root directory")
endif

ifndef TOPDIR
export TOPDIR		:= $(shell cd .. && cd)
export TOPSRC		= $(TOPDIR)\Source
export TARGETDIR	= $(TOPDIR)\Bin\$(PLATFORM)
export OBJSDIR		= $(TOPDIR)\Obj\$(PLATFORM)\$(BUILD)
export OBJSABSOLUTE	= $(subst \,/,$(OBJSDIR))
endif

.SILENT :
.SUFFIXES : .c .src .bin .lib .elf

PROJECT := SCREAM
TARGET := $(PROJECT)
OUTFILE = $(TARGETDIR)\$(TARGET).$(EXTENSION)

PLATFORM := Dreamcast
BUILD_PLATFORM := DREAMCAST
ARCH := SH4
BITSIZE := 32
BUILD_TYPE = development

EXTENSION ?= elf

LINK_FILES = $(addprefix xlnk.,$(basename $(DEFAULT_OBJS))) $(addprefix lnk.,$(OBJS))

BUILD = Development

SOURCEDIR = Source

MAKE	:= make.exe
ELFCONV	:= elfcnv.exe
ELF2BIN	:= elf2bin.exe
DWFCONV	:= dwfcnv.exe
CC		= shc.exe
ASM		= asmsh.exe
LIBR	= lbr.exe
LINK	= lnk.exe

OPTIMISE_TYPE = size

LIBDIR	= $(KATANA_ROOT)\lib
DEFAULT_INCLUDE	= $(KATANA_ROOT)\include $(KATANA_ROOT)\include\nexgen

DEFAULT_LIBS	= shinobi sh4nlfzn dmcast Audio64 modem nt_utl

DEFAULT_OBJS = strt1.obj strt2.obj systemid.obj toc.obj sg_sec.obj \
	sg_arejp.obj sg_areus.obj sg_areec.obj sg_are00.obj sg_are01.obj \
	sg_are02.obj sg_are03.obj sg_are04.obj sg_ini.obj aip.obj zero.obj

OPTIMISE ?= 0
#LISTING
LISTING = -list="$(OBJSDIR)\$*.lst"

COPT_LINK = -g
CDEFS = _KAMUI2_ __SET4__ __SET5__ __KATANA__ _CLX_ _CLX2_ _STRICT_UNION_ _SH4 \
	NG_COMP_SHC NG_CPU_SH4 NG_LITTLE_ENDIAN NG_HW_SEGADC \
	SCREAM_PLATFORM_$(BUILD_PLATFORM) SCREAM_ARCH_$(ARCH) SCREAM_BITSIZE_$(BITSIZE) SCREAM_BUILD_$(BUILD_DEF)

COPT = -cpu=sh4 -division=cpu -endian=little -fpu=single -round=nearest \
	-pic=0 -macsave=0 -string=const -comment=nonest -sjis \
	-section=p=P,c=C,d=D,b=B -show=obj,source,expansion,w=80,l=0

COPTIMISE = -optimize=$(OPTIMISE)

CFLAGS = $(COPT) $(CDEBUG) $(DEBUGCFLAGS) $(COPT_LINK) $(LISTING) $(COPTIMISE)

## Assembly flags
debug: ASM_DEBUG = -debug=d
development: ASM_DEBUG = -debug=d
release: ASM_DEBUG =

ASM_LIST = -list="$(OBJSDIR)\$*.lst"
ASM_OPTIONS = -cpu=sh4 -endian=little -sjis -private

ASM_FLAGS = $(ASM_OPTIONS) $(ASM_DEBUG) $(ASM_LIST)

## Linker
LINK_DEBUG =
LINK_LIST = print $*.map

L_OPT_LINK = op speed

#if bin
#if opt link
LINK_EXTRA = entry SG_SEC

LINK_START = start IP(8C008000),DSGLH(8C010000)
LINK_FORM = form a
LINK_OPTS = align_section udfcheck elf

LINK_FLAGS = $(LINK_OPTS) $(LINK_DEBUG)

development: BUILD = Development
development: BUILD_TYPE = development
development: BUILD_DEF = DEVELOPMENT
development: COPT_LINK = -g
development: LINK = lnk.exe -noecho
development: CDEBUG = -debug
development: TARGET := $(TARGET)_DEV
development: DEFAULT_LIBS += kamui2 libcross sh4nlfzn nexgen
development: OPTIMISE := 0
development: OPTIMISE_LINK := 0
development: CDEFS += DEVELOPMENT
development: LINK_DEBUG = debug
development: $(TARGET)

debug: BUILD = Debug
debug: BUILD_TYPE = debug
debug: BUILD_DEF = DEBUG
debug: COPT_LINK =
debug: LINK = lnk.exe -noecho
debug: CDEBUG = -debug
debug: TARGET := $(TARGET)_DEBUG
debug: DEFAULT_LIBS += kamui2_d libcross sh4nlfzn nexgend
debug: OPTIMISE := 0
debug: OPTIMISE_LINK := 0
debug: CDEFS += DEBUG NG_DEBUG
debug: LINK_DEBUG = debug
debug: $(TARGET)

release: BUILD = Release
release: BUILD_TYPE = release
release: BUILD_DEF = RELEASE
release: LINK = optlnksh.exe -op=st,sy,sam,b
release: COPT_LINK = -g
release: CDEBUG = -nodebug
release: EXTENSION = BIN
release: TARGET := 1ST_READ
release: DEFAULT_LIBS += kamui2 nexgen
release: OPTIMISE := 1
release: OPTIMISE_LINK := 1
release: CDEFS += RELEASE
release: LINK_DEBUG =
release: $(TARGET)

LINK_SCRIPT = "$(OBJSDIR)\lnk.sub"
TRIM_QUOTES = ~

ifneq ($(OBJSABSOLUTE), $(CURDIR))

VERSIONINFO:
	-mkdir Headers
	gitversion.bat Headers\GitVersion.h SCREAM

CLEAR:
	@cls

TARGETDIR:
	-mkdir $(TARGETDIR) 2> nul:

OBJSDIR:
	-mkdir $(OBJSDIR) 2> nul:

CFILES	:= $(foreach dir,$(SOURCEDIR),$(notdir $(wildcard $(dir)/*.c)))
ASMFILES := $(foreach dir,$(SOURCEDIR),$(notdir $(wildcard $(dir)/*.src)))
ICFILES := $(foreach dir,$(SOURCEDIR),$(notdir $(wildcard $(dir)/*.ic)))
export VPATH := $(foreach dir,$(SOURCEDIR),$(CURDIR)/$(dir))
export OBJS := $(CFILES:.c=.obj) $(ASMFILES:.src=_src.obj) $(ICFILES:.ic=_ic.obj)

# Target re-direction to make in the OBJSDIR directory
$(TARGET): CLEAR OBJSDIR TARGETDIR VERSIONINFO
	$(MAKE) -C $(OBJSDIR) -f "$(TOPSRC)\Makefile" OPTIMISE=$(OPTIMISE) \
		OPTIMISE_LINK=$(OPTIMISE_LINK) EXTENSION=$(EXTENSION) $(BUILD_TYPE)
else

# Clear the linker script
LINK_SCRIPT:
	echo. > "$(OBJSDIR)\lnk.sub"

# Target linking
$(TARGET): $(OBJS) LINK_SCRIPT $(LINK_FILES)
	echo $(LINK_START) >> "$(OBJSDIR)\lnk.sub"
	echo $(LINK_FORM) >> "$(OBJSDIR)\lnk.sub"
ifdef LINK_EXTRA
	echo $(LINK_EXTRA) >> "$(OBJSDIR)\lnk.sub"
endif
ifdef LINK_LIST
	echo $(LINK_LIST) >> "$(OBJSDIR)\lnk.sub"
endif
	for %%x in ($(LINK_FLAGS)) do echo %%x >> "$(OBJSDIR)\lnk.sub"
	for %%x in ($(SYSTEM_LIBS) $(DEFAULT_LIBS)) do echo library $(LIBDIR)\%%x.lib >> "$(OBJSDIR)\lnk.sub"
	echo output $(TARGETDIR)\$(TARGET).elf >> "$(OBJSDIR)\lnk.sub"
	echo exit >> "$(OBJSDIR)\lnk.sub"
	$(LINK) -sub="$(OBJSDIR)\lnk.sub"
ifeq ($(EXTENSION),BIN)
	$(ELF2BIN) -s 8C010000 "$(TARGETDIR)\$(TARGET).elf" $(OUTFILE)
endif
	echo Created $(OUTFILE)
# Old link command was $(LINK) -noecho -sub="$(OBJSDIR)\lnk.sub"

# C file compilation
%.obj: %.c
	echo Compiling $< ...
	echo $(CFLAGS) > "$(OBJSDIR)\shc.sub"
	for %%x in ($(OPTIMISE_TYPE)) do echo -%%x >> "$(OBJSDIR)\shc.sub"
	echo -define= >> "$(OBJSDIR)\shc.sub"
	for %%x in ($(CDEFS) $(EXTRADEFS)) do echo %%$(TRIM_QUOTES)x, >> "$(OBJSDIR)\shc.sub"
	echo __SHC__ -include= >> "$(OBJSDIR)\shc.sub"
	for %%x in ($(DEFAULT_INCLUDE)) do echo %%x, >> "$(OBJSDIR)\shc.sub"
	echo $(TOPSRC)\Headers >> "$(OBJSDIR)\shc.sub"
ifeq ($(OPTIMISE),0)
	$(CC) $< -ob="$(OBJSDIR)\$*.o" -sub="$(OBJSDIR)\shc.sub"
	$(DWFCONV) -sysrof="$(OBJSDIR)\$*.obj" "$(OBJSDIR)\$*.o
else
	$(CC) $< -ob="$(OBJSDIR)\$*.obj" -sub="$(OBJSDIR)\shc.sub"
endif


# Inline C compilation (files with inline assembly)
%_ic.obj: %.ic
	echo Compliing $< ...
	echo $(CFLAGS) -code=asmcode > "$(OBJSDIR)\shc.sub"
	for %%x in ($(OPTIMISE_TYPE)) do echo -%%x >> "$(OBJSDIR)\shc.sub"
	echo -define= >> "$(OBJSDIR)\shc.sub"
	for %%x in ($(CDEFS) $(EXTRADEFS)) do echo %%$(TRIM_QUOTES)x, >> "$(OBJSDIR)\shc.sub"
	echo __SHC__ -include= >> "$(OBJSDIR)\shc.sub"
	for %%x in ($(DEFAULT_INCLUDE)) do echo %%x, >> "$(OBJSDIR)\shc.sub"
	echo $(TOPSRC)\Headers >> "$(OBJSDIR)\shc.sub"
	$(CC) $< -ob="$(OBJSDIR)\$*.src" -sub="$(OBJSDIR)\shc.sub"
	$(ASM) $(OBJSDIR)\$*.src $(ASM_FLAGS) -O="$@"

# Assembly file compilation
$_src.obj: $.src
	echo Assembling $<
	$(ASM) $< $(ASM_FLAGS) -O="$@"

lnk.% :
	echo input $* >> "$(OBJSDIR)\lnk.sub"
xlnk.% :
	echo input $(LIBDIR)\$*.obj >> "$(OBJSDIR)\lnk.sub"


endif

clean:
	-rd /s /q "$(TOPDIR)\Obj" >nul: 2>nul:
