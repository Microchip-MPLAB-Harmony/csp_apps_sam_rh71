/*******************************************************************************
  Controller Area Network (MCAN) Peripheral Library Source File

  Company:
    Microchip Technology Inc.

  File Name:
    plib_mcan0.c

  Summary:
    MCAN peripheral library interface.

  Description:
    This file defines the interface to the MCAN peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

  Remarks:
    None.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries.
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
//DOM-IGNORE-END
// *****************************************************************************
// *****************************************************************************
// Header Includes
// *****************************************************************************
// *****************************************************************************

#include "device.h"
#include "interrupts.h"
#include "plib_mcan0.h"

// *****************************************************************************
// *****************************************************************************
// Global Data
// *****************************************************************************
// *****************************************************************************
#define MCAN_STD_ID_Msk        0x7FFU

static MCAN_TX_FIFO_CALLBACK_OBJ mcan0TxFifoCallbackObj;
static MCAN_TX_EVENT_FIFO_CALLBACK_OBJ mcan0TxEventFifoCallbackObj;
static MCAN_TXRX_BUFFERS_CALLBACK_OBJ mcan0RxBufferCallbackObj;
static MCAN_RX_FIFO_CALLBACK_OBJ mcan0RxFifoCallbackObj[2];
static MCAN_OBJ mcan0Obj;

static const mcan_sidfe_registers_t mcan0StdFilter[] =
{
    {
        .MCAN_SIDFE_0 = MCAN_SIDFE_0_SFT(0UL) |
                  MCAN_SIDFE_0_SFID1(0x469UL) |
                  MCAN_SIDFE_0_SFID2(0x469UL) |
                  MCAN_SIDFE_0_SFEC(1UL)
    },
    {
        .MCAN_SIDFE_0 = MCAN_SIDFE_0_SFT(0UL) |
                  MCAN_SIDFE_0_SFID1(0x45aUL) |
                  MCAN_SIDFE_0_SFID2(0x0UL) |
                  MCAN_SIDFE_0_SFEC(7UL)
    },
};

static const mcan_xidfe_registers_t mcan0ExtFilter[] =
{
    {
        .MCAN_XIDFE_0 = MCAN_XIDFE_0_EFID1(0x100000a5UL) | MCAN_XIDFE_0_EFEC(7UL),
        .MCAN_XIDFE_1 = MCAN_XIDFE_1_EFID2(0x0UL) | MCAN_XIDFE_1_EFT(0UL),
    },
    {
        .MCAN_XIDFE_0 = MCAN_XIDFE_0_EFID1(0x10000096UL) | MCAN_XIDFE_0_EFEC(2UL),
        .MCAN_XIDFE_1 = MCAN_XIDFE_1_EFID2(0x10000096UL) | MCAN_XIDFE_1_EFT(0UL),
    },
};

// *****************************************************************************
// *****************************************************************************
// MCAN0 PLib Interface Routines
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* Function:
    void MCAN0_Initialize(void)

   Summary:
    Initializes given instance of the MCAN peripheral.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    None
*/
void MCAN0_Initialize(void)
{
    /* Start MCAN initialization */
    MCAN0_REGS->MCAN_CCCR = MCAN_CCCR_INIT_Msk;
    while ((MCAN0_REGS->MCAN_CCCR & MCAN_CCCR_INIT_Msk) != MCAN_CCCR_INIT_Msk)
    {
        /* Wait for initialization complete */
    }

    /* Set CCE to unlock the configuration registers */
    MCAN0_REGS->MCAN_CCCR |= MCAN_CCCR_CCE_Msk;

    /* Set Data Bit Timing and Prescaler Register */
    MCAN0_REGS->MCAN_DBTP = MCAN_DBTP_DTSEG2(6) | MCAN_DBTP_DTSEG1(16) | MCAN_DBTP_DBRP(0) | MCAN_DBTP_DSJW(5);

    /* Set Nominal Bit timing and Prescaler Register */
    MCAN0_REGS->MCAN_NBTP  = MCAN_NBTP_NTSEG2(4) | MCAN_NBTP_NTSEG1(13) | MCAN_NBTP_NBRP(4) | MCAN_NBTP_NSJW(3);

    /* Receive Buffer / FIFO Element Size Configuration Register */
    MCAN0_REGS->MCAN_RXESC = 0UL  | MCAN_RXESC_F0DS(7UL) | MCAN_RXESC_F1DS(7UL) | MCAN_RXESC_RBDS(7UL);
    /*lint -e{9048} PC lint incorrectly reports a missing 'U' Suffix */
    MCAN0_REGS->MCAN_NDAT1 = MCAN_NDAT1_Msk;
    /*lint -e{9048} PC lint incorrectly reports a missing 'U' Suffix */
    MCAN0_REGS->MCAN_NDAT2 = MCAN_NDAT2_Msk;

    /* Transmit Buffer/FIFO Element Size Configuration Register */
    MCAN0_REGS->MCAN_TXESC = MCAN_TXESC_TBDS(7UL);

    /* Global Filter Configuration Register */
    MCAN0_REGS->MCAN_GFC = MCAN_GFC_ANFS(2) | MCAN_GFC_ANFE(2);

    /* Extended ID AND Mask Register */
    MCAN0_REGS->MCAN_XIDAM = MCAN_XIDAM_Msk;

    /* Timestamp Counter Configuration Register */
    MCAN0_REGS->MCAN_TSCC = MCAN_TSCC_TCP(0) | MCAN_TSCC_TSS_TCP_INC;

    /* Set the operation mode */
    MCAN0_REGS->MCAN_CCCR = MCAN_CCCR_INIT_DISABLED | MCAN_CCCR_FDOE_ENABLED | MCAN_CCCR_BRSE_ENABLED;
    while ((MCAN0_REGS->MCAN_CCCR & MCAN_CCCR_INIT_Msk) == MCAN_CCCR_INIT_Msk)
    {
        /* Wait for initialization complete */
    }

    /* Select interrupt line */
    MCAN0_REGS->MCAN_ILS = 0x0U;

    /* Enable interrupt line */
    MCAN0_REGS->MCAN_ILE = MCAN_ILE_EINT0_Msk;

    /* Enable MCAN interrupts */
    MCAN0_REGS->MCAN_IE = MCAN_IE_BOE_Msk | MCAN_IE_TFEE_Msk | MCAN_IE_TEFNE_Msk | MCAN_IE_RF0NE_Msk | MCAN_IE_RF1NE_Msk | MCAN_IE_DRXE_Msk;

    memset(&mcan0Obj.msgRAMConfig, 0x00, sizeof(MCAN_MSG_RAM_CONFIG));
}


