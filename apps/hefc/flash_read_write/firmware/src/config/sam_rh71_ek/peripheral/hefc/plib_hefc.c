/*******************************************************************************
Interface definition of HEFC PLIB.

Company:
Microchip Technology Inc.

File Name:
plib_hefc.h

Summary:
Interface definition of HEFC Plib.

Description:
This file defines the interface for the HEFC Plib.
It allows user to Program, Erase and lock the on-chip FLASH memory.
*******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

#include <string.h>
#include "device.h"
#include "plib_hefc.h"
#include "interrupts.h"

static uint32_t hefcStatus = 0;


void HEFC_Initialize(void)
{
}
bool HEFC_Read( uint32_t *data, uint32_t length, uint32_t address )
{   uint32_t *pAddress = (uint32_t *)address;
    (void)memcpy(data, pAddress, length);
    return true;
}

bool HEFC_SectorErase( uint32_t address )
{
    uint16_t page_number;

    /*Calculate the Page number to be passed for FARG register*/
    page_number = (uint16_t)((address - IFLASH_ADDR) / IFLASH_PAGE_SIZE);
    /* Issue the FLASH erase operation*/
    HEFC_REGS->HEFC_FCR = (HEFC_FCR_FCMD_EPA| HEFC_FCR_FARG(((uint32_t)page_number << 2U)|0x2U)| HEFC_FCR_FKEY_PASSWD);

    hefcStatus = 0;


    return true;
}

bool HEFC_PageWrite( uint32_t *data, uint32_t address )
{
    uint16_t page_number;

    /*Calculate the Page number to be passed for FARG register*/
    page_number = (uint16_t)((address - IFLASH_ADDR) / IFLASH_PAGE_SIZE);

    for (uint32_t  i = 0U; i < IFLASH_PAGE_SIZE; i += 4U)
    {
    *((uint32_t *)( IFLASH_ADDR + ( page_number * IFLASH_PAGE_SIZE ) + i )) =    *data;
    data++;
    }

    __DSB();
    __ISB();

    /* Issue the FLASH write operation*/
    HEFC_REGS->HEFC_FCR = (HEFC_FCR_FCMD_WP | HEFC_FCR_FARG((uint32_t)page_number)| HEFC_FCR_FKEY_PASSWD);

    hefcStatus = 0;


    return true;
}


void HEFC_RegionLock(uint32_t address)
{
    uint16_t page_number;

    /*Calculate the Page number to be passed for FARG register*/
    page_number = (uint16_t)((address - IFLASH_ADDR) / IFLASH_PAGE_SIZE);
    HEFC_REGS->HEFC_FCR = (HEFC_FCR_FCMD_SLB | HEFC_FCR_FARG((uint32_t)page_number)| HEFC_FCR_FKEY_PASSWD);

    hefcStatus = 0;

}

void HEFC_RegionUnlock(uint32_t address)
{
    uint16_t page_number;

    /*Calculate the Page number to be passed for FARG register*/
    page_number = (uint16_t)((address - IFLASH_ADDR) / IFLASH_PAGE_SIZE);
    HEFC_REGS->HEFC_FCR = (HEFC_FCR_FCMD_CLB | HEFC_FCR_FARG((uint32_t)page_number)| HEFC_FCR_FKEY_PASSWD);

    hefcStatus = 0;

}

bool HEFC_IsBusy(void)
{
    hefcStatus |= HEFC_REGS->HEFC_FSR;
    return (bool)((hefcStatus & HEFC_FSR_FRDY_Msk) == 0U);
}

HEFC_ERROR HEFC_ErrorGet( void )
{
    hefcStatus |= HEFC_REGS->HEFC_FSR;
    return (HEFC_ERROR)hefcStatus;
}


