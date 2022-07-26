/*--------------------------------------------------------------------------
LMS7002_REGx51.h (Ver 1.0)

Header file for the LIME MCU chip.
Copyright (c) 2013 LIME, Inc.  All rights reserved.
--------------------------------------------------------------------------*/

/*------------------------------------------------
Make sure that this file gets included only once.
------------------------------------------------*/
#ifndef __LMS7002_H__
#define __LMS7002_H__

/*------------------------------------------------
Byte Registers
------------------------------------------------*/
sfr P0    = 0x80;
sfr SP    = 0x81;
sfr DPL0  = 0x82; 
sfr DPH0  = 0x83; 
sfr DPL1  = 0x84; 
sfr DPH1  = 0x85;
sfr DPS   = 0x86; 
sfr PCON  = 0x87; 
sfr TCON  = 0x88; 
sfr TMOD  = 0x89;
sfr TL0   = 0x8A;
sfr TL1   = 0x8B;
sfr TH0   = 0x8C;
sfr TH1   = 0x8D;
sfr PMSR  = 0x8E; 
sfr P1    = 0x90; 
sfr DIR1	= 0x91; 
sfr SCON  = 0x98;
sfr SBUF  = 0x99;
sfr P2    = 0xA0;
sfr DIR2	= 0xA1; 
sfr DIR0	= 0xA2; 
sfr IEN0    = 0xA8; 
sfr IEN1	= 0xA9; 
sfr EECTRL	= 0xB0; 
sfr EEDATA	= 0xB1; 
sfr IP0     = 0xB8; 
sfr IP1   = 0xB9; 
sfr USR2	= 0xBF; 
sfr IRCON	= 0xC0; 
sfr T2CON	= 0xC8; 
sfr RCAP2L	= 0xCA; 
sfr RCAP2H	= 0xCB; 
sfr TL2 = 0xCC; 
sfr TH2 = 0xCD; 
sfr PSW   = 0xD0;
sfr ACC   = 0xE0;
sfr REG0	= 0xEC; 
sfr REG1	= 0XED; 
sfr REG2	= 0xEE; 
sfr REG3	= 0xEF; 
sfr B     = 0xF0; 
sfr REG4	= 0xF4; 
sfr REG5	= 0xF5; 
sfr REG6	= 0xF6; 
sfr REG7	= 0xF7; 
sfr REG8	= 0xFC; 
sfr REG9	= 0xFD; 

/*------------------------------------------------
P0 bits 
------------------------------------------------*/
sbit P0_0 = 0x80;
sbit P0_1 = 0x81;
sbit P0_2 = 0x82;
sbit P0_3 = 0x83;
sbit P0_4 = 0x84;
sbit P0_5 = 0x85;
sbit P0_6 = 0x86;
sbit P0_7 = 0x87;

/*------------------------------------------------
TCON bits
------------------------------------------------*/
sbit IT0  = 0x88;
sbit IE0  = 0x89;
sbit IT1  = 0x8A;
sbit IE1  = 0x8B;
sbit TR0  = 0x8C;
sbit TF0  = 0x8D;
sbit TR1  = 0x8E;
sbit TF1  = 0x8F;

/*------------------------------------------------
P1 bits
------------------------------------------------*/
sbit P1_0 = 0x90;
sbit P1_1 = 0x91;
sbit P1_2 = 0x92;
sbit P1_3 = 0x93;
sbit P1_4 = 0x94;
sbit P1_5 = 0x95;
sbit P1_6 = 0x96;
sbit P1_7 = 0x97;

/*------------------------------------------------
SCON bits
------------------------------------------------*/
sbit RI   = 0x98;
sbit TI   = 0x99;
sbit RB8  = 0x9A;
sbit TB8  = 0x9B;
sbit REN  = 0x9C;
sbit SM2  = 0x9D;
sbit SM1  = 0x9E;
sbit SM0  = 0x9F;

/*------------------------------------------------
P2 bits
------------------------------------------------*/
sbit P2_0 = 0xA0;
sbit P2_1 = 0xA1;
sbit P2_2 = 0xA2;
sbit P2_3 = 0xA3;
/// reserved for SPI
sbit ucSCLK = 0xA0;
sbit ucSEN = 0xA1;
sbit ucSDIN = 0xA2;
sbit ucSDOUT = 0xA3;


/*------------------------------------------------
IEN0 bits
------------------------------------------------*/
sbit EX0  = 0xA8;       /* 1=Enable External interrupt 0 */
sbit ET0  = 0xA9;       /* 1=Enable Timer 0 interrupt */
sbit EX1  = 0xAA;       /* 1=Enable External interrupt 1 */
sbit ET1  = 0xAB;       /* 1=Enable Timer 1 interrupt */
sbit ES0  = 0xAC;       /* 1=Enable Serial port 0 interrupt */
sbit ET2  = 0xAD;		/* 1=Enable Timer 2 interrupt */
//sbit XX   = 0xAE; /* Don't Care */
sbit EA   = 0xAF;       /* 0=Disable all interrupts */