// *****************************************************************************
/* Function:
    bool MCAN0_MessageTransmitFifo(uint8_t numberOfMessage, MCAN_TX_BUFFER *txBuffer)

   Summary:
    Transmit multiple messages into MCAN bus from Tx FIFO.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    numberOfMessage - Total number of message.
    txBuffer        - Pointer to Tx buffer

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool MCAN0_MessageTransmitFifo(uint8_t numberOfMessage, MCAN_TX_BUFFER *txBuffer)
{
    uint8_t  *txFifo = NULL;
    uint8_t  *txBuf = (uint8_t *)txBuffer;
    uint32_t bufferNumber = 0U;
    uint8_t  tfqpi = 0U;
    uint8_t  count = 0U;

    if (((numberOfMessage < 1U) || (numberOfMessage > 1U)) || (txBuffer == NULL))
    {
        return false;
    }

    tfqpi = (uint8_t)((MCAN0_REGS->MCAN_TXFQS & MCAN_TXFQS_TFQPI_Msk) >> MCAN_TXFQS_TFQPI_Pos);

    for (count = 0; count < numberOfMessage; count++)
    {
        txFifo = (uint8_t *)((uint8_t*)mcan0Obj.msgRAMConfig.txBuffersAddress + ((uint32_t)tfqpi * MCAN0_TX_FIFO_BUFFER_ELEMENT_SIZE));

        memcpy(txFifo, txBuf, MCAN0_TX_FIFO_BUFFER_ELEMENT_SIZE);

        txBuf += MCAN0_TX_FIFO_BUFFER_ELEMENT_SIZE;
        bufferNumber |= (1UL << tfqpi);
        tfqpi++;
        if (tfqpi == 1U)
        {
            tfqpi = 0U;
        }
    }

    /* Set Transmission request */
    MCAN0_REGS->MCAN_TXBAR = bufferNumber;

    return true;
}

// *****************************************************************************
/* Function:
    uint8_t MCAN0_TxFifoFreeLevelGet(void)

   Summary:
    Returns Tx FIFO Free Level.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    None.

   Returns:
    Tx FIFO Free Level.
*/
uint8_t MCAN0_TxFifoFreeLevelGet(void)
{
    return (uint8_t)(MCAN0_REGS->MCAN_TXFQS & MCAN_TXFQS_TFFL_Msk);
}

// *****************************************************************************
/* Function:
    bool MCAN0_TxBufferIsBusy(uint8_t bufferNumber)

   Summary:
    Check if Transmission request is pending for the specific Tx buffer.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    None.

   Returns:
    true  - Transmission request is pending.
    false - Transmission request is not pending.
*/
bool MCAN0_TxBufferIsBusy(uint8_t bufferNumber)
{
    return ((MCAN0_REGS->MCAN_TXBRP & (1UL << bufferNumber)) != 0U);
}

