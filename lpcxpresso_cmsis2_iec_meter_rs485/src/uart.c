/***********************************************************************
 * $Id::                                                               $
 *
 * Project:	uart: Simple UART echo for LPCXpresso 1700
 * File:	uart.c
 * Description:
 * 			LPCXpresso Baseboard uses pins mapped to UART3 for
 * 			its USB-to-UART bridge. This application simply echos
 * 			all characters received.
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

/*****************************************************************************
 *   History
 *   2010.07.01  ver 1.01    Added support for UART3, tested on LPCXpresso 1700
 *   2009.05.27  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include "LPC17xx.h"
#include "type.h"
#include "uart.h"
#include <stdlib.h>

#define RS485_USE_RTS1


volatile uint32_t UART0Status, UART1Status, UART2Status, UART3Status;
volatile uint8_t UART0TxEmpty = 1, UART1TxEmpty = 1, UART2TxEmpty = 1, UART3TxEmpty=1;
volatile uint8_t UART0Buffer[BUFSIZE], UART1Buffer[BUFSIZE], UART2Buffer[BUFSIZE], UART3Buffer[BUFSIZE];
volatile uint8_t UART0Count = 0, UART1Count = 0, UART2Count = 0, UART3Count = 0;

volatile uint32_t UART1LastReceived = 0;
volatile uint32_t UARTMsTicks = 0;

/*****************************************************************************
** Function name:		UART0_IRQHandler
**
** Descriptions:		UART0 interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void UART0_IRQHandler (void) 
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;
	
  IIRValue = LPC_UART0->IIR;
    
  IIRValue >>= 1;			/* skip pending bit in IIR */
  IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  if ( IIRValue == IIR_RLS )		/* Receive Line Status */
  {
	LSRValue = LPC_UART0->LSR;
	/* Receive Line Status */
	if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
	{
	  /* There are errors or break interrupt */
	  /* Read LSR will clear the interrupt */
	  UART0Status = LSRValue;
	  Dummy = LPC_UART0->RBR;		/* Dummy read on RX to clear 
							interrupt, then bail out */
	  return;
	}
	if ( LSRValue & LSR_RDR )	/* Receive Data Ready */			
	{
	  /* If no error on RLS, normal ready, save into the data buffer. */
	  /* Note: read RBR will clear the interrupt */
	  UART0Buffer[UART0Count] = LPC_UART0->RBR;
	  UART0Count++;
	  if ( UART0Count == BUFSIZE )
	  {
		UART0Count = 0;		/* buffer overflow */
	  }	
	}
  }
  else if ( IIRValue == IIR_RDA )	/* Receive Data Available */
  {
	/* Receive Data Available */
	UART0Buffer[UART0Count] = LPC_UART0->RBR;
	UART0Count++;
	if ( UART0Count == BUFSIZE )
	{
	  UART0Count = 0;		/* buffer overflow */
	}
  }
  else if ( IIRValue == IIR_CTI )	/* Character timeout indicator */
  {
	/* Character Time-out indicator */
	UART0Status |= 0x100;		/* Bit 9 as the CTI error */
  }
  else if ( IIRValue == IIR_THRE )	/* THRE, transmit holding register empty */
  {
	/* THRE interrupt */
	LSRValue = LPC_UART0->LSR;		/* Check status in the LSR to see if
									valid data in U0THR or not */
	if ( LSRValue & LSR_THRE )
	{
	  UART0TxEmpty = 1;
	}
	else
	{
	  UART0TxEmpty = 0;
	}
  }
}

