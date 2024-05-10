/************************************************************************/
/*                                                                      */
/*             CP/M Hardware Emulator Card Support Program              */
/*                         CPMEMU.C Ver 1.60                            */
/*                 Copyright (c) By C.J.Chen NTUEE 1988                 */
/*                         All Right Reserved                           */
/*                                                                      */
/************************************************************************/

#ifndef _CPMEMU_H

#define _CPMEMU_H

#define   interrupt
#define   cdecl

#define   PROGRAM_NAME   "\nCP/M 2.2 Emulator and Z80 Instruction Simulator Ver 1.60\n"
#define   COPYRIGHT      "Z80 Simulator by Frank D. Cringle from his great YAZE.\n"
#define   COPYRIGHT1     "CP/M 2.2 Emulator by David Chen, 1988,1989,2001.\n"

#define   MAXFILE         20
#define   NULLSIZE        0x40

#define   GotoPC
#define   ResetZ80 resetz80()
#define   ReturnZ80 

#define   REGA            (ram+0XFF83)
#define   REGB            (ram+0XFF85)
#define   REGC            (ram+0XFF84)
#define   REGD            (ram+0XFF87)
#define   REGE            (ram+0XFF86)
#define   REGH            (ram+0XFF89)
#define   REGL            (ram+0XFF88)

#define   REGAF           (ram+0XFF82)
#define   REGBC           (ram+0XFF84)
#define   REGDE           (ram+0XFF86)
#define   REGHL           (ram+0XFF88)
#define   REGIX           (ram+0XFF8A)
#define   REGIY           (ram+0XFF8C)
#define   REGSP           (ram+0XFF8E)
#define   REGIP           (ram+0XFF90)
#define   BIOSCODE        (ram+0XFF92)
#define   BDOSCALL        (ram+0XFF81)
#define   EOP             (ram+0XFF80)
/*   CP/M SYSTEM FILE CONTROL BLOCK  */
#define   FCBDN           (ram+0X0000)
#define   FCBFN           (ram+0X0001)
#define   FCBFT           (ram+0X0009)
#define   FCBRL           (ram+0X000C)
#define   FCBRC           (ram+0X000F)
#define   FCBCR           (ram+0X0020)
#define   FCBLN           (ram+0X0021)


typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned long  UINT32;
typedef char           SINT8;
typedef short          SINT16;
typedef long           SINT32;

struct ddregs {
	UINT16 bc;
	UINT16 de;
	UINT16 hl;
};

/* CPMEMU.C */
void checknullptr(void);
void quit(void);
void resetz80(void);
void waitz80(void);
void fillfcb(char *,char *);
void store_stack_segment(void);

/* CPMBDOS.C */
void initialbdos(void);
void cpmbdos(void);

/* CPMBIOS.C */
void initialbios(void);
void cpmbios(void);

/* CPMEMUIN.C */
void handle_ctrl_c(void);

/* SIMZ80.C */
UINT32 simz80(UINT16);
UINT16 GetPC(void);
void SetPC(UINT16);

#endif /* #ifdef _CPMEMU_H */