// *****************************************************************************
/* Function:
    bool MCAN0_TxEventFifoRead(uint8_t numberOfTxEvent, MCAN_TX_EVENT_FIFO *txEventFifo)

   Summary:
    Read Tx Event FIFO for the transmitted messages.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    numberOfTxEvent - Total number of Tx Event
    txEventFifo     - Pointer to Tx Event FIFO

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool MCAN0_TxEventFifoRead(uint8_t numberOfTxEvent, MCAN_TX_EVENT_FIFO *txEventFifo)
{
    uint8_t txefgi     = 0U;
    uint8_t count      = 0U;
    uint8_t *txEvent   = NULL;
    uint8_t *txEvtFifo = (uint8_t *)txEventFifo;

    if (txEventFifo == NULL)
    {
        return false;
    }

    /* Read data from the Rx FIFO0 */
    txefgi = (uint8_t)((MCAN0_REGS->MCAN_TXEFS & MCAN_TXEFS_EFGI_Msk) >> MCAN_TXEFS_EFGI_Pos);
    for (count = 0; count < numberOfTxEvent; count++)
    {
        txEvent = (uint8_t *) ((uint8_t *)mcan0Obj.msgRAMConfig.txEventFIFOAddress + ((uint32_t)txefgi * sizeof(MCAN_TX_EVENT_FIFO)));

        memcpy(txEvtFifo, txEvent, sizeof(MCAN_TX_EVENT_FIFO));

        if ((count + 1) == numberOfTxEvent)
        {
            break;
        }
        txEvtFifo += sizeof(MCAN_TX_EVENT_FIFO);
        txefgi++;
        if (txefgi == 1U)
        {
            txefgi = 0U;
        }
    }

    /* Ack the Tx Event FIFO position */
    MCAN0_REGS->MCAN_TXEFA = MCAN_TXEFA_EFAI((uint32_t)txefgi);

    return true;
}

// *****************************************************************************
/* Function:
    bool MCAN0_MessageReceive(uint8_t bufferNumber, MCAN_RX_BUFFER *rxBuffer)

   Summary:
    Read a message from the specific Rx Buffer.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    bufferNumber - Rx buffer number
    rxBuffer     - Pointer to Rx buffer

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool MCAN0_MessageReceive(uint8_t bufferNumber, MCAN_RX_BUFFER *rxBuffer)
{
    uint8_t *rxBuf = NULL;

    if ((bufferNumber >= 1U) || (rxBuffer == NULL))
    {
        return false;
    }

    rxBuf = (uint8_t *) ((uint8_t *)mcan0Obj.msgRAMConfig.rxBuffersAddress + ((uint32_t)bufferNumber * MCAN0_RX_BUFFER_ELEMENT_SIZE));

    memcpy((uint8_t *)rxBuffer, rxBuf, MCAN0_RX_BUFFER_ELEMENT_SIZE);

    /* Clear new data flag */
    if (bufferNumber < 32U)
    {
        MCAN0_REGS->MCAN_NDAT1 = (1UL << bufferNumber);
    }
    else
    {
        MCAN0_REGS->MCAN_NDAT2 = (1UL << (bufferNumber - 32U));
    }

    return true;
}

