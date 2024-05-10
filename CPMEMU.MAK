PROJ	=CPMEMU
DEBUG	=1
CC	=cl
AS	=qcl
CFLAGS_G    = /AL /DMSC /G2
CFLAGS_D	= /Zd /Od 
CFLAGS_R	= /Ox /DNDEBUG 
CFLAGS  =$(CFLAGS_G) $(CFLAGS_R)
AFLAGS_G	= /Cx /W1 /P2 
AFLAGS_D	= /Zi /l 
AFLAGS_R	= /DNDEBUG 
AFLAGS  =$(AFLAGS_G) $(AFLAGS_R)
LFLAGS_G	= /CP:0xfff /NOI /SE:0x80 /ST:0x3800 
LFLAGS_D	= /CO 
LFLAGS_R	= 
LFLAGS  =$(LFLAGS_G) $(LFLAGS_R)
RUNFLAGS	=
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

cpmemu.obj:	cpmemu.c $(H)

cpmbdos.obj:	cpmbdos.c $(H)

cpmbios.obj:	cpmbios.c $(H)

cpmemuin.obj:	cpmemuin.c $(H)

cpmglob.obj:	cpmglob.c $(H)

mem_mmu.obj:	mem_mmu.c $(H)

simz80.obj:	simz80.c $(H)

$(PROJ).EXE:	cpmemu.obj cpmbdos.obj cpmbios.obj cpmemuin.obj cpmglob.obj mem_mmu.obj \
	simz80.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
cpmemu.obj +
cpmbdos.obj +
cpmbios.obj +
cpmemuin.obj +
cpmglob.obj +
mem_mmu.obj +
simz80.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
        link $(LFLAGS) @$(PROJ).crf

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

