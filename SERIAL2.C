/***********************************************************
*                                                          *
*   serial I/O routines for NAVSIM navigation simulator    *
*                                                          *
*   900607 V1.2                                            *
*                                                          *
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <conio.h>
#include <string.h>
#include "serial.h"

#ifdef __MSC__
#include <stdarg.h>
#endif

#define FALSE 0
#define TRUE !FALSE

#ifdef __MSC__
#define disable() _disable()
#define enable()  _enable()
#define bioscom(a,b,c) _bios_serialcom(a,c,b)
#define getvect(a) _dos_getvect(a)
#define setvect(a,b) _dos_setvect(a,b)
#endif

/*----------------------------------------
| UART Addresses (only COM1: implemented)
-----------------------------------------*/
#define  UART 0x3F8       /* Port addresses of first serial interface */
#define  RBR  UART + 0    /* Receiver Buffer Register                 */
#define  THR  UART + 0    /* Transmitter Holding Register             */
#define  IER  UART + 1    /* Interrrupt Enable Register               */
#define  IIR  UART + 2    /* Interrupt Identification Register        */
#define  LCR  UART + 3    /* Line Control Register                    */
#define  MCR  UART + 4    /* Modem Control Register                   */
#define  LSR  UART + 5    /* Line Status Register                     */
#define  MSR  UART + 6    /* Modem Status Register                    */

#define  XON  0x13        /* ^Q */
#define  XOFF 0x11        /* ^S */

/*---------------------
| Interrupt controller
----------------------*/
#define  EOI  0x20

/*---------------------
| I/O Buffer variables
----------------------*/
#define  BUFSIZE   1024
#define  LOWMARK   200
#define  HIGHMARK  BUFSIZE - LOWMARK

static	char tx_buf[BUFSIZE+3];
static	int  tx_spos = 0;
static	int  tx_rpos = 0;
static	int  tx_cnt  = 0;
static  int  tx_off  = FALSE;

static	char rx_buf[BUFSIZE+3];
static	int  rx_spos = 0;
static	int  rx_rpos = 0;
static	int  rx_cnt  = 0;
static  int  rx_off  = FALSE;

/*-----------------
| Local variables
------------------*/
static void (interrupt far *old_irq4_handler)();
static int  xon_xoff = FALSE;
static BYTE s_error      = 0;     /* Serial error value     */
static unsigned int s_code;       /* Serial data parameters */

/*------------
| prototypes
-------------*/
static void interrupt far our_irq4_handler(void);
static void THRE_int(void);
static void RDA_int(void);
static void MODEM_int(void);
static void RLS_int(void);

/*-------------------------------------------------------
| Global variables (can be modified outside this module)
--------------------------------------------------------*/
volatile int  msg_received = 0;     /* message counter   */
unsigned char input_term   = '\0';  /* Input terminator       */
unsigned char output_term  = '\0';  /* Output terminator      */

/*-------------------------------------------------------------------
|   FUNCTION: s_interm(BYTE c);
|    PURPOSE: set serial input terminator
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901106 V0.1
---------------------------------------------------------------------*/
void s_interm(BYTE c)
{
  input_term = c;
}

/*-------------------------------------------------------------------
|   FUNCTION: s_outterm(BYTE c)
|    PURPOSE: set serial output terminator
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901106 V0.1
---------------------------------------------------------------------*/
void s_outterm(BYTE c)
{
  output_term = c;
}

/*-------------------------------------------------------------------
|   FUNCTION: BYTE serial_error()
|    PURPOSE: Inquire serial error
| DESRIPTION: Returns and resets serial error indicator
|    RETURNS: Serial error indicator
|    VERSION: 901106 V0.1
---------------------------------------------------------------------*/
BYTE serial_error()
{
  BYTE c;

  c = s_error;
  s_error = 0;
  return(c);
}