// *****************************************************************************
/* Function:
    bool MCAN0_MessageReceiveFifo(MCAN_RX_FIFO_NUM rxFifoNum, uint8_t numberOfMessage, MCAN_RX_BUFFER *rxBuffer)

   Summary:
    Read messages from Rx FIFO0/FIFO1.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    rxFifoNum       - Rx FIFO number
    numberOfMessage - Total number of message
    rxBuffer        - Pointer to Rx buffer

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool MCAN0_MessageReceiveFifo(MCAN_RX_FIFO_NUM rxFifoNum, uint8_t numberOfMessage, MCAN_RX_BUFFER *rxBuffer)
{
    uint8_t rxgi = 0U;
    uint8_t count = 0U;
    uint8_t *rxFifo = NULL;
    uint8_t *rxBuf = (uint8_t *)rxBuffer;
    bool status = false;

    if (rxBuffer == NULL)
    {
        return status;
    }

    switch (rxFifoNum)
    {
        case MCAN_RX_FIFO_0:
            /* Read data from the Rx FIFO0 */
            rxgi = (uint8_t)((MCAN0_REGS->MCAN_RXF0S & MCAN_RXF0S_F0GI_Msk) >> MCAN_RXF0S_F0GI_Pos);
            for (count = 0; count < numberOfMessage; count++)
            {
                rxFifo = (uint8_t *) ((uint8_t *)mcan0Obj.msgRAMConfig.rxFIFO0Address + ((uint32_t)rxgi * MCAN0_RX_FIFO0_ELEMENT_SIZE));

                memcpy(rxBuf, rxFifo, MCAN0_RX_FIFO0_ELEMENT_SIZE);

                if ((count + 1) == numberOfMessage)
                {
                    break;
                }
                rxBuf += MCAN0_RX_FIFO0_ELEMENT_SIZE;
                rxgi++;
                if (rxgi == 1U)
                {
                    rxgi = 0U;
                }
            }

            /* Ack the fifo position */
            MCAN0_REGS->MCAN_RXF0A = MCAN_RXF0A_F0AI((uint32_t)rxgi);

            status = true;
            break;
        case MCAN_RX_FIFO_1:
            /* Read data from the Rx FIFO1 */
            rxgi = (uint8_t)((MCAN0_REGS->MCAN_RXF1S & MCAN_RXF1S_F1GI_Msk) >> MCAN_RXF1S_F1GI_Pos);
            for (count = 0; count < numberOfMessage; count++)
            {
                rxFifo = (uint8_t *) ((uint8_t *)mcan0Obj.msgRAMConfig.rxFIFO1Address + ((uint32_t)rxgi * MCAN0_RX_FIFO1_ELEMENT_SIZE));

                memcpy(rxBuf, rxFifo, MCAN0_RX_FIFO1_ELEMENT_SIZE);

                if ((count + 1) == numberOfMessage)
                {
                    break;
                }
                rxBuf += MCAN0_RX_FIFO1_ELEMENT_SIZE;
                rxgi++;
                if (rxgi == 1U)
                {
                    rxgi = 0U;
                }
            }
            /* Ack the fifo position */
            MCAN0_REGS->MCAN_RXF1A = MCAN_RXF1A_F1AI((uint32_t)rxgi);

            status = true;
            break;
        default:
            /* Do nothing */
            break;
    }
    return status;
}

// *****************************************************************************
/* Function:
    MCAN_ERROR MCAN0_ErrorGet(void)

   Summary:
    Returns the error during transfer.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    None.

   Returns:
    Error during transfer.
*/
MCAN_ERROR MCAN0_ErrorGet(void)
{
    MCAN_ERROR error;
    uint32_t   errorStatus = MCAN0_REGS->MCAN_PSR;

    error = (MCAN_ERROR) ((errorStatus & MCAN_PSR_LEC_Msk) | (errorStatus & MCAN_PSR_EP_Msk) | (errorStatus & MCAN_PSR_EW_Msk)
            | (errorStatus & MCAN_PSR_BO_Msk) | (errorStatus & MCAN_PSR_DLEC_Msk) | (errorStatus & MCAN_PSR_PXE_Msk));

    if ((MCAN0_REGS->MCAN_CCCR & MCAN_CCCR_INIT_Msk) == MCAN_CCCR_INIT_Msk)
    {
        MCAN0_REGS->MCAN_CCCR |= MCAN_CCCR_CCE_Msk;
        MCAN0_REGS->MCAN_CCCR = MCAN_CCCR_INIT_DISABLED | MCAN_CCCR_FDOE_ENABLED | MCAN_CCCR_BRSE_ENABLED;
        while ((MCAN0_REGS->MCAN_CCCR & MCAN_CCCR_INIT_Msk) == MCAN_CCCR_INIT_Msk)
        {
            /* Wait for initialization complete */
        }
    }

    return error;
}

// *****************************************************************************
/* Function:
    void MCAN0_ErrorCountGet(uint8_t *txErrorCount, uint8_t *rxErrorCount)

   Summary:
    Returns the transmit and receive error count during transfer.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    txErrorCount - Transmit Error Count to be received
    rxErrorCount - Receive Error Count to be received

   Returns:
    None.
*/
void MCAN0_ErrorCountGet(uint8_t *txErrorCount, uint8_t *rxErrorCount)
{
    *txErrorCount = (uint8_t)(MCAN0_REGS->MCAN_ECR & MCAN_ECR_TEC_Msk);
    *rxErrorCount = (uint8_t)((MCAN0_REGS->MCAN_ECR & MCAN_ECR_REC_Msk) >> MCAN_ECR_REC_Pos);
}