/*****************************************************************************
** Function name:		UART1_IRQHandler
**
** Descriptions:		UART1 interrupt handler
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/
void UART1_IRQHandler (void) 
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;
	
  IIRValue = LPC_UART1->IIR;
    
  IIRValue >>= 1;			/* skip pending bit in IIR */
  IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  if ( IIRValue == IIR_RLS )		/* Receive Line Status */
  {
	LSRValue = LPC_UART1->LSR;
	/* Receive Line Status */
	if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
	{
	  /* There are errors or break interrupt */
	  /* Read LSR will clear the interrupt */
	  UART1Status = LSRValue;
	  Dummy = LPC_UART1->RBR;		/* Dummy read on RX to clear 
								interrupt, then bail out */
	  return;
	}
	if ( LSRValue & LSR_RDR )	/* Receive Data Ready */			
	{
	  /* If no error on RLS, normal ready, save into the data buffer. */
	  /* Note: read RBR will clear the interrupt */
      UART1LastReceived = UARTMsTicks;
	  UART1Buffer[UART1Count] = LPC_UART1->RBR;
	  UART1Count++;
	  if ( UART1Count == BUFSIZE )
	  {
		UART1Count = 0;		/* buffer overflow */
	  }	
	}
  }
  else if ( IIRValue == IIR_RDA )	/* Receive Data Available */
  {
	/* Receive Data Available */
    UART1LastReceived = UARTMsTicks;
	UART1Buffer[UART1Count] = LPC_UART1->RBR;
	UART1Count++;
	if ( UART1Count == BUFSIZE )
	{
	  UART1Count = 0;		/* buffer overflow */
	}
  }
  else if ( IIRValue == IIR_CTI )	/* Character timeout indicator */
  {
	/* Character Time-out indicator */
	UART1Status |= 0x100;		/* Bit 9 as the CTI error */
  }
  else if ( IIRValue == IIR_THRE )	/* THRE, transmit holding register empty */
  {
	/* THRE interrupt */
	LSRValue = LPC_UART1->LSR;		/* Check status in the LSR to see if
								valid data in U0THR or not */
	if ( LSRValue & LSR_THRE )
	{
	  UART1TxEmpty = 1;
	}
	else
	{
	  UART1TxEmpty = 0;
	}
  }

}
/*****************************************************************************
** Function name:		UART0_IRQHandler
**
** Descriptions:		UART0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
*****************************************************************************/
void UART2_IRQHandler (void)
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;

  IIRValue = LPC_UART2->IIR;

  IIRValue >>= 1;			/* skip pending bit in IIR */
  IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  if ( IIRValue == IIR_RLS )		/* Receive Line Status */
  {
	LSRValue = LPC_UART2->LSR;
	/* Receive Line Status */
	if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
	{
	  /* There are errors or break interrupt */
	  /* Read LSR will clear the interrupt */
	  UART2Status = LSRValue;
	  Dummy = LPC_UART2->RBR;		/* Dummy read on RX to clear
							interrupt, then bail out */
	  return;
	}
	if ( LSRValue & LSR_RDR )	/* Receive Data Ready */
	{
	  /* If no error on RLS, normal ready, save into the data buffer. */
	  /* Note: read RBR will clear the interrupt */
	  UART2Buffer[UART2Count] = LPC_UART2->RBR;
	  UART2Count++;
	  if ( UART2Count == BUFSIZE )
	  {
		UART2Count = 0;		/* buffer overflow */
	  }
	}
  }
  else if ( IIRValue == IIR_RDA )	/* Receive Data Available */
  {
	/* Receive Data Available */
	UART2Buffer[UART2Count] = LPC_UART2->RBR;
	UART2Count++;
	if ( UART2Count == BUFSIZE )
	{
	  UART2Count = 0;		/* buffer overflow */
	}
  }
  else if ( IIRValue == IIR_CTI )	/* Character timeout indicator */
  {
	/* Character Time-out indicator */
	UART2Status |= 0x100;		/* Bit 9 as the CTI error */
  }
  else if ( IIRValue == IIR_THRE )	/* THRE, transmit holding register empty */
  {
	/* THRE interrupt */
	LSRValue = LPC_UART2->LSR;		/* Check status in the LSR to see if
									valid data in U0THR or not */
	if ( LSRValue & LSR_THRE )
	{
	  UART2TxEmpty = 1;
	}
	else
	{
	  UART2TxEmpty = 0;
	}
  }
}



void UART3_IRQHandler (void)
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;

  IIRValue = LPC_UART3->IIR;

  IIRValue >>= 1;			/* skip pending bit in IIR */
  IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  if ( IIRValue == IIR_RLS )		/* Receive Line Status */
  {
	LSRValue = LPC_UART3->LSR;
	/* Receive Line Status */
	if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
	{
	  /* There are errors or break interrupt */
	  /* Read LSR will clear the interrupt */
	  UART3Status = LSRValue;
	  Dummy = LPC_UART3->RBR;		/* Dummy read on RX to clear
							interrupt, then bail out */
	  return;
	}
	if ( LSRValue & LSR_RDR )	/* Receive Data Ready */
	{
	  /* If no error on RLS, normal ready, save into the data buffer. */
	  /* Note: read RBR will clear the interrupt */
	  UART3Buffer[UART3Count] = LPC_UART3->RBR;
	  UART3Count++;
	  if ( UART3Count == BUFSIZE )
	  {
		UART3Count = 0;		/* buffer overflow */
	  }
	}
  }
  else if ( IIRValue == IIR_RDA )	/* Receive Data Available */
  {
	/* Receive Data Available */
	UART3Buffer[UART3Count] = LPC_UART3->RBR;
	UART3Count++;
	if ( UART3Count == BUFSIZE )
	{
	  UART3Count = 0;		/* buffer overflow */
	}
  }
  else if ( IIRValue == IIR_CTI )	/* Character timeout indicator */
  {
	/* Character Time-out indicator */
	UART3Status |= 0x100;		/* Bit 9 as the CTI error */
  }
  else if ( IIRValue == IIR_THRE )	/* THRE, transmit holding register empty */
  {
	/* THRE interrupt */
	LSRValue = LPC_UART3->LSR;		/* Check status in the LSR to see if
									valid data in U0THR or not */
	if ( LSRValue & LSR_THRE )
	{
	  UART3TxEmpty = 1;
	}
	else
	{
	  UART3TxEmpty = 0;
	}
  }
}


