/*******************************************************************************
  FLEXCOM1 USART PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_flexcom1_usart.c

  Summary:
    FLEXCOM1 USART PLIB Implementation File

  Description
    This file defines the interface to the FLEXCOM1 USART peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

  Remarks:
    None.
*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/* This section lists the other files that are included in this file.
*/
#include "plib_flexcom1_usart.h"
#include "interrupts.h"


#define FLEXCOM_USART_RHR_8BIT_REG      (*(volatile uint8_t* const)((FLEXCOM1_BASE_ADDRESS + FLEX_US_RHR_REG_OFST)))
#define FLEXCOM_USART_RHR_9BIT_REG      (*(volatile uint16_t* const)((FLEXCOM1_BASE_ADDRESS + FLEX_US_RHR_REG_OFST)))

#define FLEXCOM_USART_THR_8BIT_REG      (*(volatile uint8_t* const)((FLEXCOM1_BASE_ADDRESS + FLEX_US_THR_REG_OFST)))
#define FLEXCOM_USART_THR_9BIT_REG      (*(volatile uint16_t* const)((FLEXCOM1_BASE_ADDRESS + FLEX_US_THR_REG_OFST)))

// *****************************************************************************
// *****************************************************************************
// Section: FLEXCOM1 USART Implementation
// *****************************************************************************
// *****************************************************************************
volatile static FLEXCOM_USART_OBJECT flexcom1UsartObj;