// *****************************************************************************
/* Function:
    void MCAN0_MessageRAMConfigSet(uint8_t *msgRAMConfigBaseAddress)

   Summary:
    Set the Message RAM Configuration.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    msgRAMConfigBaseAddress - Pointer to application allocated buffer base address.
                              Application must allocate buffer from non-cached
                              contiguous memory and buffer size must be
                              MCAN0_MESSAGE_RAM_CONFIG_SIZE

   Returns:
    None
*/
void MCAN0_MessageRAMConfigSet(uint8_t *msgRAMConfigBaseAddress)
{
    uint32_t offset = 0U;

    memset((void*)msgRAMConfigBaseAddress, 0x00, MCAN0_MESSAGE_RAM_CONFIG_SIZE);

    /* Set MCAN CCCR Init for Message RAM Configuration */
    MCAN0_REGS->MCAN_CCCR = MCAN_CCCR_INIT_ENABLED;
    while ((MCAN0_REGS->MCAN_CCCR & MCAN_CCCR_INIT_Msk) != MCAN_CCCR_INIT_Msk)
    {
        /* Wait for configuration complete */
    }

    /* Set CCE to unlock the configuration registers */
    MCAN0_REGS->MCAN_CCCR |= MCAN_CCCR_CCE_Msk;

    mcan0Obj.msgRAMConfig.rxFIFO0Address = (mcan_rxf0e_registers_t *)msgRAMConfigBaseAddress;
    offset = MCAN0_RX_FIFO0_SIZE;
    /* Receive FIFO 0 Configuration Register */
    MCAN0_REGS->MCAN_RXF0C = MCAN_RXF0C_F0S(1UL) | MCAN_RXF0C_F0WM(0UL) | MCAN_RXF0C_F0OM_Msk |
            MCAN_RXF0C_F0SA(((uint32_t)mcan0Obj.msgRAMConfig.rxFIFO0Address >> 2));

    mcan0Obj.msgRAMConfig.rxFIFO1Address = (mcan_rxf1e_registers_t *)(msgRAMConfigBaseAddress + offset);
    offset += MCAN0_RX_FIFO1_SIZE;
    /* Receive FIFO 1 Configuration Register */
    MCAN0_REGS->MCAN_RXF1C = MCAN_RXF1C_F1S(1UL) | MCAN_RXF1C_F1WM(0UL) | MCAN_RXF1C_F1OM_Msk |
            MCAN_RXF1C_F1SA(((uint32_t)mcan0Obj.msgRAMConfig.rxFIFO1Address >> 2));

    mcan0Obj.msgRAMConfig.rxBuffersAddress = (mcan_rxbe_registers_t *)(msgRAMConfigBaseAddress + offset);
    offset += MCAN0_RX_BUFFER_SIZE;
    MCAN0_REGS->MCAN_RXBC = MCAN_RXBC_RBSA(((uint32_t)mcan0Obj.msgRAMConfig.rxBuffersAddress >> 2));

    mcan0Obj.msgRAMConfig.txBuffersAddress = (mcan_txbe_registers_t *)(msgRAMConfigBaseAddress + offset);
    offset += MCAN0_TX_FIFO_BUFFER_SIZE;
    /* Transmit Buffer/FIFO Configuration Register */
    MCAN0_REGS->MCAN_TXBC = MCAN_TXBC_TFQS(1UL) |
            MCAN_TXBC_TBSA(((uint32_t)mcan0Obj.msgRAMConfig.txBuffersAddress >> 2));

    mcan0Obj.msgRAMConfig.txEventFIFOAddress =  (mcan_txefe_registers_t *)(msgRAMConfigBaseAddress + offset);
    offset += MCAN0_TX_EVENT_FIFO_SIZE;
    /* Transmit Event FIFO Configuration Register */
    MCAN0_REGS->MCAN_TXEFC = MCAN_TXEFC_EFWM(0UL) | MCAN_TXEFC_EFS(1UL) |
            MCAN_TXEFC_EFSA(((uint32_t)mcan0Obj.msgRAMConfig.txEventFIFOAddress >> 2));

    mcan0Obj.msgRAMConfig.stdMsgIDFilterAddress = (mcan_sidfe_registers_t *)(msgRAMConfigBaseAddress + offset);
    memcpy((void *)mcan0Obj.msgRAMConfig.stdMsgIDFilterAddress,
           (const void *)mcan0StdFilter,
           MCAN0_STD_MSG_ID_FILTER_SIZE);
    offset += MCAN0_STD_MSG_ID_FILTER_SIZE;
    /* Standard ID Filter Configuration Register */
    MCAN0_REGS->MCAN_SIDFC = MCAN_SIDFC_LSS(2UL) |
            MCAN_SIDFC_FLSSA(((uint32_t)mcan0Obj.msgRAMConfig.stdMsgIDFilterAddress >> 2));

    mcan0Obj.msgRAMConfig.extMsgIDFilterAddress = (mcan_xidfe_registers_t *)(msgRAMConfigBaseAddress + offset);
    memcpy((void *)mcan0Obj.msgRAMConfig.extMsgIDFilterAddress,
           (const void *)mcan0ExtFilter,
           MCAN0_EXT_MSG_ID_FILTER_SIZE);
    /* Extended ID Filter Configuration Register */
    MCAN0_REGS->MCAN_XIDFC = MCAN_XIDFC_LSE(2UL) |
            MCAN_XIDFC_FLESA(((uint32_t)mcan0Obj.msgRAMConfig.extMsgIDFilterAddress >> 2));

    /* Set 16-bit MSB of mcan0 base address */
    SFR_REGS->SFR_CAN0 =
        (SFR_REGS->SFR_CAN0 & ~SFR_CAN0_EXT_MEM_ADDR_Msk) |
         SFR_CAN0_EXT_MEM_ADDR(((uint32_t)msgRAMConfigBaseAddress >> 16));

    /* Reference offset variable once to remove warning about the variable not being used after increment */
    (void)offset;

    /* Complete Message RAM Configuration by clearing MCAN CCCR Init */
    MCAN0_REGS->MCAN_CCCR = MCAN_CCCR_INIT_DISABLED | MCAN_CCCR_FDOE_ENABLED | MCAN_CCCR_BRSE_ENABLED;
    while ((MCAN0_REGS->MCAN_CCCR & MCAN_CCCR_INIT_Msk) == MCAN_CCCR_INIT_Msk)
    {
        /* Wait for configuration complete */
    }
}