/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART port, setup pin select,
**						clock, parity, stop bits, FIFO, etc.
**
** parameters:			portNum(0 or 1) and UART baudrate
** Returned value:		true or false, return false only if the 
**						interrupt handler can't be installed to the 
**						VIC table
** 
*****************************************************************************/
uint32_t UARTInit( uint8_t PortNum, uint32_t baudrate )
{
  uint32_t Fdiv;
  uint32_t pclkdiv, pclk;

  if ( PortNum == 0 )
  {
	LPC_PINCON->PINSEL0 &= ~0x000000F0;
	LPC_PINCON->PINSEL0 |= 0x00000050;  /* RxD0 is P0.3 and TxD0 is P0.2 */
	/* By default, the PCLKSELx value is zero, thus, the PCLK for
	all the peripherals is 1/4 of the SystemFrequency. */
	/* Bit 6~7 is for UART0 */
	pclkdiv = (LPC_SC->PCLKSEL0 >> 6) & 0x03;
	switch ( pclkdiv )
	{
	  case 0x00:
	  default:
		pclk = SystemCoreClock/4;
		break;
	  case 0x01:
		pclk = SystemCoreClock;
		break; 
	  case 0x02:
		pclk = SystemCoreClock/2;
		break; 
	  case 0x03:
		pclk = SystemCoreClock/8;
		break;
	}

	LPC_UART0->LCR = 0x83;
	Fdiv = ( pclk / 16 ) / baudrate ;	/*baud rate */
    LPC_UART0->DLM = Fdiv / 256;							
    LPC_UART0->DLL = Fdiv % 256;
	LPC_UART0->LCR = 0x03;		/* DLAB = 0, 8 bits, no Parity, 1 Stop bit */
    LPC_UART0->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */

   	NVIC_EnableIRQ(UART0_IRQn);

    LPC_UART0->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART0 interrupt */
    return (TRUE);
  }
  else if ( PortNum == 1 )
  {

	// set TXD1 function //
    LPC_PINCON->PINSEL0	&= (~(0b11 << 30));
    LPC_PINCON->PINSEL0	|= (0b01 << 30);

    // set RXD1 function //
    LPC_PINCON->PINSEL1	&= (~(0b11 << 0));
    LPC_PINCON->PINSEL1	|= (0b01 << 0);

#ifdef RS485_USE_RTS1
    // set RTS1 function for RS485 direction control
    LPC_PINCON->PINSEL1	&= (~(0b11 << 12));
    LPC_PINCON->PINSEL1	|= (0b01 << 12);
#else
    // set DTR1 function for RS485 direction control
    LPC_PINCON->PINSEL1	&= (~(0b11 << 8));
    LPC_PINCON->PINSEL1	|= (0b01 << 8);
#endif

    /*
	LPC_PINCON->PINSEL4 &= ~0x0000000F;
	LPC_PINCON->PINSEL4 |= 0x0000000A;	// Enable RxD1 P2.1, TxD1 P2.0
    */
	
	/* By default, the PCLKSELx value is zero, thus, the PCLK for
	all the peripherals is 1/4 of the SystemFrequency. */
	/* Bit 8,9 are for UART1 */
	pclkdiv = (LPC_SC->PCLKSEL0 >> 8) & 0x03;
	switch ( pclkdiv )
	{
	  case 0x00:
	  default:
		pclk = SystemCoreClock/4;
		break;
	  case 0x01:
		pclk = SystemCoreClock;
		break; 
	  case 0x02:
		pclk = SystemCoreClock/2;
		break; 
	  case 0x03:
		pclk = SystemCoreClock/8;
		break;
	}

	LPC_UART1->LCR = 0x83;

	Fdiv = ( pclk / 16 ) / baudrate ;	/*baud rate */
    LPC_UART1->DLM = Fdiv / 256;							
    LPC_UART1->DLL = Fdiv % 256;
	//LPC_UART1->LCR = 0x03;	 /* DLAB = 0, 8N1 */
	LPC_UART1->LCR = 0x1A;		 /* DLAB = 0, 7E1 */
    LPC_UART1->FCR = 0x07;		 /* Enable and reset TX and RX FIFO. */

    /* RS485 features */
    /* set DTR as direction control (bit 3) */
    /* enable direction control (bit 4) */
    /* DTR high for sending (bit 5) */
#ifdef RS485_USE_RTS1
    LPC_UART1->RS485CTRL = 1 << 4 | 1 << 5;
#else
    LPC_UART1->RS485CTRL = 1 << 3 | 1 << 4 | 1 << 5;
#endif


   	NVIC_EnableIRQ(UART1_IRQn);

    LPC_UART1->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART1 interrupt */
    return (TRUE);
  }
  else if ( PortNum == 2 )
  {
 	  LPC_PINCON->PINSEL0 &= ~0x00F00000;

 	  /*
 	  //disable pull up
 	  LPC_PINCON->PINMODE0 &= ~(0b11<<22);
  	  LPC_PINCON->PINMODE0 |= 0b10<<22;
  	  */

 	  //LPC_PINCON->PINSEL0 |=  0x0000000A;  /* RxD3 is P0.1 and TxD3 is P0.0 */
 	  /* oder aequivalent: */

 	  LPC_PINCON->PINSEL0 |= 0b01 << 20 | 0b01 << 22;
 	  LPC_SC->PCONP |= 1<<4 | 1<<24; //Enable PCUART1 PCUART2

 	  /* By default, the PCLKSELx value is zero, thus, the PCLK for
 		all the peripherals is 1/4 of the SystemFrequency. */
 	  /* Bit 6~7 is for UART3 */
 	  pclkdiv = (LPC_SC->PCLKSEL1 >> 16) & 0x03;
 	  switch ( pclkdiv )
 	  {
 	  case 0x00:
 	  default:
 		  pclk = SystemCoreClock/4;
 		  break;
 	  case 0x01:
 		  pclk = SystemCoreClock;
 		  break;
 	  case 0x02:
 		  pclk = SystemCoreClock/2;
 		  break;
 	  case 0x03:
 		  pclk = SystemCoreClock/8;
 		  break;
 	  }

      LPC_UART2->LCR = 0x83;

 	  Fdiv = ( pclk / 16 ) / baudrate ;	/*baud rate */
 	  LPC_UART2->DLM = Fdiv / 256;
 	  LPC_UART2->DLL = Fdiv % 256;
 	  //LPC_UART2->LCR = 0x03;	 /* DLAB = 0, 8N1 */
 	  LPC_UART2->LCR = 0x1A;		 /* DLAB = 0, 7E1 */
 	  LPC_UART2->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */

 	  NVIC_EnableIRQ(UART2_IRQn);

 	  LPC_UART2->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART2 interrupt */
 	  return (TRUE);
   }
  else if ( PortNum == 3 )
  {
	  LPC_PINCON->PINSEL0 &= ~0x0000000F;
	  //LPC_PINCON->PINSEL0 |=  0x0000000A;  /* RxD3 is P0.1 and TxD3 is P0.0 */
	  /* oder aequivalent: */
	  LPC_PINCON->PINSEL0 |= 0b10 << 0 | 0b10 << 2;
	  LPC_SC->PCONP |= 1<<4 | 1<<25; //Enable PCUART1
	  /* By default, the PCLKSELx value is zero, thus, the PCLK for
		all the peripherals is 1/4 of the SystemFrequency. */
	  /* Bit 6~7 is for UART3 */
	  pclkdiv = (LPC_SC->PCLKSEL1 >> 18) & 0x03;
	  switch ( pclkdiv )
	  {
	  case 0x00:
	  default:
		  pclk = SystemCoreClock/4;
		  break;
	  case 0x01:
		  pclk = SystemCoreClock;
		  break;
	  case 0x02:
		  pclk = SystemCoreClock/2;
		  break;
	  case 0x03:
		  pclk = SystemCoreClock/8;
		  break;
	  }
	  LPC_UART3->LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
	  Fdiv = ( pclk / 16 ) / baudrate ;	/*baud rate */
	  LPC_UART3->DLM = Fdiv / 256;
	  LPC_UART3->DLL = Fdiv % 256;
	  LPC_UART3->LCR = 0x03;		/* DLAB = 0 */
	  LPC_UART3->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */

	  NVIC_EnableIRQ(UART3_IRQn);

	  LPC_UART3->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART3 interrupt */
	  return (TRUE);
  }
  return( FALSE ); 
}