/*------------------------------------------------
EECTRL bits
------------------------------------------------*/
sbit CMD0   = 0xB0;       /* Command 0 bit */
sbit CMD1   = 0xB1;       /* Command 1 bit */
sbit CMD2   = 0xB2;       /* Command 2 bit */
sbit CMD3   = 0xB3;       /* Command 3 bit */
sbit TX_ACK = 0xB4;       /* 1= ACK Transmited to EEPROM */
sbit RX_ACK = 0xB5;	  /* 1= ACK Received from EEPROM */
sbit BUSY   = 0xB6;	  /* 1= Serial Data BUS is busy */
sbit ERROR  = 0xB7;       /* 1= Illegal Command Received */

/*------------------------------------------------
IP0 bits
------------------------------------------------*/
sbit PX0  = 0xB8; /* External iterrupt 0 priority bit */
sbit PT0  = 0xB9; /* Timer 0 iterrupt priority bit */
sbit PX1  = 0xBA; /* External iterrupt 1 priority bit */
sbit PT1  = 0xBB; /* Timer 1 iterrupt priority bit */
sbit PS0  = 0xBC; /* Serial port 0 iterrupt priority bit */
sbit PT2  = 0xBD; /* Timer 2 iterrupt priority bit */
//sbit XX   = 0xBE; /* Don't Care */
//sbit XX   = 0xBF; /* Don't Care */

/*------------------------------------------------
IRCON bits
------------------------------------------------*/
//sbit XX   = 0xCO; /* Don't Care */
//sbit XX   = 0xC1; /* Don't Care */
sbit IE2  = 0xC2; /* External iterrupt 2 flag bit */
sbit IE3  = 0xC3; /* External iterrupt 3 flag bit */
sbit IE4  = 0xC4; /* External iterrupt 4 flag bit */
sbit IE5  = 0xC5; /* External iterrupt 5 flag bit */
//sbit XX   = 0xC6; /* Don't Care */
//sbit XX   = 0xC7; /* Don't Care */

/*------------------------------------------------
T2CON bits
------------------------------------------------*/
sbit CP_RL2 = 0xC8; /* Capture/Reload Flag */
sbit C_T2	= 0xC9; /* 0/1 - Timer/Counter select */
sbit TR2	= 0xCA; /* 0/1 - Stop/Strat Timer */
sbit EXEN2  = 0xCB; /* Timer 2 enable Flag */
sbit TCLK	= 0xCC; /* Transmit Clock Flag */
sbit RCLK	= 0xCD; /* Receive Clock Flag */
sbit EXF2	= 0xCE; /* Timer 2 External Flag */
sbit TF2	= 0xCF; /* Timer 2 Overflow Flag */ 

/*------------------------------------------------
PSW bits
------------------------------------------------*/
sbit P    = 0xD0; /* Parity Flag */
//sbit XX	  = 0xD1; /* Don't Care */
sbit OV   = 0xD2; /* Overflow Flag */
sbit RS0  = 0xD3; /* Register Bank Select 0 */
sbit RS1  = 0xD4; /* Register Bank Select 0 */
//sbit XX   = 0xD5; /* Don't Care */
sbit AC   = 0xD6; /* Auxiliary Carry Flag */
sbit CY   = 0xD7; /* Carry Flag */


/*------------------------------------------------
TMOD Bit Values - Defines the bit position in the reg!!!
------------------------------------------------*/
#define T0_M0_   0x01 /* 0000 0001 = T0_M0 */
#define T0_M1_   0x02 /* 0000 0010 = T0_M1 */
#define T0_CT_   0x04 /* 0000 0100 = T0_CT */
#define T0_GATE_ 0x08 /* 0000 1000 = T0_GATE */
#define T1_M0_   0x10 /* 0001 0000 = T1_M0 */
#define T1_M1_   0x20 /* 0010 0000 = T1_M1 */
#define T1_CT_   0x40 /* 0100 0000 = T1_CT */
#define T1_GATE_ 0x80 /* 1000 0000 = T1_GATE */

#define T1_MASK_ 0xF0
#define T0_MASK_ 0x0F

/*------------------------------------------------
PCON Bit Values - Defines the bit position in the reg!!!
------------------------------------------------*/
#define IDL_    0x01
#define STOP_   0x02
#define SMOD_   0x80

/*------------------------------------------------
PMSR Bit Values - Defines the bit position in the reg!!!
------------------------------------------------*/
#define SEL_DIV_0_ 0x01
#define SEL_DIV_1_ 0x02
#define SEL_DIV_2_ 0x04

/*------------------------------------------------
Interrupt Vectors:
Interrupt Address = (Number * 8) + 3 
------------------------------------------------*/

#define IE0_VECTOR      0  /* 0x03 External Interrupt 0 */
#define TF0_VECTOR      1  /* 0x0B Timer 0 */
#define IE1_VECTOR      2  /* 0x13 External Interrupt 1 */
#define TF1_VECTOR      3  /* 0x1B Timer 1 */
#define SIO_VECTOR	4  /* 0x23 Serial channel 0 interrupt! (RI0/TI0) */
#define TF2_VECTOR      5  /* 0x2B Timer 2 */
//  SKIPPED #define X	6  /* RESERVED*/
#define IEX2_VECTOR	7    /* 0x3B External interrupt 2 */
#define IEX3_VECTOR	8    /* 0x43 External interrupt 3 */
#define IEX4_VECTOR	9    /* 0x4B External interrupt 4 */
#define IEX5_VECTOR	10   /* 0x53 External interrupt 5 */

/*------------------------------------------------
------------------------------------------------*/
#endif