// *****************************************************************************
/* Function:
    bool MCAN0_StandardFilterElementSet(uint8_t filterNumber, mcan_sidfe_registers_t *stdMsgIDFilterElement)

   Summary:
    Set a standard filter element configuration.

   Precondition:
    MCAN0_Initialize and MCAN0_MessageRAMConfigSet must have been called
    for the associated MCAN instance.

   Parameters:
    filterNumber          - Standard Filter number to be configured.
    stdMsgIDFilterElement - Pointer to Standard Filter Element configuration to be set on specific filterNumber.

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool MCAN0_StandardFilterElementSet(uint8_t filterNumber, mcan_sidfe_registers_t *stdMsgIDFilterElement)
{
    if ((filterNumber > 2U) || (stdMsgIDFilterElement == NULL))
    {
        return false;
    }
    mcan0Obj.msgRAMConfig.stdMsgIDFilterAddress[filterNumber - 1U].MCAN_SIDFE_0 = stdMsgIDFilterElement->MCAN_SIDFE_0;

    return true;
}

// *****************************************************************************
/* Function:
    bool MCAN0_StandardFilterElementGet(uint8_t filterNumber, mcan_sidfe_registers_t *stdMsgIDFilterElement)

   Summary:
    Get a standard filter element configuration.

   Precondition:
    MCAN0_Initialize and MCAN0_MessageRAMConfigSet must have been called
    for the associated MCAN instance.

   Parameters:
    filterNumber          - Standard Filter number to get filter configuration.
    stdMsgIDFilterElement - Pointer to Standard Filter Element configuration for storing filter configuration.

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool MCAN0_StandardFilterElementGet(uint8_t filterNumber, mcan_sidfe_registers_t *stdMsgIDFilterElement)
{
    if ((filterNumber > 2U) || (stdMsgIDFilterElement == NULL))
    {
        return false;
    }
    stdMsgIDFilterElement->MCAN_SIDFE_0 = mcan0Obj.msgRAMConfig.stdMsgIDFilterAddress[filterNumber - 1U].MCAN_SIDFE_0;

    return true;
}

// *****************************************************************************
/* Function:
    bool MCAN0_ExtendedFilterElementSet(uint8_t filterNumber, mcan_xidfe_registers_t *extMsgIDFilterElement)

   Summary:
    Set a Extended filter element configuration.

   Precondition:
    MCAN0_Initialize and MCAN0_MessageRAMConfigSet must have been called
    for the associated MCAN instance.

   Parameters:
    filterNumber          - Extended Filter number to be configured.
    extMsgIDFilterElement - Pointer to Extended Filter Element configuration to be set on specific filterNumber.

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool MCAN0_ExtendedFilterElementSet(uint8_t filterNumber, mcan_xidfe_registers_t *extMsgIDFilterElement)
{
    if ((filterNumber > 2U) || (extMsgIDFilterElement == NULL))
    {
        return false;
    }
    mcan0Obj.msgRAMConfig.extMsgIDFilterAddress[filterNumber - 1U].MCAN_XIDFE_0 = extMsgIDFilterElement->MCAN_XIDFE_0;
    mcan0Obj.msgRAMConfig.extMsgIDFilterAddress[filterNumber - 1U].MCAN_XIDFE_1 = extMsgIDFilterElement->MCAN_XIDFE_1;

    return true;
}

// *****************************************************************************
/* Function:
    bool MCAN0_ExtendedFilterElementGet(uint8_t filterNumber, mcan_xidfe_registers_t *extMsgIDFilterElement)

   Summary:
    Get a Extended filter element configuration.

   Precondition:
    MCAN0_Initialize and MCAN0_MessageRAMConfigSet must have been called
    for the associated MCAN instance.

   Parameters:
    filterNumber          - Extended Filter number to get filter configuration.
    extMsgIDFilterElement - Pointer to Extended Filter Element configuration for storing filter configuration.

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool MCAN0_ExtendedFilterElementGet(uint8_t filterNumber, mcan_xidfe_registers_t *extMsgIDFilterElement)
{
    if ((filterNumber > 2U) || (extMsgIDFilterElement == NULL))
    {
        return false;
    }
    extMsgIDFilterElement->MCAN_XIDFE_0 = mcan0Obj.msgRAMConfig.extMsgIDFilterAddress[filterNumber - 1U].MCAN_XIDFE_0;
    extMsgIDFilterElement->MCAN_XIDFE_1 = mcan0Obj.msgRAMConfig.extMsgIDFilterAddress[filterNumber - 1U].MCAN_XIDFE_1;

    return true;
}

void MCAN0_SleepModeEnter(void)
{
    MCAN0_REGS->MCAN_CCCR |=  MCAN_CCCR_CSR_Msk;
    while ((MCAN0_REGS->MCAN_CCCR & MCAN_CCCR_CSA_Msk) != MCAN_CCCR_CSA_Msk)
    {
        /* Wait for clock stop request to complete */
    }
}

