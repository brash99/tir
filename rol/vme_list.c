/*************************************************************************
 *
 *  vme_list.c - Library of routines for readout and buffering of
 *                events using a JLAB Trigger Interface (TI) with
 *                a Linux VME controller.
 *
 */

/* Event Buffer definitions */
#define MAX_EVENT_POOL     400
#define MAX_EVENT_LENGTH   1024*10      /* Size in Bytes */

/* Define Interrupt source and address */
#define TIR_SOURCE
#define TIR_ADDR 0x0ed0
/* TIR_MODE:  0 : interrupt on trigger,
              1 : interrupt from Trigger Supervisor signal
              2 : polling for trigger
              3 : polling for Trigger Supervisor signal  */
#define TIR_MODE 2

#define ADC_ID 0
#define MAX_ADC_DATA 34

#define TDC_ID 0
#define MAX_TDC_DATA 34

#define C792_BANKID 792
#define C775_BANKID 775

#include "dmaBankTools.h"
#include "linuxvme_list.c" /* source required for CODA */
#include "c792Lib.h"
#include "c775Lib.h"

/* function prototype */
void rocTrigger(int arg, int EVTYPE, int EVNUM);

void
rocDownload()
{

  /* Setup Address and data modes for DMA transfers
   *
   *  vmeDmaConfig(addrType, dataType, sstMode);
   *
   *  addrType = 0 (A16)    1 (A24)    2 (A32)
   *  dataType = 0 (D16)    1 (D32)    2 (BLK32) 3 (MBLK) 4 (2eVME) 5 (2eSST)
   *  sstMode  = 0 (SST160) 1 (SST267) 2 (SST320)
   */
  vmeDmaConfig(2,5,1);

  c775Init(0xa10000,0,1,0);
  c792Init(0x110000,0,1,0);

  printf("rocDownload: User Download Executed\n");

}

void
rocPrestart()
{
  unsigned short iflag;
  int stat;

  /* Program/Init VME Modules Here */
  /* Setup ADCs (no sparsification, enable berr for block reads) */
  c792Sparse(ADC_ID,0,0);
  c792Clear(ADC_ID);
  c792DisableBerr(ADC_ID); // Disable berr - multiblock read
/*  c792EnableBerr(ADC_ID); /\* for 32bit block transfer *\/ */

  c792Status(ADC_ID,0,0);

/* Program/Init VME Modules Here */
  /* Setup TDCs (no sparcification, enable berr for block reads) */
  c775Clear(TDC_ID);
/*  c775DisableBerr(TDC_ID); // Disable berr - multiblock read */
/*   c775EnableBerr(TDC_ID); /\* for 32bit block transfer *\/ */
  c775CommonStop(TDC_ID);
  //c775CommonStart(TDC_ID);

  c775Status(TDC_ID);


  /* Program/Init VME Modules Here */
  

  printf("rocPrestart: User Prestart Executed\n");

}

void
rocGo()
{
  /* Enable modules, if needed, here */
  c775Status(TDC_ID);
  c792Status(ADC_ID,0,0); //TONY ADDED THIS
  /* Interrupts/Polling enabled after conclusion of rocGo() */
}

void
rocEnd()
{

  c775Status(TDC_ID);
  c792Status(ADC_ID,0,0); //TONY ADDED THIS

  printf("rocEnd: Ended after %d events\n",tirGetIntCount());

}