void static FLEXCOM1_USART_ErrorClear( void )
{
    if ((FLEXCOM1_REGS->FLEX_US_CSR & (FLEX_US_CSR_OVRE_Msk | FLEX_US_CSR_FRAME_Msk | FLEX_US_CSR_PARE_Msk)) != 0U)
    {
        /* Clear the error flags */
        FLEXCOM1_REGS->FLEX_US_CR = FLEX_US_CR_RSTSTA_Msk;

        /* Flush existing error bytes from the RX FIFO */
        while((FLEXCOM1_REGS->FLEX_US_CSR & FLEX_US_CSR_RXRDY_Msk) != 0U)
        {
            if ((FLEXCOM1_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
            {
                (void)(FLEXCOM_USART_RHR_9BIT_REG);
            }
            else
            {
                (void)(FLEXCOM_USART_RHR_8BIT_REG);
            }
        }
    }
}


void static __attribute__((used)) FLEXCOM1_USART_ISR_RX_Handler( void )
{
    size_t rxProcessedSize = flexcom1UsartObj.rxProcessedSize;
    size_t rxSize = flexcom1UsartObj.rxSize;

    if(flexcom1UsartObj.rxBusyStatus == true)
    {
        while(((FLEXCOM1_REGS->FLEX_US_CSR & FLEX_US_CSR_RXRDY_Msk) != 0U) && (rxProcessedSize < rxSize))
        {
            if ((FLEXCOM1_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
            {
                ((uint16_t*)flexcom1UsartObj.rxBuffer)[rxProcessedSize] = (FLEXCOM_USART_RHR_9BIT_REG & (uint16_t)FLEX_US_RHR_RXCHR_Msk);
                rxProcessedSize++;
            }
            else
            {
                ((uint8_t*)flexcom1UsartObj.rxBuffer)[rxProcessedSize] = (FLEXCOM_USART_RHR_8BIT_REG);
                rxProcessedSize++;
            }
        }

        flexcom1UsartObj.rxProcessedSize = rxProcessedSize;

        /* Check if the buffer is done */
        if(rxProcessedSize >= rxSize)
        {
            flexcom1UsartObj.rxBusyStatus = false;

            /* Disable Read, Overrun, Parity and Framing error interrupts */
            FLEXCOM1_REGS->FLEX_US_IDR = (FLEX_US_IDR_RXRDY_Msk | FLEX_US_IDR_FRAME_Msk | FLEX_US_IDR_PARE_Msk | FLEX_US_IDR_OVRE_Msk);

            if(flexcom1UsartObj.rxCallback != NULL)
            {
                uintptr_t rxContext = flexcom1UsartObj.rxContext;

                flexcom1UsartObj.rxCallback(rxContext);
            }
        }
    }
    else
    {
        /* Nothing to process */
    }
}

void static __attribute__((used)) FLEXCOM1_USART_ISR_TX_Handler( void )
{
    if(flexcom1UsartObj.txBusyStatus == true)
    {
        size_t txProcessedSize = flexcom1UsartObj.txProcessedSize;
        size_t txSize = flexcom1UsartObj.txSize;

        while( ((FLEXCOM1_REGS->FLEX_US_CSR & FLEX_US_CSR_TXRDY_Msk) != 0U) && (txProcessedSize < txSize))
        {
            if ((FLEXCOM1_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
            {
                FLEXCOM_USART_THR_9BIT_REG =  ((uint16_t*)flexcom1UsartObj.txBuffer)[txProcessedSize] & (uint16_t)FLEX_US_THR_TXCHR_Msk;
                txProcessedSize++;
            }
            else
            {
                FLEXCOM_USART_THR_8BIT_REG =  ((uint8_t*)flexcom1UsartObj.txBuffer)[txProcessedSize];
                txProcessedSize++;
            }
        }

        flexcom1UsartObj.txProcessedSize = txProcessedSize;

        /* Check if the buffer is done */
        if(txProcessedSize >= txSize)
        {
            flexcom1UsartObj.txBusyStatus = false;

            FLEXCOM1_REGS->FLEX_US_IDR = FLEX_US_IDR_TXRDY_Msk;

            if(flexcom1UsartObj.txCallback != NULL)
            {
                uintptr_t txContext = flexcom1UsartObj.txContext;

                flexcom1UsartObj.txCallback(txContext);
            }
        }
    }
    else
    {
        /* Nothing to process */
    }
}

void __attribute__((used)) FLEXCOM1_InterruptHandler( void )
{
    /* Channel status */
    uint32_t channelStatus = FLEXCOM1_REGS->FLEX_US_CSR;

    uint32_t interruptMask = FLEXCOM1_REGS->FLEX_US_IMR;


    /* Error status */
    uint32_t errorStatus = (channelStatus & (FLEX_US_CSR_OVRE_Msk | FLEX_US_CSR_FRAME_Msk | FLEX_US_CSR_PARE_Msk));

    if((errorStatus != 0U) && ((interruptMask & (FLEX_US_IMR_RXRDY_Msk | FLEX_US_IMR_FRAME_Msk | FLEX_US_IMR_PARE_Msk | FLEX_US_IMR_OVRE_Msk)) != 0U))
    {
        /* Save error to report it later */
        flexcom1UsartObj.errorStatus = errorStatus;

        /* Clear error flags and flush the error data */
        FLEXCOM1_USART_ErrorClear();

        /* Transfer complete. Clear the busy flag. */
        flexcom1UsartObj.rxBusyStatus = false;


        /* Disable Read, Overrun, Parity and Framing error interrupts */
        FLEXCOM1_REGS->FLEX_US_IDR = (FLEX_US_IDR_RXRDY_Msk | FLEX_US_IDR_FRAME_Msk | FLEX_US_IDR_PARE_Msk | FLEX_US_IDR_OVRE_Msk);

        /* USART errors are normally associated with the receiver, hence calling receiver context */
        if( flexcom1UsartObj.rxCallback != NULL )
        {
            uintptr_t rxContext = flexcom1UsartObj.rxContext;

            flexcom1UsartObj.rxCallback(rxContext);
        }
    }

    /* Receiver status */
    if((channelStatus & FLEX_US_CSR_RXRDY_Msk) != 0U)
    {
        FLEXCOM1_USART_ISR_RX_Handler();
    }


    /* Transmitter status */
    if((channelStatus & FLEX_US_CSR_TXRDY_Msk) != 0U)
    {
        FLEXCOM1_USART_ISR_TX_Handler();
    }

}


void FLEXCOM1_USART_Initialize( void )
{
    /* Set FLEXCOM USART operating mode */
    FLEXCOM1_REGS->FLEX_MR = FLEX_MR_OPMODE_USART;

    /* Reset FLEXCOM1 USART */
    FLEXCOM1_REGS->FLEX_US_CR = (FLEX_US_CR_RSTRX_Msk | FLEX_US_CR_RSTTX_Msk | FLEX_US_CR_RSTSTA_Msk );


    /* Setup transmitter timeguard register */
    FLEXCOM1_REGS->FLEX_US_TTGR = 0;

    /* Configure FLEXCOM1 USART mode */
    FLEXCOM1_REGS->FLEX_US_MR = ( FLEX_US_MR_USART_MODE_NORMAL | FLEX_US_MR_USCLKS_MCK | FLEX_US_MR_CHRL_8_BIT | FLEX_US_MR_PAR_NO | FLEX_US_MR_NBSTOP_1_BIT | (0UL << FLEX_US_MR_OVER_Pos));

    /* Configure FLEXCOM1 USART Baud Rate */
    FLEXCOM1_REGS->FLEX_US_BRGR = FLEX_US_BRGR_CD(27) | FLEX_US_BRGR_FP(1);

    /* Enable FLEXCOM1 USART */
    FLEXCOM1_REGS->FLEX_US_CR = (FLEX_US_CR_TXEN_Msk | FLEX_US_CR_RXEN_Msk);


    /* Initialize instance object */
    flexcom1UsartObj.rxBuffer = NULL;
    flexcom1UsartObj.rxSize = 0;
    flexcom1UsartObj.rxProcessedSize = 0;
    flexcom1UsartObj.rxBusyStatus = false;
    flexcom1UsartObj.rxCallback = NULL;
    flexcom1UsartObj.errorStatus = FLEXCOM_USART_ERROR_NONE;
    flexcom1UsartObj.txBuffer = NULL;
    flexcom1UsartObj.txSize = 0;
    flexcom1UsartObj.txProcessedSize = 0;
    flexcom1UsartObj.txBusyStatus = false;
    flexcom1UsartObj.txCallback = NULL;
}

FLEXCOM_USART_ERROR FLEXCOM1_USART_ErrorGet( void )
{
    FLEXCOM_USART_ERROR errorStatus = flexcom1UsartObj.errorStatus;

    flexcom1UsartObj.errorStatus = FLEXCOM_USART_ERROR_NONE;

    /* All errors are cleared, but send the previous error state */
    return errorStatus;
}

static void FLEXCOM1_USART_BaudCalculate(uint32_t srcClkFreq, uint32_t reqBaud, uint8_t overSamp, uint32_t* cd, uint32_t* fp, uint32_t* baudError)
{
    uint32_t actualBaud = 0U;

    *cd = srcClkFreq / (reqBaud * 8U * (2U - overSamp));

    if (*cd > 0U)
    {
        *fp = ((srcClkFreq / (reqBaud * (2U - (uint32_t)overSamp))) - ((*cd) * 8U));
        actualBaud = (srcClkFreq / (((*cd) * 8U) + (*fp))) / (2U - overSamp);
        *baudError = ((100U * actualBaud)/reqBaud) - 100U;
    }
}

bool FLEXCOM1_USART_SerialSetup( FLEXCOM_USART_SERIAL_SETUP *setup, uint32_t srcClkFreq )
{
    uint32_t baud = 0;
    uint32_t overSampVal = 0;
    uint32_t usartMode;
    uint32_t cd0, fp0, cd1, fp1, baudError0, baudError1;
    bool status = false;

    cd0 = fp0 = cd1 = fp1 = baudError0 = baudError1 = 0U;

    bool rxBusyStatus = flexcom1UsartObj.rxBusyStatus;

    if((flexcom1UsartObj.txBusyStatus == true) || (rxBusyStatus == true))
    {
        /* Transaction is in progress, so return without updating settings */
        return false;
    }

    if (setup != NULL)
    {
        baud = setup->baudRate;

        if(srcClkFreq == 0U)
        {
            srcClkFreq = FLEXCOM1_USART_FrequencyGet();
        }

        /* Calculate baud register values for 8x/16x oversampling values */

        FLEXCOM1_USART_BaudCalculate(srcClkFreq, baud, 0, &cd0, &fp0, &baudError0);
        FLEXCOM1_USART_BaudCalculate(srcClkFreq, baud, 1, &cd1, &fp1, &baudError1);

        if ( (!(cd0 > 0U && cd0 <= 65535U)) && (!(cd1 > 0U && cd1 <= 65535U)) )
        {
            /* Requested baud cannot be generated with current clock settings */
            return status;
        }

        if ( ((cd0 > 0U) && (cd0 <= 65535U)) && ((cd1 > 0U) && (cd1 <= 65535U)) )
        {
            /* Requested baud can be generated with both 8x and 16x oversampling. Select the one with less % error. */
            if (baudError1 < baudError0)
            {
                cd0 = cd1;
                fp0 = fp1;
                overSampVal = (1UL << FLEX_US_MR_OVER_Pos) & FLEX_US_MR_OVER_Msk;
            }
        }
        else
        {
            /* Requested baud can be generated with either with 8x oversampling or with 16x oversampling. Select valid one. */
            if ((cd1 > 0U )&& (cd1 <= 65535U))
            {
                cd0 = cd1;
                fp0 = fp1;
                overSampVal = (1UL << FLEX_US_MR_OVER_Pos) & FLEX_US_MR_OVER_Msk;
            }
        }

        /* Configure FLEXCOM1 USART mode */
        usartMode = FLEXCOM1_REGS->FLEX_US_MR;
        usartMode &= ~(FLEX_US_MR_CHRL_Msk | FLEX_US_MR_MODE9_Msk | FLEX_US_MR_PAR_Msk | FLEX_US_MR_NBSTOP_Msk | FLEX_US_MR_OVER_Msk);
        FLEXCOM1_REGS->FLEX_US_MR = usartMode | ((uint32_t)setup->dataWidth | (uint32_t)setup->parity | (uint32_t)setup->stopBits | overSampVal);

        /* Configure FLEXCOM1 USART Baud Rate */
        FLEXCOM1_REGS->FLEX_US_BRGR = FLEX_US_BRGR_CD(cd0) | FLEX_US_BRGR_FP(fp0);
        status = true;
    }

    return status;
}

bool FLEXCOM1_USART_Read( void *buffer, const size_t size )
{
    bool status = false;
    if(buffer != NULL)
    {
        /* Check if receive request is in progress */
        if(flexcom1UsartObj.rxBusyStatus == false)
        {
            /* Clear errors that may have got generated when there was no active read request pending */
            FLEXCOM1_USART_ErrorClear();

            /* Clear the errors related to pervious read requests */
            flexcom1UsartObj.errorStatus = FLEXCOM_USART_ERROR_NONE;

            flexcom1UsartObj.rxBuffer = buffer;
            flexcom1UsartObj.rxSize = size;
            flexcom1UsartObj.rxProcessedSize = 0;
            flexcom1UsartObj.rxBusyStatus = true;
            status = true;


            /* Enable Read, Overrun, Parity and Framing error interrupts */
            FLEXCOM1_REGS->FLEX_US_IER = (FLEX_US_IER_RXRDY_Msk | FLEX_US_IER_FRAME_Msk | FLEX_US_IER_PARE_Msk | FLEX_US_IER_OVRE_Msk);

        }
    }

    return status;
}

bool FLEXCOM1_USART_Write( void *buffer, const size_t size )
{
    bool status = false;
    if(buffer != NULL)
    {
        /* Check if transmit request is in progress */
        if(flexcom1UsartObj.txBusyStatus == false)
        {
            flexcom1UsartObj.txBuffer = buffer;
            flexcom1UsartObj.txSize = size;
            flexcom1UsartObj.txProcessedSize = 0;
            flexcom1UsartObj.txBusyStatus = true;
            status = true;


            size_t txProcessedSize = flexcom1UsartObj.txProcessedSize;
            size_t txSize = flexcom1UsartObj.txSize;

            /* Initiate the transfer by sending first byte */
            while(((FLEXCOM1_REGS->FLEX_US_CSR & FLEX_US_CSR_TXRDY_Msk) != 0U) && (txProcessedSize < txSize))
            {
                if ((FLEXCOM1_REGS->FLEX_US_MR & FLEX_US_MR_MODE9_Msk) != 0U)
                {
                    FLEXCOM_USART_THR_9BIT_REG = ((uint16_t*)flexcom1UsartObj.txBuffer)[txProcessedSize] & (uint16_t)FLEX_US_THR_TXCHR_Msk;
                }
                else
                {
                    FLEXCOM_USART_THR_8BIT_REG = ((uint8_t*)flexcom1UsartObj.txBuffer)[txProcessedSize];
                }
                txProcessedSize++;
            }

            flexcom1UsartObj.txProcessedSize = txProcessedSize;

            FLEXCOM1_REGS->FLEX_US_IER = FLEX_US_IER_TXRDY_Msk;
        }
    }

    return status;
}

void FLEXCOM1_USART_WriteCallbackRegister( FLEXCOM_USART_CALLBACK callback, uintptr_t context )
{
    flexcom1UsartObj.txCallback = callback;
    flexcom1UsartObj.txContext = context;
}

void FLEXCOM1_USART_ReadCallbackRegister( FLEXCOM_USART_CALLBACK callback, uintptr_t context )
{
    flexcom1UsartObj.rxCallback = callback;
    flexcom1UsartObj.rxContext = context;
}

bool FLEXCOM1_USART_WriteIsBusy( void )
{
    return flexcom1UsartObj.txBusyStatus;
}

bool FLEXCOM1_USART_ReadIsBusy( void )
{
    return flexcom1UsartObj.rxBusyStatus;
}

size_t FLEXCOM1_USART_WriteCountGet( void )
{
    return flexcom1UsartObj.txProcessedSize;
}

size_t FLEXCOM1_USART_ReadCountGet( void )
{
    return flexcom1UsartObj.rxProcessedSize;
}

bool FLEXCOM1_USART_ReadAbort(void)
{
    if (flexcom1UsartObj.rxBusyStatus == true)
    {
        /* Disable Read, Overrun, Parity and Framing error interrupts */
        FLEXCOM1_REGS->FLEX_US_IDR = (FLEX_US_IDR_RXRDY_Msk | FLEX_US_IDR_FRAME_Msk | FLEX_US_IDR_PARE_Msk | FLEX_US_IDR_OVRE_Msk);

        flexcom1UsartObj.rxBusyStatus = false;

        /* If required application should read the num bytes processed prior to calling the read abort API */
        flexcom1UsartObj.rxSize = 0U;
        flexcom1UsartObj.rxProcessedSize = 0U;
    }

    return true;
}


bool FLEXCOM1_USART_TransmitComplete( void )
{
    return ((FLEXCOM1_REGS->FLEX_US_CSR & FLEX_US_CSR_TXEMPTY_Msk) != 0U);
}