void MCAN0_SleepModeExit(void)
{
    MCAN0_REGS->MCAN_CCCR &=  ~MCAN_CCCR_CSR_Msk;
    while ((MCAN0_REGS->MCAN_CCCR & MCAN_CCCR_CSA_Msk) == MCAN_CCCR_CSA_Msk)
    {
        /* Wait for no clock stop */
    }
    MCAN0_REGS->MCAN_CCCR &= ~MCAN_CCCR_INIT_Msk;
    while ((MCAN0_REGS->MCAN_CCCR & MCAN_CCCR_INIT_Msk) == MCAN_CCCR_INIT_Msk)
    {
        /* Wait for initialization complete */
    }
}


// *****************************************************************************
/* Function:
    void MCAN0_TxFifoCallbackRegister(MCAN_TX_FIFO_CALLBACK callback, uintptr_t contextHandle)

   Summary:
    Sets the pointer to the function (and it's context) to be called when the
    given MCAN's transfer events occur.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    callback - A pointer to a function with a calling signature defined
    by the MCAN_TX_FIFO_CALLBACK data type.

    contextHandle - A value (usually a pointer) passed (unused) into the function
    identified by the callback parameter.

   Returns:
    None.
*/
void MCAN0_TxFifoCallbackRegister(MCAN_TX_FIFO_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback == NULL)
    {
        return;
    }

    mcan0TxFifoCallbackObj.callback = callback;
    mcan0TxFifoCallbackObj.context = contextHandle;
}

// *****************************************************************************
/* Function:
    void MCAN0_TxEventFifoCallbackRegister(MCAN_TX_EVENT_FIFO_CALLBACK callback, uintptr_t contextHandle)

   Summary:
    Sets the pointer to the function (and it's context) to be called when the
    given MCAN's transfer events occur.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    callback - A pointer to a function with a calling signature defined
    by the MCAN_TX_EVENT_FIFO_CALLBACK data type.

    contextHandle - A value (usually a pointer) passed (unused) into the function
    identified by the callback parameter.

   Returns:
    None.
*/
void MCAN0_TxEventFifoCallbackRegister(MCAN_TX_EVENT_FIFO_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback == NULL)
    {
        return;
    }

    mcan0TxEventFifoCallbackObj.callback = callback;
    mcan0TxEventFifoCallbackObj.context = contextHandle;
}

// *****************************************************************************
/* Function:
    void MCAN0_RxBuffersCallbackRegister(MCAN_TXRX_BUFFERS_CALLBACK callback, uintptr_t contextHandle)

   Summary:
    Sets the pointer to the function (and it's context) to be called when the
    given MCAN's transfer events occur.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    callback - A pointer to a function with a calling signature defined
    by the MCAN_TXRX_BUFFERS_CALLBACK data type.

    contextHandle - A value (usually a pointer) passed (unused) into the function
    identified by the callback parameter.

   Returns:
    None.
*/
void MCAN0_RxBuffersCallbackRegister(MCAN_TXRX_BUFFERS_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback == NULL)
    {
        return;
    }

    mcan0RxBufferCallbackObj.callback = callback;
    mcan0RxBufferCallbackObj.context = contextHandle;
}