void
rocTrigger(int EVNUM, int EVTYPE, int syncFlag)
{
  int ii, status, dma, count;
  int nwords;
  unsigned int datascan, tirval, vme_addr;
  int itimeout=0;

  /* Insert Trigger Bank, 0xFF10 : raw trigger, no timestamps */
  InsertDummyTriggerBank(0xFF10, EVNUM, EVTYPE, blockLevel); 


  /* unsigned int scanmask =0, datascan = 0;
  int iadc;
  int dCnt;
  int Nc792 = 1; */

  /* Check if an Event is available */
  /* scanmask = c792ScanMask();
  datascan = c792GDReady(scanmask, 1000); */

  /* Open Bank number 5 (for example), Bank of 32bit unsigned integers */
  BANKOPEN(5,BT_UI4,0);
  /* Insert some data here - Make sure bytes are ordered little-endian (LSWAP)*/
  *dma_dabufp++ = LSWAP(0xda000022);
  for(ii=0; ii<20; ii++)
    {
      *dma_dabufp++ = LSWAP(ii);
    }
  *dma_dabufp++ = LSWAP(0xda0000ff);

  BANKCLOSE;

  BANKOPEN(C792_BANKID,BT_UI4,blockLevel);
/*  if(datascan==scanmask)
    {
      for(iadc = 0; iadc < Nc792; iadc++)
        {
          dCnt = c792ReadBlock(iadc,dma_dabufp,MAX_ADC_DATA+40);
          if(dCnt <= 0)
            {
              logMsg("%4d: ERROR: ADC %2d Read Failed - Status 0x%x\n",
                     tirGetIntCount(),
                     iadc, dCnt,0,0,0,0);
              *dma_dabufp++ = LSWAP(iadc);
              *dma_dabufp++ = LSWAP(0xda00bad1);
              c792Clear(iadc);
            }
          else
            {
              dma_dabufp += dCnt;
            }
        }

    }
  else
    {
      logMsg("%4d: ERROR: datascan != scanmask for ADC  (0x%08x != 0x%08x)\n",
             tirGetIntCount(),
             datascan,scanmask,0,0,0,0);
      *dma_dabufp++ = LSWAP(datascan);
      *dma_dabufp++ = LSWAP(scanmask);
      *dma_dabufp++ = LSWAP(0xda00bad2);

      for(iadc = 0; iadc < Nc792; iadc++)
        c792Clear(iadc);
    } */

  while(itimeout<1000)
    {
      itimeout++;
      status = c792Dready(ADC_ID);
      if(status>0) break;
    }
  if(status > 0)
    {
      if(tirGetIntCount() %100==0)
        {
          printf("itimeout = %d\n",itimeout);
          c792PrintEvent(ADC_ID,0);
        }
      else
        {
          nwords = c792ReadEvent(ADC_ID,dma_dabufp);
          /* or use c792ReadBlock, if BERR was enabled */
/*        nwords = c792ReadBlock(ADC_ID,dma_dabufp,MAX_ADC_DATA); */
          if(nwords<=0)
            {
              logMsg("ERROR: ADC Read Failed - Status 0x%x\n",nwords,0,0,0,0,0);
              *dma_dabufp++ = 0xda000bad;
              c792Clear(ADC_ID);
            }
          else
            {
              dma_dabufp += nwords;
            }
        }
    }
  else
    {
      logMsg("ERROR: NO data in ADC  datascan = 0x%x, itimeout=%d\n",status,itimeout,0,0,0,0);
      c792Clear(ADC_ID);
    }

  BANKCLOSE;

  BANKOPEN(C775_BANKID,BT_UI4,blockLevel);
  while(itimeout<1000)
    {
      itimeout++;
      status = c775Dready(TDC_ID);
      if(status>0) break;
    }
  if(status > 0)
    {
      if(tirGetIntCount() %100==0)
        {
          printf("itimeout = %d\n",itimeout);
          c775PrintEvent(TDC_ID,0);
        }
      else
        {
          nwords = c775ReadEvent(TDC_ID,dma_dabufp);
          /* or use c775ReadBlock, if BERR was enabled */
/*        nwords = c775ReadBlock(TDC_ID,dma_dabufp,MAX_TDC_DATA); */
          if(nwords<=0)
            {
              logMsg("ERROR: TDC Read Failed - Status 0x%x\n",nwords,0,0,0,0,0);
              *dma_dabufp++ = 0xda000bad;
              c775Clear(TDC_ID);
            }
          else
            {
              dma_dabufp += nwords;
            }
        }
    }
  else
    {
      //      logMsg("ERROR: NO data in TDC  datascan = 0x%x, itimeout=%d\n",status,itimeout,0,0,0,0);
      c775Clear(TDC_ID);
    }
  *dma_dabufp++ = LSWAP(0xdaebd00d); /* Event EOB */ //TONY - made no change
  BANKCLOSE;

}

void
rocCleanup()
{
  /* Put any clean up code that should be done before the next readout list is downloaded */
}
