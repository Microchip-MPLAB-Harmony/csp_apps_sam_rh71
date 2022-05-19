/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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

#include "device.h"
#include "plib_clk.h"
#include <stdbool.h>
#include <string.h>

static volatile uint32_t gpnvm_table[4] = {0};
static bool crc_valid = false;

static bool checkGpnvmWordCrc(void)
{
    uint8_t count = 0U;

    /* Read GPNVM bits */
    HEFC_REGS->HEFC_FCR = (HEFC_FCR_FKEY(0x5AU) | HEFC_FCR_FCMD_GGPB );
    while ( (HEFC_REGS-> HEFC_FSR & HEFC_FSR_FRDY_Msk) != HEFC_FSR_FRDY_Msk)
    {
        /* Waiting for the Flash Ready state */
    }
    for (count = 0U; count < 4U; count++)
    {
        gpnvm_table[count] = HEFC_REGS->HEFC_FRR;
    }

    __DSB();
    __ISB();

    if ( (gpnvm_table[1] == gpnvm_table[2]) && (gpnvm_table[2] == gpnvm_table[3]))
    {
        crc_valid = true;
    }
    return crc_valid;
}
static void rc2ckTrimCalibrationSequence(void)
{
    // Compute calibration value using ftrim and ttrim from GPNVM_WORD
    uint32_t calibSec4 = ( ((gpnvm_table[1] & FUSES_USER_WORD_1_RC2_FTRIM_4MHZ_Msk) >> FUSES_USER_WORD_1_RC2_FTRIM_4MHZ_Pos) << 2U ) |\
                          ((gpnvm_table[1] & FUSES_USER_WORD_1_RC2_TTRIM_Msk) >> FUSES_USER_WORD_1_RC2_TTRIM_Pos) ;
    uint32_t calibSec8 = ( ((gpnvm_table[1] & FUSES_USER_WORD_1_RC2_FTRIM_8MHZ_Msk) >> FUSES_USER_WORD_1_RC2_FTRIM_8MHZ_Pos) << 2U ) |\
                          ((gpnvm_table[1] & FUSES_USER_WORD_1_RC2_TTRIM_Msk) >> FUSES_USER_WORD_1_RC2_TTRIM_Pos) ;
    uint32_t calibSec10 = ( ((gpnvm_table[1] & FUSES_USER_WORD_1_RC2_FTRIM_10MHZ_Msk) >> FUSES_USER_WORD_1_RC2_FTRIM_10MHZ_Pos) << 2U ) |\
                  ((gpnvm_table[1] & FUSES_USER_WORD_1_RC2_TTRIM_Msk) >> FUSES_USER_WORD_1_RC2_TTRIM_Pos) ;
    uint32_t calibSec12 = ( ((gpnvm_table[1] & FUSES_USER_WORD_1_RC2_FTRIM_12MHZ_Msk) >> FUSES_USER_WORD_1_RC2_FTRIM_12MHZ_Pos) << 2U ) |\
                  ((gpnvm_table[1] & FUSES_USER_WORD_1_RC2_TTRIM_Msk) >> FUSES_USER_WORD_1_RC2_TTRIM_Pos) ;

    /* Configure the RC Oscillator frequency to select 4MHz */
    PMC_REGS->PMC_OSC2 = (PMC_REGS->PMC_OSC2 & ~PMC_OSC2_OSCRCF_Msk) | PMC_OSC2_KEY_PASSWD | PMC_OSC2_OSCRCF_4_MHZ;

    __DSB();
    __ISB();

    /* Set SEL for 8/10/12 MHz */
    PMC_REGS->PMC_OCR2 |= (PMC_OCR2_SEL8_Msk | PMC_OCR2_SEL10_Msk | PMC_OCR2_SEL12_Msk);

    /* Set Calib values for 8/10/12 MHz */
    PMC_REGS->PMC_OSC2 = (PMC_REGS->PMC_OSC2 | CKGR_MOR_KEY_PASSWD) | PMC_OSC2_EN_WR_CALIB_Msk;
    PMC_REGS->PMC_OCR2 = (PMC_REGS->PMC_OCR2 & ~(PMC_OCR2_CAL8_Msk | PMC_OCR2_CAL10_Msk | PMC_OCR2_CAL12_Msk) ) | \
                         PMC_OCR2_CAL8(calibSec8) | PMC_OCR2_CAL10(calibSec10) | PMC_OCR2_CAL12(calibSec12);
    PMC_REGS->PMC_OSC2 = (PMC_REGS->PMC_OSC2 | CKGR_MOR_KEY_PASSWD) & (~PMC_OSC2_EN_WR_CALIB_Msk);

    __DSB();
    __ISB();

    /* Configure the RC Oscillator frequency to select 8MHz */
    PMC_REGS->PMC_OSC2 = (PMC_REGS->PMC_OSC2 & ~PMC_OSC2_OSCRCF_Msk) | PMC_OSC2_KEY_PASSWD | PMC_OSC2_OSCRCF_8_MHZ;

    __DSB();
    __ISB();

    /* Set SEL for 4 MHz */
    PMC_REGS->PMC_OCR2 |= PMC_OCR2_SEL4_Msk;

    /* Set Calib values for 4 MHz */
    PMC_REGS->PMC_OSC2 = (PMC_REGS->PMC_OSC2 | CKGR_MOR_KEY_PASSWD) | PMC_OSC2_EN_WR_CALIB_Msk;
    PMC_REGS->PMC_OCR2 = (PMC_REGS->PMC_OCR2 & ~(PMC_OCR2_CAL4_Msk)) | PMC_OCR2_CAL4(calibSec4);
    PMC_REGS->PMC_OSC2 = (PMC_REGS->PMC_OSC2 | CKGR_MOR_KEY_PASSWD) & (~PMC_OSC2_EN_WR_CALIB_Msk);
}
/*********************************************************************************
Initialize Slow Clock (SLCK)
*********************************************************************************/
static void CLK_SlowClockInitialize(void)
{
    SUPC_REGS->SUPC_CR = SUPC_CR_KEY_PASSWD;
}