// *****************************************************************************
/* Function:
    void MCAN0_RxFifoCallbackRegister(MCAN_RX_FIFO_NUM rxFifoNum, MCAN_RX_FIFO_CALLBACK callback, uintptr_t contextHandle)

   Summary:
    Sets the pointer to the function (and it's context) to be called when the
    given MCAN's transfer events occur.

   Precondition:
    MCAN0_Initialize must have been called for the associated MCAN instance.

   Parameters:
    rxFifoNum - Rx FIFO Number

    callback  - A pointer to a function with a calling signature defined
    by the MCAN_RX_FIFO_CALLBACK data type.

    contextHandle - A value (usually a pointer) passed (unused) into the function
    identified by the callback parameter.

   Returns:
    None.
*/
void MCAN0_RxFifoCallbackRegister(MCAN_RX_FIFO_NUM rxFifoNum, MCAN_RX_FIFO_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback == NULL)
    {
        return;
    }

    mcan0RxFifoCallbackObj[rxFifoNum].callback = callback;
    mcan0RxFifoCallbackObj[rxFifoNum].context = contextHandle;
}

// *****************************************************************************
/* Function:
    void MCAN0_INT0_InterruptHandler(void)

   Summary:
    MCAN0 Peripheral Interrupt Handler.

   Description:
    This function is MCAN0 Peripheral Interrupt Handler and will
    called on every MCAN0 interrupt.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    None.

   Remarks:
    The function is called as peripheral instance's interrupt handler if the
    instance interrupt is enabled. If peripheral instance's interrupt is not
    enabled user need to call it from the main while loop of the application.
*/
void MCAN0_INT0_InterruptHandler(void)
{
    uint32_t newData1 = 0U;
    uint8_t bufferNumber = 0U;
    uint8_t numberOfMessage = 0;
    uint8_t numberOfTxEvent = 0;

    uint32_t ir = MCAN0_REGS->MCAN_IR;

    /* Check if error occurred */
    if ((ir & MCAN_IR_BO_Msk) != 0U)
    {
        MCAN0_REGS->MCAN_IR = MCAN_IR_BO_Msk;
    }
    /* New Message in Rx FIFO 0 */
    if ((ir & MCAN_IR_RF0N_Msk) != 0U)
    {
        MCAN0_REGS->MCAN_IR = MCAN_IR_RF0N_Msk;

        numberOfMessage = (uint8_t)(MCAN0_REGS->MCAN_RXF0S & MCAN_RXF0S_F0FL_Msk);

        if (mcan0RxFifoCallbackObj[MCAN_RX_FIFO_0].callback != NULL)
        {
            mcan0RxFifoCallbackObj[MCAN_RX_FIFO_0].callback(numberOfMessage, mcan0RxFifoCallbackObj[MCAN_RX_FIFO_0].context);
        }
    }
    /* New Message in Rx FIFO 1 */
    if ((ir & MCAN_IR_RF1N_Msk) != 0U)
    {
        MCAN0_REGS->MCAN_IR = MCAN_IR_RF1N_Msk;

        numberOfMessage = (uint8_t)(MCAN0_REGS->MCAN_RXF1S & MCAN_RXF1S_F1FL_Msk);

        if (mcan0RxFifoCallbackObj[MCAN_RX_FIFO_1].callback != NULL)
        {
            mcan0RxFifoCallbackObj[MCAN_RX_FIFO_1].callback(numberOfMessage, mcan0RxFifoCallbackObj[MCAN_RX_FIFO_1].context);
        }
    }
    /* New Message in Dedicated Rx Buffer */
    if ((ir & MCAN_IR_DRX_Msk) != 0U)
    {
        MCAN0_REGS->MCAN_IR = MCAN_IR_DRX_Msk;

        newData1 = MCAN0_REGS->MCAN_NDAT1;
        if (newData1 != 0U)
        {
            for (bufferNumber = 0U; bufferNumber < 1U; bufferNumber++)
            {
                if ((newData1 & (1UL << bufferNumber)) == (1UL << bufferNumber))
                {
                    if (mcan0RxBufferCallbackObj.callback != NULL)
                    {
                        mcan0RxBufferCallbackObj.callback(bufferNumber, mcan0RxBufferCallbackObj.context);
                    }
                }
            }
        }
    }

    /* TX FIFO is empty */
    if ((ir & MCAN_IR_TFE_Msk) != 0U)
    {
        MCAN0_REGS->MCAN_IR = MCAN_IR_TFE_Msk;
        if (mcan0TxFifoCallbackObj.callback != NULL)
        {
            mcan0TxFifoCallbackObj.callback(mcan0TxFifoCallbackObj.context);
        }
    }
    /* Tx Event FIFO new entry */
    if ((ir & MCAN_IR_TEFN_Msk) != 0U)
    {
        MCAN0_REGS->MCAN_IR = MCAN_IR_TEFN_Msk;

        numberOfTxEvent = (uint8_t)(MCAN0_REGS->MCAN_TXEFS & MCAN_TXEFS_EFFL_Msk);

        if (mcan0TxEventFifoCallbackObj.callback != NULL)
        {
            mcan0TxEventFifoCallbackObj.callback(numberOfTxEvent, mcan0TxEventFifoCallbackObj.context);
        }
    }
}

/*******************************************************************************
 End of File
*/
