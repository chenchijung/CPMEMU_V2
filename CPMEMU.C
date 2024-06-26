/************************************************************************/
/*                                                                      */
/*             CP/M Hardware Emulator Card Support Program              */
/*                         CPMEMU.C Ver 1.51                            */
/*                 Copyright (c) By C.J.Chen NTUEE 1988                 */
/*                         All Right Reserved                           */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <bios.h>
#include <dos.h>
#include <stdlib.h>
#include <direct.h>
#include <process.h>
#include <setjmp.h>
#include <string.h>
#include <memory.h>
#include <conio.h>

#ifdef DJGPP
  #include <unistd.h>
#endif

#include "cpmemu.h"
#include "cpmglob.h"


static char *hexptr;
static UINT8 halt;
static UINT8 doscommand=1;

static char *nullptr = NULL;
static char nulldata[NULLSIZE];

static FILE *subfile=NULL;
static FILE *fp;

static UINT8 checknum;

static char submitstring[128];
static char *submitptr[10];

static jmp_buf cold_start;
/*----------------------------------------------------------------------*/
void printtitle(void)
{
    printf(PROGRAM_NAME);
    printf(COPYRIGHT);
    printf(COPYRIGHT1);
    printf("TPA Area: 0100H - FDFFH\n");
    return;
}
/*----------------------------------------------------------------------*/
char getchfromcpmhex(void)
{
    char ch;
    ch = *hexptr++;
    return(ch);
}
/*----------------------------------------------------------------------*/
char fgetc1()
{
    char tmp;

	tmp = getchfromcpmhex();
	if (tmp > '9') tmp -= 55;
	else tmp -= 48;
    return(tmp);
}
/*----------------------------------------------------------------------*/
unsigned char readbyte()
{
	UINT8 result;

	result = (UINT8)fgetc1() * 16;
	result += (UINT8)fgetc1();
	checknum += result;
	return(result);
}
/*----------------------------------------------------------------------*/
int readline(void)
{
	char ch;
	UINT16 addr=0;
	unsigned char data_type;
	unsigned char data_number;
	unsigned char i,tmp;
	unsigned char *ptr;

	checknum = 0;
	if ((ch = getchfromcpmhex()) == EOF) {
		printf("\ninvalid EOF \n"); quit();
	}
	if (ch != ':') { 
		printf("\nInvalid HEX file!\n");
		quit();
	}
	if (( data_number = readbyte() ) == 0) return(EOF);
	addr = readbyte();
	tmp = readbyte() ;
	addr = addr * 256 + tmp;
	if (( data_type = readbyte() ) == 1) return(EOF);
	else if (data_type != 0) { 
		printf("\nInvalid data type char.\n");
		quit();
	}
	ptr = (char *)(&ram[addr]);
	for (i=1 ; i <= data_number ; i++) *ptr++ = readbyte();
	readbyte();             /* read checknumber */
	getchfromcpmhex();      /* read Line Feed char */
	if (checknum != 0) { printf("\nCheck Sum error!\n"); quit(); }
	return(0);
}
/*----------------------------------------------------------------------*/
void loadcpmhex(void)
{
	int result;

	hexptr = (char *)cpmhex;
	do {
            result = readline();
	} while (result != EOF);
}
/*----------------------------------------------------------------------*/
UINT8 loadcom(char *filename)
{
	char *addr = (char *)(&ram[0x0100]);
	char buffer[128];
	char *ptr1;
	unsigned char i;
	unsigned filelen = 0;
	unsigned loadlen = 0;

	if ((fp = fopen(filename,"rb"))==NULL) {
	        printf("Z80 Execution File %s not found\n", filename);
        	return(1);
	}
	fseek(fp,0L,SEEK_END);
	filelen = (unsigned)ftell(fp);
	fseek(fp,0L,SEEK_SET);
	do {
		ptr1 = buffer;
        	fread(buffer,128,1,fp);
	        for (i = 0 ; i < 128 ; i++) *addr++ = *ptr1++;
        	loadlen += 128;
	} while (loadlen < filelen);
	fclose(fp);
	return(0);
}
/*----------------------------------------------------------------------*/
void clearmem(void)
{
	long *ptr;
	UINT16 addr;

	GotoPC;
	ptr = (long *)ram;
	*ptr = 0x55aa55aa;
	ptr++;
	if (*(--ptr) != 0x55aa55aa ) {
	        printf("\n\7Z80 Card Bad or not Implimented!\n");
        	quit();
	}
	for (addr = 0 ; addr <= 0x4000 ; addr++) *ptr++=0L;
        return;
}
/*----------------------------------------------------------------------*/
void fillfcb(char *addr,char *ptr)
{
	char count = 0;
	char *ptr3;
	char i;

	ptr3 = addr;
	*ptr3++ = 0x00;
	for (i = 0 ; i < 11 ; i++) *ptr3++ = 0x20;

	while (*ptr == ' ') ptr++;
	if (*(ptr+1) == ':') {*addr = *ptr - 'A' + 1; ptr += 2; }
	addr++;
	while (*ptr != '.' && *ptr != 0x00 && count < 8)
		{*addr++ = *ptr++; count++; }
	if (*ptr != 0x00) {
		 while (count < 8) { addr++; count++; }
	        if (*ptr=='.') {
        	        ptr++;
                	count = 0;
	                while(*ptr != 0x00 && count < 3) {
        	                *addr++ = *ptr++;
                	        count++;
                	}
        	}
    	}
        return;
}
/*----------------------------------------------------------------------*/
void submit(char *ptr1)
{
	char *ptr2;
	char filename[20];
	UINT8 submitcount = 1;

	ptr1 += 6;
	while (*ptr1 == ' ') ptr1++;
	ptr2 = filename;
	while (*ptr1 != ' '&& *ptr1 != '.'&& *ptr1 != 0x00) *ptr2++ = *ptr1++;
	switch (*ptr1) {
		case '.': while (*ptr1 != ' ' && *ptr1 != 0x00)
				 *ptr2++ = *ptr1++; *ptr2=0x00; break;
		case ' ': strcpy(ptr2,".SUB"); break;
		case  0 : strcpy(ptr2,".SUB"); break;
	}
	if ((subfile = fopen(filename,"r"))==NULL) printf("File not found\n");
	else if (*ptr1 != 0x00) {
	        ptr1++;
        	strcpy(submitstring,ptr1);
	        ptr1 = submitstring;
	        while (submitcount < 10) {
    		    	if (*ptr1 == 0x00) break;
			submitptr[submitcount++] = ptr1;
			while (*ptr1 != ' ' && *ptr1 != 0x00) ptr1++;
			if (*ptr1 == ' ') *ptr1++ = 0x00;
		}
	}
        return;
}
/*----------------------------------------------------------------------*/
void debug(char *ptr1)
{
	if (strnicmp(ptr1,"DEBUG ON",8) == 0) {
		printf("Debug is set on\n");
	        DebugFlag = 1;
	        lastcall = 0xff;
	        if (lpt == NULL) {
	                lpt = fopen("btrace.dat","w");
	                fputs("Times  Z80PC    Z80DE    Z80DMA   FUNCTION\n"
				,lpt);
	                fputs("-----  -----    -----    ------   --------\n"
				,lpt);
	        }
	}
	else if (strnicmp(ptr1,"DEBUG OFF",9) == 0) {
	        printf("Debug is set off\n");
	        DebugFlag = 0;
	        if (lpt != NULL) {
	                if (lastcall != 0xFF) {
	                        fprintf(lpt,"% 5d  %s%s\n", repeats, debugmess1
					, debugmess2);
	                }
	                fputs("--------- END OF BDOS TRACE TABLE -------- "
				,lpt);
	                fclose(lpt);
	                lpt = NULL;
	        }
	}
	else {
		if (DebugFlag == 0) printf("Debug is off\n");
	        else printf("Debug is on\n");
	}
        return;
}
/*----------------------------------------------------------------------*/
void z80debug(char *ptr1)
{
	if (strnicmp(ptr1,"Z80DBG ON",9) == 0) {
		printf("Z80 Debug is set on\n");
	        Z80DebugFlag = 1;
	        if (Z80Trace == NULL) {
	            Z80Trace = fopen("z80trace.dat","w");
	            fputs("--------- Start OF Z80 TRACE TABLE --------\n",Z80Trace);

	        }
	}
	else if (strnicmp(ptr1,"Z80DBG OFF",10) == 0) {
	        printf("Z80 Debug is set off\n");
	        Z80DebugFlag = 0;
	        if (Z80Trace != NULL) {
	                fputs("--------- END OF Z80 TRACE TABLE --------\n",Z80Trace);
	                fclose(Z80Trace);
	                Z80Trace = NULL;
	        }
	}
	else {
		if (Z80DebugFlag == 0) printf("Z80 Debug is off\n");
	        else printf("Z80 Debug is on\n");
	}
        return;
}
/*----------------------------------------------------------------------*/
void CheckDosCommand(char *ptr1)
{
	doscommand = 1;
	if (strnicmp(ptr1,"EXIT",4) == 0)         quit();
	else if (strnicmp(ptr1,"QUIT",4) == 0)    quit();
	else if (strnicmp(ptr1,"DIR",3) == 0)     system(ptr1);
	else if (strnicmp(ptr1,"EDIT ",5) == 0)   system(ptr1);
	else if (strnicmp(ptr1,"COPY ",5) == 0)   system(ptr1);
	else if (strnicmp(ptr1,"REN ",4) == 0)    system(ptr1);
	else if (strnicmp(ptr1,"RENAME ",7) == 0) system(ptr1);
	else if (strnicmp(ptr1,"CD ",3) == 0)     system(ptr1);
	else if (strnicmp(ptr1,"CD",2) == 0 && strlen(ptr1) == 2) system(ptr1);
	else if (strnicmp(ptr1,"DEL ",4) == 0)    system(ptr1);
	else if (strnicmp(ptr1,"TYPE ",5) == 0)   system(ptr1);
	else if (strnicmp(ptr1,"VER",3) == 0 && strlen(ptr1) == 3)
		printtitle();
	else if (strnicmp(ptr1,"SUBMIT ",7) == 0) submit(ptr1);
	else if (strnicmp(ptr1,"!",1) == 0)       system(++ptr1);
	else if (strnicmp(ptr1,"DEBUG",5) == 0)   debug(ptr1);
	else if (strnicmp(ptr1,"Z80DBG",6) == 0)  z80debug(ptr1);
	else if (strnicmp(ptr1,"COLD!",5) == 0) {
		clearmem();
	        loadcpmhex();
	        longjmp(cold_start,0);
	}
	else if (strlen(ptr1) == 0);
	else if (strlen(ptr1) == 2 && ptr1[1] == ':') system(ptr1);
	else if (*ptr1 == ';');
	else doscommand = 0;
        return;
}
/*----------------------------------------------------------------------*/
void upcase(char *ptr1)
{
	while (*ptr1 != 0x00) { 
		if ( *ptr1 > 0x60 && *ptr1 < 0x7b) *ptr1 -= 32;
		ptr1++;
	}
        return;
}
/*----------------------------------------------------------------------*/
void getstring(char *filename)
{
	char *ptr1,*ptr2;
	char tempname[129];
	unsigned char ch,tmp;
	int i;

	ptr1 = filename;
	if (subfile != NULL) {
	        if (feof(subfile) != 0) {
	                fclose(subfile);
        	        subfile = NULL;
	                strncpy(filename,"\n\0",2);
            	}
	        else {
                	do {
	                        ch = fgetc(subfile);
        	                if (ch == 0x0a) ch=0x00;
                	        else if (ch == 0xff) {
                                	ch = 0x00;
	                                fclose(subfile);
        	                        subfile = NULL;
	                                for (i = 1 ; i < 10 ;
						submitptr[i++] = NULL);
	                                *submitstring = 0x00;
				}
	                        else if (ch == '$') {
	                                tmp = getc(subfile);
         	                      	if (!isdigit(tmp)) ungetc(tmp,subfile);
	        	        	else {
                                        	ptr2 = submitptr[tmp-'0'];
	                                        if (ptr2 != NULL) {
        	                                	while (*ptr2 != 0x00)
							    *ptr1++ = *ptr2++;
                        	                	ch = *(--ptr2);
	                                        	ptr1--;
        	                                }
	                                        else ch = getc(subfile);
        	                        }
                	        }
	                        *ptr1++ = ch;
			} while( ch != 0x00 );
	                printf("%s\n",filename);
                }
	}
	else {
		/* gets(filename); */
		tempname[0] = 127;
		strncpy( filename , cgets(tempname) ,127);
		printf("\n");
	}
        return;
}
/*----------------------------------------------------------------------*/
void getcommand(void)
{
	char filename[128], first_file[20], second_name[30], third_name[30];
	char *ptr1,*ptr2,*ptr4;
	char *ptr3;
	UINT8 i,count=0;
	char dir_now[60];

	do {
        	do {
        		getcwd(dir_now,60);
	                printf("\nZ80 %s>",dir_now);
        	        getstring(filename);
                	ptr1 = filename;
	                while (*ptr1 == ' ') ptr1++;
	                CheckDosCommand(ptr1);
        	        upcase(ptr1);
		} while (doscommand == 1);

	        ptr2 = first_file;
	        for (i = 0 ; i < 20 ; i++) first_file[i] = 0x00;
		for (i = 0 ; i < 30 ; i++)
			second_name[i] = third_name[i] = 0x00;
        	while (!isalpha(*ptr1) && *ptr1 != 0x00) ptr1++;
	        while (*ptr1 != ' '&& *ptr1 != '.'&& *ptr1 != 0x00
			 && *ptr1 != '\n') *ptr2++ = *ptr1++;
		switch (*ptr1) {
		        case '.' : while (*ptr1 != ' '&& *ptr1 != 0x00)
					 *ptr2++ = *ptr1++;
				   break;
            		case '\n':
		        case ' ' : strncpy(ptr2,".COM",4);break;
		        case  0  : strncpy(ptr2,".COM",4);break;
	        }
	} while (loadcom(first_file) != 0);

	ptr3 = (char *)(&ram[0x0080]);
	*ptr3++ = 0x00;
	*ptr3++ = 0x00;

	if (*ptr1 != 0x00) {
	        ptr2 = ptr1;
        	ptr3 = (char *)(&ram[0x0081]);
	        while (*ptr2 != 0x00) { *ptr3++ = *ptr2++; count++; }
	        *ptr3++ = 0x0D;
        	*ptr3 = 0x00;
	        ptr3 = (char *)(&ram[0x0080]);
	        *ptr3 = count;

        	ptr2 = ptr1;
	        ptr4 = second_name;
        	*second_name = 0x00;         /* clear name */
	        while (*ptr2 == ' ') ptr2++;
	        while (*ptr2 != 0x00 && *ptr2 != ' ') *ptr4++ = *ptr2++;
		*third_name = *ptr4 = 0x00;
	        if (*ptr2 == ' ') {
	                ptr4 = third_name;
        	        while (*ptr2 == ' ') ptr2++;
                	while (*ptr2 != 0x00) {*ptr4++ = *ptr2++;}
	                *ptr4 = 0x00;
        	}
	        fillfcb((char *)(&ram[0x005c]),second_name);
	        fillfcb((char *)(&ram[0x006c]),third_name);
	} 

	while ((i = _bios_keybrd(_KEYBRD_READY)) != 0)
	        _bios_keybrd(_KEYBRD_READ);    /* clear keybrd buffer */
	ResetZ80;        /* pull reset line to low */
	ReturnZ80;        /* set z80 to raad RAM */
	ResetZ80;   /* pull reset line to high ,z80 begin running*/
	waitz80();
	ReturnZ80;
	waitz80();
	halt = *eop = 0;
        return;
}
/*----------------------------------------------------------------------*/
void initpointer(void)               /* initial pointers value */
{
	rega = (UINT8 *)REGA;  /* initial 8 bits register pointers */
	regb = (UINT8 *)REGB;
	regc = (UINT8 *)REGC;
	regd = (UINT8 *)REGD;
	rege = (UINT8 *)REGE;
	regh = (UINT8 *)REGH;
	regl = (UINT8 *)REGL;

	regaf = (UINT16 *)REGAF; /* initial 16 bits register pointers */
	regbc = (UINT16 *)REGBC;
	regde = (UINT16 *)REGDE;
	reghl = (UINT16 *)REGHL;
	regix = (UINT16 *)REGIX;
	regiy = (UINT16 *)REGIY;
	regip = (UINT16 *)REGIP;
	regsp = (UINT16 *)REGSP;

	eop = (UINT8 *)EOP;
	bioscode = (UINT8 *)BIOSCODE;
	bdoscall = (UINT8 *)BDOSCALL;

	fcbdn = (UINT8 *)FCBDN; /* initial FCB block pointers */
	fcbfn = (UINT8 *)FCBFN;
	fcbft = (UINT8 *)FCBFT;
	fcbrl = (UINT8 *)FCBRL;
	fcbrc = (UINT8 *)FCBRC;
	fcbcr = (UINT8 *)FCBCR;
	fcbln = (UINT8 *)FCBLN;
        return;
}
/*----------------------------------------------------------------------*/
void set_ctrl_c(void)
{
#ifdef MSC	
	void *ptr1;

	ptr1 = (void *)_dos_getvect(35);
	ctrl_c_seg = FP_SEG(ptr1);
	ctrl_c_offset = FP_OFF(ptr1);
	_dos_setvect(35, (void *)handle_ctrl_c);
#endif

/*
	union REGS regs;
	struct SREGS sregs;

	regs.h.al = 35;
	regs.h.ah = 0x35;
	int86x(0x21,&regs,&regs,&sregs);
	ctrl_c_offset = (void *)regs.x.bx;

	regs.h.al = 35;
	regs.h.ah = 0x25;
	sregs.ds = 0;
	regs.x.dx = (UINT32)handle_ctrl_c;
	int86x(0x21,&regs,&regs,&sregs);
*/
/*
	ctrl_c_offset = _dos_getvect(35)
	_dos_setvect(35,handle_ctrl_c);
*/
    return;
}
/*----------------------------------------------------------------------*/
void initial(void)
{
	set_ctrl_c();
	store_stack_segment();
	initpointer();
	initialbdos();
	initialbios();
	printtitle();
	_bios_printer(_PRINTER_INIT,0,0);

	subfile = fopen("$$$.SUB","r");
    return;
}
/*----------------------------------------------------------------------*/
void waitz80(void)
{
#if 0 /* Hardware */
 	register test,test1;

	_enable();
	do {
	        test = (unsigned char)inp(STATUS_PORT);
        	if ((test & 0x02) == 0x00) {
			printf("Program halted!\n"); halt = 1;
		}
		test1 = _bios_keybrd(_KEYBRD_READY);
		if ( test1 != 0 ) {
			if (test1 == 0xffff) {
				puts("^Break");
				GotoPC;
				longjmp(ctrl_c,0);
			}
			else if ((test1 & 0x00ff) == 0x0003 );
			else _bios_keybrd(_KEYBRD_READ);
		}
	} while (((test & 0x01) != 0) | halt == 1 ) ;
#endif

 	simz80(GetPC());		/* call z80 engine */
     return;
}
/*----------------------------------------------------------------------*/
void resetz80(void)
{
	SetPC(0x0000);	/* initialize pc */
	/* There should be some other registers to be initialized */
    return;
}
/*----------------------------------------------------------------------*/

