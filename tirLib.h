/*----------------------------------------------------------------------------*
 *  Copyright (c) 2009        Southeastern Universities Research Association, *
 *                            Thomas Jefferson National Accelerator Facility  *
 *                                                                            *
 *    This software was developed under a United States Government license    *
 *    described in the NOTICE file included as part of this distribution.     *
 *                                                                            *
 *    Authors: David Abbott                                                   *
 *             abbottd@jlab.org                  Jefferson Lab, MS-12B3       *
 *             Phone: (757) 269-7190             12000 Jefferson Ave.         *
 *             Fax:   (757) 269-5800             Newport News, VA 23606       *
 *                                                                            *
 *             Bryan Moffit                                                   *
 *             moffit@jlab.org                   Jefferson Lab, MS-12B3       *
 *             Phone: (757) 269-5660             12000 Jefferson Ave.         *
 *             Fax:   (757) 269-5800             Newport News, VA 23606       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *
 * Description:
 *     Header file for the TJNAF VME Trigger Interface Library
 *
 *----------------------------------------------------------------------------*/

#ifndef __TIRLIB__
#define __TIRLIB__

#ifdef VXWORKS
#define GEF_VME_BUS_HDL  unsigned int
#else
#define TRUE  1
#define FALSE 0
#define OK    0
#define ERROR -1
#include <pthread.h>
#endif

pthread_mutex_t tirISR_mutex=PTHREAD_MUTEX_INITIALIZER;

#define INTLOCK {				\
    if(pthread_mutex_lock(&tirISR_mutex)<0)	\
      perror("pthread_mutex_lock");		\
}

#define INTUNLOCK {				\
    if(pthread_mutex_unlock(&tirISR_mutex)<0)	\
      perror("pthread_mutex_lock");		\
}

/* Define TIR Memory structure */
struct vme_tir {
    volatile unsigned short tir_csr;
    volatile unsigned short tir_vec;
    volatile unsigned short tir_dat;
    volatile unsigned short tir_oport;
    volatile unsigned short tir_iport;
  };


/* Define TIR Version IDs (readback of a reset CSR register */
#define TIR_VERSION_MASK 0x00ff
#define TIR_VERSION_1    0x00c0
#define TIR_VERSION_2    0x0080

/* Define TIR Modes of operation:    Ext trigger - Interrupt mode   0
                                     TS  trigger - Interrupt mode   1
                                     Ext trigger - polling  mode    2 
                                     TS  trigger - polling  mode    3  */
#define TIR_EXT_INT    0
#define TIR_TS_INT     1
#define TIR_EXT_POLL   2
#define TIR_TS_POLL    3

#define TIR_CLEAR_COUNT  1

#define TIR_EXTERNAL       0x1
#define TIR_ENABLED        0x2
#define TIR_INTERRUPT      0x4
#define TIR_TEST_MODE      0x8
#define TIR_LEVEL_MASK   0x700
#define TIR_INT_PENDING 0x4000
#define TIR_TRIG_STATUS 0x8000


/* Define default Interrupt vector and address for possible sources */
#define TIR_DEFAULT_ADDR 0x0ed0
#define TIR_INT_VEC      0xec


/* Define Functions prototypes */
BOOL tirIntIsRunning();
int    tirIntInit(unsigned int tAddr, unsigned int mode, int force);
int    tirIntConnect ( unsigned int vector, VOIDFUNCPTR routine, unsigned int arg);
void   tirIntDisconnect();
int    tirIntEnable(int iflag);
void   tirIntDisable();
void   tirIntAck();
void   tirIntPause();
void   tirIntResume();
unsigned int tirIntType();
int    tirIntPoll();
void tirPoll();
void   tirIntOutput(unsigned short out);
unsigned int    tirGetIntCount();
void   tirClearIntCount();
unsigned short tirReadCsr();
unsigned short tirReadVec();
unsigned short tirReadData();
unsigned short tirReadOutput();
unsigned short tirReadInput();
void   tirWriteCsr(unsigned short val);
int   tirIntStatus(int pflag);


#endif