/*****************************************************************************
** Function name:		UARTSend
**
** Descriptions:		Send a block of data to the UART 0 port based
**						on the data length
**
** parameters:			portNum, buffer pointer, and data length
** Returned value:		None
** 
*****************************************************************************/
void UARTSend( uint8_t portNum, uint8_t *BufferPtr, uint32_t Length )
{
  if ( portNum == 0 )
  {
    while ( Length != 0 )
    {
	  /* THRE status, contain valid data */
	  while ( !(UART0TxEmpty & 0x01) );	
	  LPC_UART0->THR = *BufferPtr;
	  UART0TxEmpty = 0;	/* not empty in the THR until it shifts out */
	  BufferPtr++;
	  Length--;
	}
  }
  else if (portNum == 1)
  {
	while ( Length != 0 )
    {
	  /* THRE status, contain valid data */
	  while ( !(UART1TxEmpty & 0x01) );	
	  LPC_UART1->THR = *BufferPtr;
	  UART1TxEmpty = 0;	/* not empty in the THR until it shifts out */
	  BufferPtr++;
	  Length--;
    }
  }
  else if (portNum == 2)
  {
	// wait until tx buffer is empty
    while ( !(UART2TxEmpty & 0x01) );
	while ( Length != 0 )
    {
	  /* THRE status, contain valid data */
	  /* since FIFO can hold 16 bytes we only
	   * need to check Emtpy bit each 16 bytes
	   * here we do the check every 14 bytes
	   */

	  if (!(Length % 14)) {
		  while ( !(UART2TxEmpty & 0x01) );
	  }

	  LPC_UART2->THR = *BufferPtr;
	  BufferPtr++;
	  Length--;
	  UART2TxEmpty = 0;
    }
  }
  else if ( portNum == 3 )
  {
    while ( Length != 0 )
    {
	  /* THRE status, contain valid data */
	  while ( !(UART3TxEmpty & 0x01) );
	  LPC_UART3->THR = *BufferPtr;
	  UART3TxEmpty = 0;	/* not empty in the THR until it shifts out */
	  BufferPtr++;
	  Length--;
	}
  }
  return;
}