/*-------------------------------------------------------------------
|   FUNCTION: s_putc(BYTE c)
|    PURPOSE: Serial putc
| DESRIPTION: Send c to UART or put it in circular output buffer
|    RETURNS: nothing
|    VERSION: 901106 V0.2
---------------------------------------------------------------------*/
void s_putc(unsigned char c)
{
  if (tx_cnt > 0)
  {
    if (tx_cnt < BUFSIZE)	                /* Room in tx_buffer ?	   */
    {
      tx_buf[tx_spos++] = c;			    /* store c in buf	       */
      if (tx_spos > BUFSIZE)  tx_spos = 0;  /* wrap if needed          */
      tx_cnt++;
    }
  }
  else
  {
    while (!(inp(LSR) & 0x20)) ;           /* Wait until tx buffer clear */
    outp(THR,(int)c);
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: s_getc(BYTE *c)
|    PURPOSE: Serial getc
| DESRIPTION: Get a character from input buffer (if any)
|    RETURNS: 1 if successfull, 0 if buffer empty
|    VERSION: 901106 V0.2
---------------------------------------------------------------------*/
int s_getc(unsigned char *c)
{
  disable();
  if (rx_cnt > 0)			    /* Characters in rx_buf ?  */
  {
    *c = rx_buf[rx_rpos++];		     /* Get char		*/
    if (rx_rpos > BUFSIZE) rx_rpos = 0;     /* wrap if needed	       */
    rx_cnt--;				    /* dec. no. of char's      */
    enable();
    return(TRUE);			    /*	 return success        */
  }
  else
  {
    *c = '\0';				     /* No, return NULL character */
    enable();
    return(FALSE);			    /*	and Return no success	 */
  }
}

/*-------------------------------------------------------------------
|   FUNCTION: RLS_int()
|    PURPOSE: Receiver Line Status interrupt
| DESRIPTION: Interrupt is set by Overrun Error, Parity Error or
|             Framing error or Break Interrupt.
|             V0.1 will only read Line Status Register and do nothing
|    RETURNS: Nothing
|    VERSION: 910417 V0.1
---------------------------------------------------------------------*/
void RLS_int()
{
  BYTE c;

  c = (BYTE) inp(LSR);       /* Read Modem Status Register */
  s_error |= c;              /* Set error code             */

}

/*-------------------------------------------------------------------
|   FUNCTION: MODEM_int()
|    PURPOSE: Modem interrupt
| DESRIPTION: Interrupt is set by Clear to Send (CTS),
|             Data Set Ready (DSR),
|             Ring Indicator (RI),
|             Data Carrier Detect
|             V0.1 will only read Modem Status Register and do nothing
|    RETURNS: Nothing
|    VERSION: 910417 V0.1
---------------------------------------------------------------------*/
void MODEM_int()
{
  BYTE c;

  c = (BYTE) inp(MSR);       /* Read Modem Status Register */
  c++;
}

/*-------------------------------------------------------------------
|   FUNCTION: RDA_int()
|    PURPOSE: Receive Data Available interrupt
| DESRIPTION: After a RDA interrupt is detected, this function will
|             be called. It get the character from UART and stores
|             it in the serial input buffer
|    RETURNS: Nothing
|    VERSION: 901106 V0.3
---------------------------------------------------------------------*/
void RDA_int()
{
  BYTE c;

  c = (BYTE) inp(RBR);       /* Get character from UART */

  if ((input_term) && c == input_term)
  {
    msg_received++;
  }
  else
  {
    if (xon_xoff)
    {
      if (c == XON)
        tx_off = FALSE;
      else if (c == XOFF)
        tx_off = TRUE;
    }
  }

  if (rx_cnt < BUFSIZE)
  {
    rx_buf[rx_spos++] = c;		            /* store c in buf	  */
    if (rx_spos > BUFSIZE)  rx_spos = 0;    /* wrap		          */
    rx_cnt++;				                /* inc no. of chars   */
  }

  if (xon_xoff && !rx_off && rx_cnt>HIGHMARK)
  {
    s_putc(XOFF);
    rx_off = TRUE;
  }

}

/*-------------------------------------------------------------------
|   FUNCTION: THRE_int()
|    PURPOSE: Transmitter Holding Register Empty interrupt
| DESRIPTION: After detecting a THRE interrupt, this function will be
|             called.
|    RETURNS: Nothing
|    VERSION: 901106 V0.3
---------------------------------------------------------------------*/
void THRE_int()
{
  char c;

  if (!tx_off && tx_cnt>0)
  {
    c = tx_buf[tx_rpos++];		      /* Get char	     */
    outp(THR,c);
    if (tx_rpos > BUFSIZE)  tx_rpos = 0;      /* wrap if needed      */
    tx_cnt--;				      /* dec. no. of char's  */
  }

  if (xon_xoff && rx_off && (rx_cnt < LOWMARK))
  {
    s_putc(XON);	      /* send XON */
    rx_off = FALSE;
  }

}

/*-------------------------------------------------------------------
|   FUNCTION: our_irq4_handler()
|    PURPOSE: This function will be called by an IRQ4
| DESRIPTION: Performs dummy reads on Modem Control Reg. and Interrupt
|             Identification Reg. It then get the Line Status Register,
|             determines the source of the interrupt and takes action.
|             It also detects a serial error
|    RETURNS: Nothing
|    VERSION: 901106 V0.4
---------------------------------------------------------------------*/

void interrupt far our_irq4_handler()
{
  static BYTE index;

  static void (*isr[4])() =        /* Array of pointers to handlers */
  {
    MODEM_int,
    THRE_int,
    RDA_int,
    RLS_int,
  };

  disable();
  for (;;)
  {
    index = (BYTE)inp(IIR);
    index &= 0x07;
    if (index & 01) break;            /* Break if no more interrupts */
    index >>= 1;                      /* Adjust index                */
    (*isr[index])();
  }

  outp(0x20,EOI);	    /* send EOI */
  enable();
}

/*-------------------------------------------------------------------
|   FUNCTION: s_puts(char *s)
|    PURPOSE: Puts string on serial port
| DESRIPTION: Ouput of a string to serial port by adding the string
|             to the circular output buffer. If the output buffer
|             was empty, the transmission will be started by writing
|             the first character directly to the UART
|    RETURNS: Nothing
|    VERSION: 901106 V0.3
---------------------------------------------------------------------*/
void s_puts(char *s)
{
  int i,j;
  int flag = FALSE;
  static int time_out = 0;

  if (tx_off) return;

  j = 0;

  /*---------------------------------------------------------------
  | With an empty buffer, write the string from second character
  | in the buffer and write first character directly to UART
  ----------------------------------------------------------------*/
  if (tx_cnt == 0)
  {
    j = 1;          /* Start with 2nd character */
    flag = TRUE;
  }

  while (s[j])
  {
    if (tx_cnt < BUFSIZE-10)	            /* Room in tx_buffer ? */
    {
      tx_buf[tx_spos++] = s[j++];           /* store char in buf   */
      if (tx_spos > BUFSIZE) tx_spos = 0;
      tx_cnt++;
    }
    else                      /* Buffer is full */
    {
      if (flag)               /* If buffer was empty then */
      {
        flag = FALSE;         /* Premature kick-start of output */
        outp(THR,s[0]);
      }
      else
      {
        s_error |= 0x20;      /* Set buffer full error       */
        time_out++;           /* Increment time-out counter  */
        if (time_out > 20)    /* If time_out, reset pointers */
        {
          tx_spos=0;
          tx_cnt=0;
        }
      }
    }
  }

  if (flag)
    outp(THR,s[0]);    /* Write first character directly in UART */

}

/*-------------------------------------------------------------------
|   FUNCTION: s_debug(short line, short col, char *format, ....)
|    PURPOSE: Serial debug
| DESRIPTION: Sends a formatted string to a terminal on COM1:
|             Starts string on 'line' and 'col'
|    RETURNS: Nothing
|    VERSION: 901106 V0.2
---------------------------------------------------------------------*/
void s_debug( short line, short col, char *format, ...)
{
  va_list arguments;
  static  char  esc_str[] = {0x1B,'Y','.','.',0};
  static  char  s[132];
  static  short last_line;
  static  short last_col;

  if (line == -1)
    line = ++last_line;
  else
    last_line = line;

  if (col == -1)
    col = last_col;
  else
    last_col = col;

  /*---------------------------------------------------------
  | Position cursor of terminal on specified line and column
  ----------------------------------------------------------*/
  #ifdef PC_SPEED         /* faster then sprintf() */
    esc_str[2] = (char) col+32;
    esc_str[3] = (char) line+32;
  #else
    esc_str[2] = (char) line+32;
    esc_str[3] = (char) col+32;
  #endif
  s_puts(esc_str);

  /*-----------------------------------
  | Send debug information to terminal
  -------------------------------------*/
  va_start(arguments,format);
  vsprintf(s,format,arguments);
  s_puts(s);
  va_end(arguments);
}

/*-------------------------------------------------------------------
|   FUNCTION: s_gets(char *s)
|    PURPOSE: Serial gets
| DESRIPTION: Gets string out of circular input buffer. The end of
|             the string is determined by the current input terminator
|    RETURNS: Nothing
|    VERSION: 901106 V0.2
---------------------------------------------------------------------*/
void s_gets(char *s)
{
  char c;
  int  i=0;

  do
  {
    disable();
    if (rx_cnt > 0)		        /* Characters in rx_buf ?  */
    {
      c = rx_buf[rx_rpos++];	/* Get char		           */
      s[i++] = c;		        /* Add to message	       */
      if (rx_rpos > BUFSIZE) rx_rpos = 0;
      rx_cnt--; 		        /* dec. no. of char's      */
    }
    enable();
  }
  while (c != input_term);
  s[i] = '\0';
}

/*-------------------------------------------------------------------
|   FUNCTION: s_baudrate(unsigned int i)
|    PURPOSE: Set baudrate of COM1
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901106 V0.1
---------------------------------------------------------------------*/
void s_baudrate(unsigned int i)
{

  s_code &= 0x1F;     /* Reset bits 5-7 */

  switch (i)
  {
    case 110 : s_code |= 0x00; break;
    case 150 : s_code |= 0x20; break;
    case 300 : s_code |= 0x40; break;
    case 600 : s_code |= 0x60; break;
    case 1200: s_code |= 0x80; break;
    case 2400: s_code |= 0xA0; break;
    case 4800: s_code |= 0xC0; break;
    case 9600: s_code |= 0xE0; break;
    default:   s_code |= 0xE0; break;  /* 9600 baud */
  }
  bioscom(0,s_code,0);              /* Initialize UART / COM1: */

}

/*-------------------------------------------------------------------
|   FUNCTION: s_databits(unsigned int i)
|    PURPOSE: Set number of databits of COM1
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901106 V0.1
---------------------------------------------------------------------*/
void s_databits(BYTE i)
{
  s_code &= 0xFC;     /* Reset bits 0-1 */

  switch (i)
  {
    case 7:  s_code |= 0x02; break;
    case 8:  s_code |= 0x03; break;
    default: s_code |= 0x03; break;   /* 8 databits */
  }

  bioscom(0,s_code,0);              /* Initialize UART / COM1: */

}

/*-------------------------------------------------------------------
|   FUNCTION: s_stopbits(unsigned int i)
|    PURPOSE: Set number of stopbits of COM1
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901106 V0.1
---------------------------------------------------------------------*/
void s_stopbits(BYTE i)
{
  s_code &= 0xFB;     /* Reset bit 2 */

  switch (i)
  {
    case 1:  s_code |= 0x00; break;
    case 2:  s_code |= 0x04; break;
    default: s_code |= 0x00; break;   /* 1 stopbit */
  }

  bioscom(0,s_code,0);              /* Initialize UART / COM1: */

}

/*-------------------------------------------------------------------
|   FUNCTION: s_parity(char p)
|    PURPOSE: Set parity of COM1
| DESRIPTION: -
|    RETURNS: Nothing
|    VERSION: 901106 V0.1
---------------------------------------------------------------------*/
void s_parity(char p)
{
  s_code &= 0xE7;     /* Reset bits 3-4 */

  switch (p)
  {
    case 'n':
    case 'N':  s_code |= 0x00; break;
    case 'o':
    case 'O':  s_code |= 0x08; break;
    case 'e':
    case 'E':  s_code |= 0x18; break;
    default:   s_code |= 0x00; break;  /* no parity */
  }

  bioscom(0,s_code,0);              /* Initialize UART / COM1: */

}


/*-------------------------------------------------------------------
|   FUNCTION: init_serial()
|    PURPOSE: Initialize serial I/O.
| DESRIPTION: Replaces IRQ4 interrupt vector when called for first
|             time, initializes I/O buffers, initializes UART and
|             sends a XON to serial port
|    RETURNS: Nothing
|    VERSION: 901106 V0.5
---------------------------------------------------------------------*/
void init_serial()
{
  int   c;
  static int first_time = TRUE;

  /*------------------------------------------------------
  |  Replace original IRQ4 handler, but do this only once
  --------------------------------------------------------*/
  if (first_time)
  {
    disable();
    old_irq4_handler = getvect(0x0C);  /* store old int. vector     */
    setvect(0x0C,our_irq4_handler);    /* set new int. vector       */
    enable();
    atexit(reset_serial);              /* Perform reset_serial upon exit() */
    first_time = FALSE;
  }

  /*--------------------------------------
  | Initialize input buffer and pointers
  ---------------------------------------*/
  rx_cnt  = 0;	   /* number of characters in buffer */
  rx_spos = 0;	   /* store position		     */
  rx_rpos = 0;	   /* retrieve postion		     */
  rx_off  = FALSE;

  /*--------------------------------------
  | Initialize output buffer and pointers
  ---------------------------------------*/
  tx_cnt  = 0;
  tx_rpos = 0;
  tx_spos = 0;
  tx_off  = FALSE;

  /*-----------------------------------
  | Initialize other control variables
  ------------------------------------*/
  msg_received = 0;
  s_error  = 0;

  /*-------------------
  | Enable interrupts
  --------------------*/
  disable();
  c = inp(LCR) & 0x7F;
  outp(LCR,c);	                /* Reset DLAB	    	*/
  outp(IER,0x0F);		 		/* enable interrupts 	*/
  outp(LSR,0x00);
  outp(MCR,0x08);		 		/* out2 on				*/
  c = inp(0x21) & 0xEF;
  outp(0x21,c);	                /* enable serial int. 	*/
  outp(0x20,EOI);		 		/* send EOI				*/
  enable();

}

/*-------------------------------------------------------------------
|   FUNCTION: reset_serial()
|    PURPOSE: Reset serial I/O
| DESRIPTION: Disables interrupts, resets original interrupt vector,
|             resets circular I/O buffers
|    RETURNS: Nothing
|    VERSION: 901106 V0.3
---------------------------------------------------------------------*/
void reset_serial()
{

  disable();

  outp(0x21,inp(0x21) | 0x10);	  /* disable serial int. in 8259  */
  outp(IER,0);      			  /* clear int. enable register   */
  outp(MCR,0);		        	  /* clear modem contr. register  */

  outp(0x20,EOI);		          /* send EOI			  */
  enable();

  rx_cnt  = 0;	   /* number of characters in buffer */
  rx_spos = 0;	   /* store position		     */
  rx_rpos = 0;	   /* retrieve postion		     */

  tx_cnt  = 0;	   /* number of characters in buffer */
  tx_spos = 0;	   /* store position		     */
  tx_rpos = 0;	   /* retrieve postion		     */

  disable();
  setvect(0x0C,old_irq4_handler); /* Reset irq4 vector     */
  enable();
}