/*********************************************************************************
Initialize Main Clock (MAINCK)
*********************************************************************************/
static void CLK_MainClockInitialize(void)
{
    /* Enable Main Crystal Oscillator */
    PMC_REGS->CKGR_MOR = (PMC_REGS->CKGR_MOR & ~CKGR_MOR_MOSCXTST_Msk) | CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCXTST(255) | CKGR_MOR_MOSCXTEN_Msk;

    /* Wait until the main oscillator clock is ready */
    while ( (PMC_REGS->PMC_SR & PMC_SR_MOSCXTS_Msk) != PMC_SR_MOSCXTS_Msk);

    /* Main Crystal Oscillator is selected as the Main Clock (MAINCK) source.
    Switch Main Clock (MAINCK) to Main Crystal Oscillator clock */
    PMC_REGS->CKGR_MOR |= CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCSEL_Msk;

    /* Wait until MAINCK is switched to Main Crystal Oscillator */
    while ( (PMC_REGS->PMC_SR & PMC_SR_MOSCSELS_Msk) != PMC_SR_MOSCSELS_Msk);

    /* Disable the RC Oscillator */
    PMC_REGS->CKGR_MOR = CKGR_MOR_KEY_PASSWD | (PMC_REGS->CKGR_MOR & ~CKGR_MOR_MOSCRCEN_Msk);


    while ((PMC_REGS->PMC_SR & PMC_SR_MCKRDY_Msk) != PMC_SR_MCKRDY_Msk)
    {

    }
}


/*********************************************************************************
Initialize RC2 Clock (RC2CK)
*********************************************************************************/
static void CLK_RC2ClockInitialize(void)
{
    PMC_REGS->PMC_OSC2 |= PMC_OSC2_EN_Msk | PMC_OSC2_KEY_PASSWD;
    if (checkGpnvmWordCrc())
    {
        rc2ckTrimCalibrationSequence();
    }
    PMC_REGS->PMC_OSC2 = (PMC_REGS->PMC_OSC2 & ~PMC_OSC2_OSCRCF_Msk) | PMC_OSC2_OSCRCF_10_MHZ | PMC_OSC2_KEY_PASSWD;
}