void UARTSendByte(uint8_t portNum, uint8_t data) {
	if (portNum == 0) {
		/* THRE status, contain valid data */
		while ( !(UART0TxEmpty & 0x01) );
		LPC_UART0->THR = data;
		UART0TxEmpty = 0;	/* not empty in the THR until it shifts out */
	} else if (portNum == 1) {
		/* THRE status, contain valid data */
		while ( !(UART1TxEmpty & 0x01) );
		LPC_UART1->THR = data;
		UART1TxEmpty = 0;	/* not empty in the THR until it shifts out */
	} else if (portNum == 2) {
		/* THRE status, contain valid data */
		while ( !(UART2TxEmpty & 0x01) );
		LPC_UART2->THR = data;
		UART2TxEmpty = 0;	/* not empty in the THR until it shifts out */
	} else if (portNum == 3) {
		/* THRE status, contain valid data */
		while ( !(UART3TxEmpty & 0x01) );
		LPC_UART3->THR = data;
		UART3TxEmpty = 0;	/* not empty in the THR until it shifts out */
	}
}

void UARTSendString( uint8_t portNum, char* data) {
	while(*data) {
		UARTSendByte(portNum, *data++);
	}
}

void UARTSendStringln( uint8_t portNum, char* data) {
	UARTSendString(portNum, data);
	UARTSendCRLF(portNum);
}

void UARTSendNumber( uint8_t portNum, uint32_t value) {
	char buf[10];
	itoa(value, buf, 10);
	UARTSendString(portNum, (char*) buf);
}

void UARTSendNumberln( uint8_t portNum, uint32_t value) {
	UARTSendNumber(portNum, value);
	UARTSendCRLF(portNum);
}

void UARTSendCRLF( uint8_t portNum) {
	UARTSendByte(portNum, 13);
	UARTSendByte(portNum, 10);
}


uint8_t UARTTXReady(uint8_t portNum){
	if (portNum == 0) {
		return UART0TxEmpty;
	} else if (portNum == 1) {
		return UART1TxEmpty;
	} else if (portNum == 2) {
		return UART2TxEmpty;
	} else if (portNum == 3) {
		return UART3TxEmpty;
	}
	return 0;
}

void UARTUpdateMsTicks(uint32_t value) {
	UARTMsTicks = value;
}
/******************************************************************************
**                            End Of File
******************************************************************************/