void quit(void)
{
#ifdef MSC
	union REGS regs;
	struct SREGS sregs;
#endif

	if (DebugFlag ==  1 && lpt !=  NULL) {
		if (lastcall  !=  0xFF) {
		        fprintf(lpt,"% 5d  %s%s\n", repeats, debugmess1
				, debugmess2);
		}
		fputs("----- END OF BDOS TRACE TABLE ----- ",lpt);
		fclose(lpt);
	}
#ifdef MSC
	regs.h.al = 35;
	regs.h.ah = 0x25;
	sregs.ds = ctrl_c_seg;
	regs.x.dx = ctrl_c_offset;
	int86x(0x21,&regs,&regs,&sregs);
#endif
/*
	_dos_setvect(35,ctrl_c_offset);
*/
	checknullptr();
	exit(0);
    return;
}
/*----------------------------------------------------------------------*/
void closeall(void)
{
	UINT8 fcbptr;

	for (fcbptr = 0 ; fcbptr < MAXFILE ; fcbptr++) {
	        if (fcbfile[fcbptr] != NULL) fclose(fcbfile[fcbptr]);
        	fcbfile[fcbptr] = NULL;
	        fcbused[fcbptr][0] = 0x00;
	        fcbfilelen[fcbptr] = 0x0000;
	}
    return;
}
/*----------------------------------------------------------------------*/
void store_stack_segment(void)
{
#ifdef MSC
    struct SREGS sregs;

    segread(&sregs);
    stack_segment = sregs.ss;
#endif
    return;
}/*----------------------------------------------------------------------*/
void printnull(void)
{
	char *temp;
	char linebuf[17];
        UINT8 i;

	printf("NULL Pointer value:\n");
	for ( temp = nullptr, i = 0 ; temp < (nullptr + NULLSIZE) ; temp++) {
		linebuf[i++] = (*temp >=  32) ? *temp : '.';
        	printf( "%02X ", *temp);
	        if (i == 16) {
	                linebuf[i] = '\0';
        	        printf("| %16s\n",linebuf);
                	i = 0;
	        }
	}
        return;
}
/*----------------------------------------------------------------------*/
void savenullptr(void)
{
	char *tmp;
        UINT8 i;

        tmp = nullptr;
        for( i = 0 ; i < NULLSIZE ; i++) nulldata[i] = *tmp++;
        return;
}
/*----------------------------------------------------------------------*/
void checknullptr(void)
{
	char *tmp;
	UINT8 i,flag;

    tmp = nullptr;
    flag = 0;
    for(i = 0 ; i < NULLSIZE ; i++) {
        if (nulldata[i]  !=  *tmp++) flag = 1;
    }
	if (flag ==  1) {
        printf("\nNull Pointer assignment error!\7\n");
        printnull();
        tmp = nullptr;
        for( i = 0 ; i < NULLSIZE ; i++) *tmp++ = nulldata[i];
    }
    return;
}
/*----------------------------------------------------------------------*/
int main(void)
{
	UINT8 tmp;
	
	savenullptr();
	initial();
	setjmp(cold_start);
	while(1) {
		clearmem();
		setjmp(ctrl_c);
	    checknullptr();
      	if (subfile  !=  NULL) {
	        fclose(subfile);
            subfile = NULL;
           	for (tmp = 1 ; tmp < 10 ; submitptr[tmp++] = NULL);
            	*submitstring = 0x00;
        }
	    while (1)
	    {
           	GotoPC;
          	loadcpmhex();
	       	closeall();
           	getcommand();
           	dmaaddr = 0x0080;
          	while (*eop == 0 && halt == 0) {
          		waitz80();
	        	if (*bdoscall != 0) cpmbdos();
        	    else cpmbios();
            }
	        halt = 0;
	        checknullptr();
        }
	}
	return 0;
}