/*********************************************************************************
Initialize PLLACK/PLLBCK
*********************************************************************************/
static void CLK_PLLxClockInitialize(void)
{
    PMC_REGS->PMC_PLL_CFG = PMC_PLL_CFG_SRB_SR_VAL_24K |
                            PMC_PLL_CFG_SCB_SC_VAL_20p |
                            PMC_PLL_CFG_OUTCUR_PLLB_ICP0 |
                            PMC_PLL_CFG_SRA_SR_VAL_24K |
                            PMC_PLL_CFG_SCA_SC_VAL_60p |
                            PMC_PLL_CFG_OUTCUR_PLLA_ICP3;

    /* Configure and Enable PLLA */
    PMC_REGS->CKGR_PLLAR =  CKGR_PLLAR_ONE_Msk |
                            CKGR_PLLAR_FREQ_VCO_VCO1 |
                            CKGR_PLLAR_PLLACOUNT(0x3fU) |
                            CKGR_PLLAR_MULA(9U) |
                            CKGR_PLLAR_DIVA(1U);

    while ( (PMC_REGS->PMC_SR & PMC_SR_LOCKA_Msk) != PMC_SR_LOCKA_Msk)
    {

    }

}

/*********************************************************************************
Initialize Master clock (MCK)
*********************************************************************************/
static void CLK_MasterClockInitialize(void)
{
    /* Program PMC_MCKR.PRES and wait for PMC_SR.MCKRDY to be set   */
    PMC_REGS->PMC_MCKR = (PMC_REGS->PMC_MCKR & ~PMC_MCKR_PRES_Msk) | PMC_MCKR_PRES_CLK_1;
    while ((PMC_REGS->PMC_SR & PMC_SR_MCKRDY_Msk) != PMC_SR_MCKRDY_Msk)
    {

    }

    /* Program PMC_MCKR.MDIV and Wait for PMC_SR.MCKRDY to be set   */
    PMC_REGS->PMC_MCKR = (PMC_REGS->PMC_MCKR & ~PMC_MCKR_MDIV_Msk) | PMC_MCKR_MDIV_PCK_DIV2;
    while ((PMC_REGS->PMC_SR & PMC_SR_MCKRDY_Msk) != PMC_SR_MCKRDY_Msk)
    {

    }

    /* Program PMC_MCKR.CSS and Wait for PMC_SR.MCKRDY to be set    */
    PMC_REGS->PMC_MCKR = (PMC_REGS->PMC_MCKR & ~PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_PLLA_CLK;
    while ((PMC_REGS->PMC_SR & PMC_SR_MCKRDY_Msk) != PMC_SR_MCKRDY_Msk)
    {

    }

}

/*********************************************************************************
Initialize Peripheral Clock
*********************************************************************************/
static void CLK_PeripheralClockInitialize(void)
{
    PMC_REGS->PMC_PCR = PMC_PCR_EN_Msk | PMC_PCR_CMD_Msk | PMC_PCR_PID(10U); /* PIO */
    PMC_REGS->PMC_PCR = PMC_PCR_EN_Msk | PMC_PCR_CMD_Msk | PMC_PCR_PID(13U); /* FLEXCOM2 */
}

/*********************************************************************************
Clock Initialize
*********************************************************************************/
void CLOCK_Initialize( void )
{
    SCB_DisableDCache();
    SCB_DisableICache();

    /* Initialize Slow Clock */
    CLK_SlowClockInitialize();

    /* Initialize Main Clock */
    CLK_MainClockInitialize();

    /* Initialize RC2 */
    CLK_RC2ClockInitialize();

    /* Initialize PLLA/PLLB */
    CLK_PLLxClockInitialize();

    /* Initialize Master Clock */
    CLK_MasterClockInitialize();

    /* Initialize Peripheral Clock */
    CLK_PeripheralClockInitialize();


    SCB_EnableDCache();
    SCB_EnableICache();
}
