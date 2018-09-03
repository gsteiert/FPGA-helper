/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        04. August 2014
 * $Revision:    V2.02
 *
 * Driver:       Driver_USART0, Driver_USART1, Driver_USART2, Driver_USART3
 * Configured:   via RTE_Device.h configuration file
 * Project:      USART Driver for NXP LPC18xx
 * -----------------------------------------------------------------------------
 * Use the following configuration settings in the middleware component
 * to connect to this driver.
 *
 *   Configuration Setting                Value     UART Interface
 *   ---------------------                -----     --------------
 *   Connect to hardware via Driver_UART# = 0       use USART0
 *   Connect to hardware via Driver_UART# = 1       use UART1
 *   Connect to hardware via Driver_UART# = 2       use USART2
 *   Connect to hardware via Driver_UART# = 3       use USART3
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.02
 *    - Corrected modem lines handling
 *  Version 2.01
 *    - Added DMA support
 *    - Other Improvements (status checking, USART_Control, ...)
 *  Version 2.00
 *    - Updated to CMSIS Driver API V2.00
 *  Version 1.01
 *    - Based on API V1.10 (namespace prefix ARM_ added)
 *  Version 1.00
 *    - Initial release
 */
#include "USART_LPC18xx.h"

#include "RTE_Device.h"
#include "RTE_Components.h"

#define ARM_USART_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(2,02)

// Driver Version
static const ARM_DRIVER_VERSION usart_driver_version = { ARM_USART_API_VERSION, ARM_USART_DRV_VERSION };

// Trigger level definitions
// Can be user defined by C preprocessor
#ifndef USART0_TRIG_LVL
#define USART0_TRIG_LVL           USART_TRIG_LVL_1
#endif
#ifndef USART1_TRIG_LVL
#define USART1_TRIG_LVL           USART_TRIG_LVL_1
#endif
#ifndef USART2_TRIG_LVL
#define USART2_TRIG_LVL           USART_TRIG_LVL_1
#endif
#ifndef USART3_TRIG_LVL
#define USART3_TRIG_LVL           USART_TRIG_LVL_1
#endif

// USART0
#if (RTE_USART0)
static USART_INFO USART0_Info = {0};
static PIN_ID USART0_pin_tx  = { RTE_USART0_TX_PORT,   RTE_USART0_TX_BIT,   RTE_USART0_TX_FUNC };
static PIN_ID USART0_pin_rx  = { RTE_USART0_RX_PORT,   RTE_USART0_RX_BIT,   RTE_USART0_RX_FUNC };
#if (RTE_USART0_UCLK_PIN_EN == 1)
static PIN_ID USART0_pin_clk = { RTE_USART0_UCLK_PORT, RTE_USART0_UCLK_BIT, RTE_USART0_UCLK_FUNC };
#endif

#if (RTE_USART0_DMA_TX_EN == 1)
void USART0_GPDMA_Tx_Event (uint32_t event);
static USART_DMA USART0_DMA_Tx = {RTE_USART0_DMA_TX_CH,
                                  RTE_USART0_DMA_TX_PERI,
                                  RTE_USART0_DMA_TX_PERI_SEL,
                                  USART0_GPDMA_Tx_Event};
#endif
#if (RTE_USART0_DMA_RX_EN == 1)
void USART0_GPDMA_Rx_Event (uint32_t event);
static USART_DMA USART0_DMA_Rx = {RTE_USART0_DMA_RX_CH,
                                  RTE_USART0_DMA_RX_PERI,
                                  RTE_USART0_DMA_RX_PERI_SEL,
                                  USART0_GPDMA_Rx_Event};
#endif

static const USART_RESOURCES USART0_Resources = {
  {     // Capabilities
    1,  // supports UART (Asynchronous) mode
#if (RTE_USART0_UCLK_PIN_EN == 1)
    1,  // supports Synchronous Master mode
    1,  // supports Synchronous Slave mode
#else
    0,  // supports Synchronous Master mode
    0,  // supports Synchronous Slave mode
#endif
    1,  // supports UART Single-wire mode
    0,  // supports UART IrDA mode
    1,  // supports UART Smart Card mode
#if (RTE_USART0_UCLK_PIN_EN == 1)
    1,  // Smart Card Clock generator
#else
    0,
#endif
    0,  // RTS Flow Control available
    0,  // CTS Flow Control available
    0,  // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
#if (RTE_USART0_DMA_RX_EN == 1)
    0,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#else
    1,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#endif
    0,  // RTS Line: 0=not available, 1=available
    0,  // CTS Line: 0=not available, 1=available
    0,  // DTR Line: 0=not available, 1=available
    0,  // DSR Line: 0=not available, 1=available
    0,  // DCD Line: 0=not available, 1=available
    0,  // RI Line: 0=not available, 1=available
    0,  // Signal CTS change event: \ref ARM_USART_EVENT_CTS
    0,  // Signal DSR change event: \ref ARM_USART_EVENT_DSR
    0,  // Signal DCD change event: \ref ARM_USART_EVENT_DCD
    0,  // Signal RI change event: \ref ARM_USART_EVENT_RI
  },
    LPC_USART0,
    NULL,
  {     // USART Pin Configuration
    &USART0_pin_tx,
    &USART0_pin_rx,
#if (RTE_USART0_UCLK_PIN_EN == 1)
    &USART0_pin_clk,
#else
    NULL,
#endif
    NULL, NULL, NULL, NULL, NULL, NULL,
  },
  {     // USART Clocks Configuration
    &LPC_CCU1->CLK_M3_USART0_CFG,
    &LPC_CCU1->CLK_M3_USART0_STAT,
    &LPC_CCU2->CLK_APB0_USART0_CFG,
    &LPC_CCU2->CLK_APB0_USART0_STAT,
    &LPC_CGU->BASE_UART0_CLK,
  },
  {    // USART Reset Configuration
    (1 << 12),
    &LPC_RGU->RESET_CTRL1,
    &LPC_RGU->RESET_ACTIVE_STATUS1,
  },
    USART0_IRQn,
    USART0_TRIG_LVL,
#if (RTE_USART0_DMA_TX_EN == 1)
    &USART0_DMA_Tx,
#else
    NULL,
#endif
#if (RTE_USART0_DMA_RX_EN == 1)
    &USART0_DMA_Rx,
#else
    NULL,
#endif
    &USART0_Info
};
#endif

// UART1
#if (RTE_UART1)
static USART_INFO USART1_Info = {0};
static PIN_ID USART1_pin_tx  = { RTE_UART1_TX_PORT,   RTE_UART1_TX_BIT,  RTE_UART1_TX_FUNC };
static PIN_ID USART1_pin_rx  = { RTE_UART1_RX_PORT,   RTE_UART1_RX_BIT,  RTE_UART1_RX_FUNC };
#if (RTE_UART1_CTS_PIN_EN == 1)
static PIN_ID USART1_pin_cts = { RTE_UART1_CTS_PORT,  RTE_UART1_CTS_BIT, RTE_UART1_CTS_FUNC };
#endif
#if (RTE_UART1_RTS_PIN_EN == 1)
static PIN_ID USART1_pin_rts = { RTE_UART1_RTS_PORT,  RTE_UART1_RTS_BIT, RTE_UART1_RTS_FUNC };
#endif
#if (RTE_UART1_DCD_PIN_EN == 1)
static PIN_ID USART1_pin_dcd = { RTE_UART1_DCD_PORT,  RTE_UART1_DCD_BIT, RTE_UART1_DCD_FUNC };
#endif
#if (RTE_UART1_DSR_PIN_EN == 1)
static PIN_ID USART1_pin_dsr = { RTE_UART1_DSR_PORT,  RTE_UART1_DSR_BIT, RTE_UART1_DSR_FUNC };
#endif
#if (RTE_UART1_DTR_PIN_EN == 1)
static PIN_ID USART1_pin_dtr = { RTE_UART1_DTR_PORT,  RTE_UART1_DTR_BIT, RTE_UART1_DTR_FUNC };
#endif
#if (RTE_UART1_RI_PIN_EN == 1)
static PIN_ID USART1_pin_ri  = { RTE_UART1_RI_PORT,   RTE_UART1_RI_BIT,  RTE_UART1_RI_FUNC };
#endif

#if (RTE_UART1_DMA_TX_EN == 1)
void USART1_GPDMA_Tx_Event (uint32_t event);
static USART_DMA USART1_DMA_Tx = {RTE_UART1_DMA_TX_CH,
                                  RTE_UART1_DMA_TX_PERI,
                                  RTE_UART1_DMA_TX_PERI_SEL,
                                  USART1_GPDMA_Tx_Event};
#endif
#if (RTE_UART1_DMA_RX_EN == 1)
void USART1_GPDMA_Rx_Event (uint32_t event);
static USART_DMA USART1_DMA_Rx = {RTE_UART1_DMA_RX_CH,
                                  RTE_UART1_DMA_RX_PERI,
                                  RTE_UART1_DMA_RX_PERI_SEL,
                                  USART1_GPDMA_Rx_Event};
#endif

static const USART_RESOURCES USART1_Resources = {
  {     // Capabilities
    1,  // supports UART (Asynchronous) mode 
    0,  // supports Synchronous Master mode
    0,  // supports Synchronous Slave mode
    0,  // supports UART Single-wire mode
    0,  // supports UART IrDA mode
    0,  // supports UART Smart Card mode
    0,  // Smart Card Clock generator
#if (RTE_UART1_RTS_PIN_EN == 1)
    1,  // RTS Flow Control available
#else
    0,  // RTS Flow Control available
#endif
#if (RTE_UART1_CTS_PIN_EN == 1)
    1,  // CTS Flow Control available
#else
    0,  // CTS Flow Control available
#endif
    0,  // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
#if (RTE_UART1_DMA_RX_EN == 1)
    0,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#else
    1,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#endif
#if (RTE_UART1_RTS_PIN_EN == 1)
    1,  // RTS Line: 0=not available, 1=available
#else
    0,
#endif
#if (RTE_UART1_CTS_PIN_EN == 1)
    1,  // CTS Line: 0=not available, 1=available
#else
    0,
#endif
#if (RTE_UART1_DTR_PIN_EN == 1)
    1,  // DTR Line: 0=not available, 1=available
#else
    0,
#endif
#if (RTE_UART1_DSR_PIN_EN == 1)
    1,  // DSR Line: 0=not available, 1=available
#else
    0,
#endif
#if (RTE_UART1_DCD_PIN_EN == 1)
    1,  // DCD Line: 0=not available, 1=available
#else
    0,
#endif
#if (RTE_UART1_RI_PIN_EN == 1)
    1,  // RI Line: 0=not available, 1=available
#else
    0,
#endif
#if (RTE_UART1_CTS_PIN_EN == 1)
    1,  // Signal CTS change event: \ref ARM_USART_EVENT_CTS
#else
    0,
#endif
#if (RTE_UART1_DSR_PIN_EN == 1)
    1,  // Signal DSR change event: \ref ARM_USART_EVENT_DSR
#else
    0,
#endif
#if (RTE_UART1_DCD_PIN_EN == 1)
    1,  // Signal DCD change event: \ref ARM_USART_EVENT_DCD
#else
    0,
#endif
#if (RTE_UART1_RI_PIN_EN == 1)
    1,  // Signal RI change event: \ref ARM_USART_EVENT_RI
#else
    0,
#endif
  },
    (LPC_USARTn_Type *)LPC_UART1,
    LPC_UART1,
  {     // USART Pin Configuration
    &USART1_pin_tx,
    &USART1_pin_rx,
    NULL,
#if (RTE_UART1_CTS_PIN_EN == 1)
    &USART1_pin_cts,
#else
    NULL,
#endif
#if (RTE_UART1_RTS_PIN_EN == 1)
    &USART1_pin_rts,
#else
    NULL,
#endif
#if (RTE_UART1_DCD_PIN_EN == 1)
    &USART1_pin_dcd,
#else
    NULL,
#endif
#if (RTE_UART1_DSR_PIN_EN == 1)
    &USART1_pin_dsr,
#else
    NULL,
#endif
#if (RTE_UART1_DTR_PIN_EN == 1)
    &USART1_pin_dtr,
#else
    NULL,
#endif
#if (RTE_UART1_RI_PIN_EN == 1)
    &USART1_pin_ri,
#else
    NULL,
#endif
  },
  {     // USART Clocks Configuration
    &LPC_CCU1->CLK_M3_UART1_CFG,
    &LPC_CCU1->CLK_M3_UART1_STAT,
    &LPC_CCU2->CLK_APB0_UART1_CFG,
    &LPC_CCU2->CLK_APB0_UART1_STAT,
    &LPC_CGU->BASE_UART1_CLK,
  },
  {    // USART Reset Configuration
    (1 << 13),
    &LPC_RGU->RESET_CTRL1,
    &LPC_RGU->RESET_ACTIVE_STATUS1,
  },
    UART1_IRQn,
    USART1_TRIG_LVL,
#if (RTE_UART1_DMA_TX_EN == 1)
    &USART1_DMA_Tx,
#else
    NULL,
#endif
#if (RTE_UART1_DMA_RX_EN == 1)
    &USART1_DMA_Rx,
#else
    NULL,
#endif
    &USART1_Info
};
#endif

// USART2
#if (RTE_USART2)
static USART_INFO USART2_Info = {0};
static PIN_ID USART2_pin_tx  = { RTE_USART2_TX_PORT,   RTE_USART2_TX_BIT,   RTE_USART2_TX_FUNC };
static PIN_ID USART2_pin_rx  = { RTE_USART2_RX_PORT,   RTE_USART2_RX_BIT,   RTE_USART2_RX_FUNC };
#if (RTE_USART2_UCLK_PIN_EN == 1)
static PIN_ID USART2_pin_clk = { RTE_USART2_UCLK_PORT, RTE_USART2_UCLK_BIT, RTE_USART2_UCLK_FUNC };
#endif

#if (RTE_USART2_DMA_TX_EN == 1)
void USART2_GPDMA_Tx_Event (uint32_t event);
static USART_DMA USART2_DMA_Tx = {RTE_USART2_DMA_TX_CH,
                                  RTE_USART2_DMA_TX_PERI,
                                  RTE_USART2_DMA_TX_PERI_SEL,
                                  USART2_GPDMA_Tx_Event};
#endif
#if (RTE_USART2_DMA_RX_EN == 1)
void USART2_GPDMA_Rx_Event (uint32_t event);
static USART_DMA USART2_DMA_Rx = {RTE_USART2_DMA_RX_CH,
                                  RTE_USART2_DMA_RX_PERI,
                                  RTE_USART2_DMA_RX_PERI_SEL,
                                  USART2_GPDMA_Rx_Event};
#endif

static const USART_RESOURCES USART2_Resources = {
  {     // Capabilities
    1,  // supports UART (Asynchronous) mode
#if (RTE_USART1_UCLK_PIN_EN == 1)
    1,  // supports Synchronous Master mode
    1,  // supports Synchronous Slave mode
#else
    0,  // supports Synchronous Master mode
    0,  // supports Synchronous Slave mode
#endif
    1,  // supports UART Single-wire mode
    0,  // supports UART IrDA mode
    1,  // supports UART Smart Card mode
#if (RTE_USART2_UCLK_PIN_EN == 1)
    1,  // Smart Card Clock generator
#else
    0,
#endif
    0,  // RTS Flow Control available
    0,  // CTS Flow Control available
    0,  // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
#if (RTE_USART2_DMA_RX_EN == 1)
    0,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#else
    1,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#endif
    0,  // RTS Line: 0=not available, 1=available
    0,  // CTS Line: 0=not available, 1=available
    0,  // DTR Line: 0=not available, 1=available
    0,  // DSR Line: 0=not available, 1=available
    0,  // DCD Line: 0=not available, 1=available
    0,  // RI Line: 0=not available, 1=available
    0,  // Signal CTS change event: \ref ARM_USART_EVENT_CTS
    0,  // Signal DSR change event: \ref ARM_USART_EVENT_DSR
    0,  // Signal DCD change event: \ref ARM_USART_EVENT_DCD
    0,  // Signal RI change event: \ref ARM_USART_EVENT_RI
  },
    LPC_USART2,
    NULL,
  {     // USART Pin Configuration
    &USART2_pin_tx,
    &USART2_pin_rx,
#if (RTE_USART2_UCLK_PIN_EN == 1)
    &USART2_pin_clk,
#else
    NULL,
#endif
    NULL, NULL, NULL, NULL, NULL, NULL,
  },
  {     // USART Clocks Configuration
    &LPC_CCU1->CLK_M3_USART2_CFG,
    &LPC_CCU1->CLK_M3_USART2_STAT,
    &LPC_CCU2->CLK_APB2_USART2_CFG,
    &LPC_CCU2->CLK_APB2_USART2_STAT,
    &LPC_CGU->BASE_UART2_CLK,
  },
  {    // USART Reset Configuration
    (1 << 14),
    &LPC_RGU->RESET_CTRL1,
    &LPC_RGU->RESET_ACTIVE_STATUS1,
  },
    USART2_IRQn,
    USART2_TRIG_LVL,
#if (RTE_USART2_DMA_TX_EN == 1)
    &USART2_DMA_Tx,
#else
    NULL,
#endif
#if (RTE_USART2_DMA_RX_EN == 1)
    &USART2_DMA_Rx,
#else
    NULL,
#endif
    &USART2_Info
};
#endif

// USART3
#if (RTE_USART3)
static USART_INFO USART3_Info = {0};
static PIN_ID USART3_pin_tx  = { RTE_USART3_TX_PORT,   RTE_USART3_TX_BIT,   RTE_USART3_TX_FUNC };
static PIN_ID USART3_pin_rx  = { RTE_USART3_RX_PORT,   RTE_USART3_RX_BIT,   RTE_USART3_RX_FUNC };
#if (RTE_USART3_UCLK_PIN_EN == 1)
static PIN_ID USART3_pin_clk = { RTE_USART3_UCLK_PORT, RTE_USART3_UCLK_BIT, RTE_USART3_UCLK_FUNC };
#endif

#if (RTE_USART3_DMA_TX_EN == 1)
void USART3_GPDMA_Tx_Event (uint32_t event);
static USART_DMA USART3_DMA_Tx = {RTE_USART3_DMA_TX_CH,
                                  RTE_USART3_DMA_TX_PERI,
                                  RTE_USART3_DMA_TX_PERI_SEL,
                                  USART3_GPDMA_Tx_Event};
#endif
#if (RTE_USART3_DMA_RX_EN == 1)
void USART3_GPDMA_Rx_Event (uint32_t event);
static USART_DMA USART3_DMA_Rx = {RTE_USART3_DMA_RX_CH,
                                  RTE_USART3_DMA_RX_PERI,
                                  RTE_USART3_DMA_RX_PERI_SEL,
                                  USART3_GPDMA_Rx_Event};
#endif

static const USART_RESOURCES USART3_Resources = {
  {     // Capabilities
    1,  // supports UART (Asynchronous) mode 
#if (RTE_USART3_UCLK_PIN_EN == 1)
    1,  // supports Synchronous Master mode
    1,  // supports Synchronous Slave mode
#else
    0,  // supports Synchronous Master mode
    0,  // supports Synchronous Slave mode
#endif
    1,  // supports UART Single-wire mode
    1,  // supports UART IrDA mode
    1,  // supports UART Smart Card mode
#if (RTE_USART3_UCLK_PIN_EN == 1)
    1,  // Smart Card Clock generator
#else
    0,
#endif
    0,  // RTS Flow Control available
    0,  // CTS Flow Control available
    0,  // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
#if (RTE_USART3_DMA_RX_EN == 1)
    0,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#else
    1,  // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#endif
    0,  // RTS Line: 0=not available, 1=available
    0,  // CTS Line: 0=not available, 1=available
    0,  // DTR Line: 0=not available, 1=available
    0,  // DSR Line: 0=not available, 1=available
    0,  // DCD Line: 0=not available, 1=available
    0,  // RI Line: 0=not available, 1=available
    0,  // Signal CTS change event: \ref ARM_USART_EVENT_CTS
    0,  // Signal DSR change event: \ref ARM_USART_EVENT_DSR
    0,  // Signal DCD change event: \ref ARM_USART_EVENT_DCD
    0,  // Signal RI change event: \ref ARM_USART_EVENT_RI
  },
    LPC_USART3,
    NULL,
  {     // USART Pin Configuration
    &USART3_pin_tx,
    &USART3_pin_rx,
#if (RTE_USART3_UCLK_PIN_EN == 1)
    &USART3_pin_clk,
#else
    NULL,
#endif
    NULL, NULL, NULL, NULL, NULL, NULL,
  },
  {     // USART Clocks Configuration
    &LPC_CCU1->CLK_M3_USART3_CFG,
    &LPC_CCU1->CLK_M3_USART3_STAT,
    &LPC_CCU2->CLK_APB2_USART3_CFG,
    &LPC_CCU2->CLK_APB2_USART3_STAT,
    &LPC_CGU->BASE_UART3_CLK,
  },
  {    // USART Reset Configuration
    (1 << 15),
    &LPC_RGU->RESET_CTRL1,
    &LPC_RGU->RESET_ACTIVE_STATUS1,
  },
    USART3_IRQn,
    USART3_TRIG_LVL,
#if (RTE_USART3_DMA_TX_EN == 1)
    &USART3_DMA_Tx,
#else
    NULL,
#endif
#if (RTE_USART3_DMA_RX_EN == 1)
    &USART3_DMA_Rx,
#else
    NULL,
#endif
    &USART3_Info
};
#endif


// Extern Function
extern uint32_t GetClockFreq (uint32_t clk_src);

// Local Function
/**
  \fn          int32_t USART_SetBaudrate (uint32_t         baudrate,
                                          USART_RESOURCES *usart)
  \brief       Set baudrate dividers
  \param[in]   baudrate  Usart baudrate
  \param[in]   usart     Pointer to USART resources)
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t USART_SetBaudrate (uint32_t         baudrate,
                           USART_RESOURCES *usart) {
  uint32_t d, m, bestd, bestm, tmp, PClk;
  uint64_t best_divisor, divisor;
  uint32_t current_error, best_error;
  uint32_t recalcbaud;

  PClk = GetClockFreq ((*usart->clk.base_clk >> 24) & 0x1F);

  // Worst case
  best_error = 0xFFFFFFFF;
  bestd = 0;
  bestm = 0;
  best_divisor = 0;
  for (m = 1; m <= 15; m++) {
    for (d = 0; d < m; d++) {
      divisor = ((uint64_t)PClk << 28) * m / (baudrate * (m + d));
      current_error = divisor & 0xFFFFFFFF;

      tmp = divisor >> 32;

      // Adjust error
      if (current_error > (1UL << 31)) {
        current_error = -current_error;
        tmp++;
      }

      // If out of range
      if ((tmp < 1) || (tmp > 65536)) continue;

      if (current_error < best_error) {
        best_error   = current_error;
        best_divisor = tmp;
        bestd = d;
        bestm = m;
        if (best_error == 0) break;
      }
    }
    if (best_error == 0) break;
  }

  // Can not find best match
  if (best_divisor == 0) return -1;

  recalcbaud = (PClk >> 4) * bestm / (best_divisor * (bestm + bestd));

  // Reuse best_error to evaluate baud error
  if (baudrate > recalcbaud) best_error = baudrate - recalcbaud;
  else best_error = recalcbaud - baudrate;

  best_error = best_error * 100 / baudrate;

  if (best_error < UART_ACCEPTED_BAUDRATE_ERROR) {
    usart->reg->LCR |= USART_LCR_DLAB;
    usart->reg->DLM  = ((best_divisor >> 8) & 0xFF) << USART_DLM_DLMSB_POS;
    usart->reg->DLL  = (best_divisor & USART_DLL_DLLSB_MSK) << USART_DLL_DLLSB_POS;
    // Reset DLAB bit
    usart->reg->LCR &= (~USART_LCR_DLAB);
    usart->reg->FDR  = ((bestm << USART_FDR_MULVAL_POS) & USART_FDR_MULVAL_MSK)  |
                        (bestd                          & USART_FDR_DIVADDVAL_MSK);
  } else return -1;

  // Save configured baudrate value
  usart->info->baudrate = recalcbaud;

  return 0;
}

/**
  \fn          uint32_t USART_RxLineIntHandler (USART_RESOURCES *usart)
  \brief       Receive line interrupt handler
  \param[in]   usart     Pointer to USART resources
  \return      Rx Line event mask
*/
static uint32_t USART_RxLineIntHandler (USART_RESOURCES *usart) {
  uint32_t lsr, event;

  event = 0;
  lsr   = usart->reg->LSR  & USART_LSR_LINE_INT;

  // OverRun error
  if (lsr & USART_LSR_OE) {
    usart->info->status.rx_overflow = 1;
    event |= ARM_USART_EVENT_RX_OVERFLOW;

    // Sync Slave mode: If Transmitter enabled, signal TX underflow
    if (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE) {
      if (usart->info->flags & USART_FLAG_SEND_ACTIVE) {
        event |= ARM_USART_EVENT_TX_UNDERFLOW;
      }
    }
  }

  // Parity error
  if (lsr & USART_LSR_PE) {
    usart->info->status.rx_parity_error = 1;
    event |= ARM_USART_EVENT_RX_PARITY_ERROR;
  }

  // Break detected
  if (lsr & USART_LSR_BI) {
    usart->info->status.rx_break = 1;
    event |= ARM_USART_EVENT_RX_BREAK;
  }

  // Framing error
  else if(lsr & USART_LSR_FE) {
    usart->info->status.rx_framing_error = 1;
    event |= ARM_USART_EVENT_RX_FRAMING_ERROR;
  }

  return event;
}

// Function Prototypes
static int32_t USART_Receive (void            *data,
                              uint32_t         num,
                              USART_RESOURCES *usart);


// USART Driver functions

/**
  \fn          ARM_DRIVER_VERSION USARTx_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION USARTx_GetVersion (void) {
  return usart_driver_version;
}

/**
  \fn          ARM_USART_CAPABILITIES USART_GetCapabilities (USART_RESOURCES *usart)
  \brief       Get driver capabilities
  \param[in]   usart     Pointer to USART resources
  \return      \ref ARM_USART_CAPABILITIES
*/
static ARM_USART_CAPABILITIES USART_GetCapabilities (USART_RESOURCES *usart) {
  return usart->capabilities;
}

/**
  \fn          int32_t USART_Initialize (ARM_USART_SignalEvent_t  cb_event
                                         USART_RESOURCES         *usart)
  \brief       Initialize USART Interface.
  \param[in]   cb_event  Pointer to \ref ARM_USART_SignalEvent
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Initialize (ARM_USART_SignalEvent_t  cb_event,
                                 USART_RESOURCES         *usart) {

  if (usart->info->flags & USART_FLAG_POWERED) {
    // Device is powered - could not be re-initialized
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->flags & USART_FLAG_INITIALIZED) {
    // Driver is already initialized
    return ARM_DRIVER_OK;
  }

  // Initialize USART Run-time Resources
  usart->info->cb_event = cb_event;

  usart->info->status.tx_busy          = 0;
  usart->info->status.rx_busy          = 0;
  usart->info->status.tx_underflow     = 0;
  usart->info->status.rx_overflow      = 0;
  usart->info->status.rx_break         = 0;
  usart->info->status.rx_framing_error = 0;
  usart->info->status.rx_parity_error  = 0;

  usart->info->mode = 0;
  usart->info->xfer.tx_def_val = 0;

  // Configure CTS pin
  if (usart->capabilities.cts) {
    SCU_PinConfigure(usart->pins.cts->port, usart->pins.cts->num,
                     SCU_PIN_CFG_INPUT_FILTER_DIS | SCU_PIN_CFG_INPUT_BUFFER_EN |
                     SCU_PIN_CFG_PULLUP_DIS | SCU_PIN_CFG_MODE(usart->pins.cts->config_val));
  }

  // Configure RTS pin
  if (usart->capabilities.rts) {
    SCU_PinConfigure(usart->pins.rts->port, usart->pins.rts->num, SCU_PIN_CFG_PULLUP_DIS |
                     SCU_PIN_CFG_MODE(usart->pins.rts->config_val));
  }

  // Configure DCD pin
  if (usart->capabilities.dcd) {
    SCU_PinConfigure(usart->pins.dcd->port, usart->pins.dcd->num,
                     SCU_PIN_CFG_INPUT_FILTER_DIS | SCU_PIN_CFG_INPUT_BUFFER_EN |
                     SCU_PIN_CFG_PULLUP_DIS | SCU_PIN_CFG_MODE(usart->pins.dcd->config_val));
  }

  // Configure DSR pin
  if (usart->capabilities.dsr) {
    SCU_PinConfigure(usart->pins.dsr->port, usart->pins.dsr->num,
                     SCU_PIN_CFG_INPUT_FILTER_DIS | SCU_PIN_CFG_INPUT_BUFFER_EN |
                     SCU_PIN_CFG_PULLUP_DIS | SCU_PIN_CFG_MODE(usart->pins.dsr->config_val));
  }

  // Configure DTR pin
  if (usart->capabilities.dtr) {
    SCU_PinConfigure(usart->pins.dtr->port, usart->pins.dtr->num, SCU_PIN_CFG_PULLUP_DIS |
                     SCU_PIN_CFG_MODE(usart->pins.dtr->config_val));
  }

  // Configure RI pin
  if (usart->capabilities.ri) {
    SCU_PinConfigure(usart->pins.ri->port, usart->pins.ri->num,
                     SCU_PIN_CFG_INPUT_FILTER_DIS | SCU_PIN_CFG_INPUT_BUFFER_EN |
                     SCU_PIN_CFG_PULLUP_DIS | SCU_PIN_CFG_MODE(usart->pins.ri->config_val));
  }

  usart->info->flags = USART_FLAG_INITIALIZED;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Uninitialize (USART_RESOURCES *usart)
  \brief       De-initialize USART Interface.
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Uninitialize (USART_RESOURCES *usart) {

  if (usart->info->flags & USART_FLAG_POWERED) {
    // Driver is powered - could not be uninitialized
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->flags == 0) {
    // Driver not initialized
    return ARM_DRIVER_OK;
  }

  // Reset TX pin configuration
  SCU_PinConfigure(usart->pins.tx->port, usart->pins.tx->num  , 0);

  // Reset RX pin configuration
  SCU_PinConfigure(usart->pins.rx->port, usart->pins.rx->num  , 0);

  // Reset CLK pin configuration
  if (usart->pins.clk)
    SCU_PinConfigure(usart->pins.clk->port, usart->pins.clk->num, 0);

  // Reset CTS pin configuration
  if (usart->capabilities.cts)
    SCU_PinConfigure(usart->pins.cts->port, usart->pins.cts->num, 0);

  // Reset RTS pin configuration
  if (usart->capabilities.rts)
    SCU_PinConfigure(usart->pins.rts->port, usart->pins.rts->num, 0);

  // Configure DCD pin configuration
  if (usart->capabilities.dcd)
    SCU_PinConfigure(usart->pins.dcd->port, usart->pins.dcd->num, 0);

  // Reset DSR pin configuration
  if (usart->capabilities.dsr)
    SCU_PinConfigure(usart->pins.dsr->port, usart->pins.dsr->num, 0);

  // Reset DTR pin configuration
  if (usart->capabilities.dtr)
    SCU_PinConfigure(usart->pins.dtr->port, usart->pins.dtr->num, 0);

  // Reset RI pin configuration
  if (usart->capabilities.ri)
    SCU_PinConfigure(usart->pins.ri->port, usart->pins.ri->num, 0);

  // Reset USART status flags
  usart->info->flags = 0;

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_PowerControl (ARM_POWER_STATE state)
  \brief       Control USART Interface Power.
  \param[in]   state  Power state
  \param[in]   usart  Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_PowerControl (ARM_POWER_STATE  state,
                                   USART_RESOURCES *usart) {
  uint32_t val;

  if ((usart->info->flags & USART_FLAG_INITIALIZED) == 0) {
    // Return error, if USART is not initialized
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->status.rx_busy == 1) {
    // Receive busy
    return ARM_DRIVER_ERROR_BUSY;
  }

  if (usart->info->flags & USART_FLAG_SEND_ACTIVE) {
    // Transmit busy
    return ARM_DRIVER_ERROR_BUSY;
  }
  if (usart->info->flags & USART_FLAG_POWERED) {
    if ((usart->reg->LSR & USART_LSR_TEMT) == 0)
      // Transmit busy
    return ARM_DRIVER_ERROR_BUSY;
  }

  switch (state) {
    case ARM_POWER_OFF:
      if ((usart->info->flags & USART_FLAG_POWERED) == 0)
        return ARM_DRIVER_OK;

      // Disable USART IRQ
      NVIC_DisableIRQ(usart->irq_num);

      // Disable USART peripheral clock
      *usart->clk.peri_cfg &= ~1;
      while (*usart->clk.peri_cfg & 1);
        
      // Disable USART register interface clock
      *usart->clk.reg_cfg &= ~1;
      while (*usart->clk.reg_cfg & 1);

      usart->info->flags = USART_FLAG_INITIALIZED;
      break;

    case ARM_POWER_LOW:
      return ARM_DRIVER_ERROR_UNSUPPORTED;

    case ARM_POWER_FULL:
      if (usart->info->flags & USART_FLAG_POWERED)
        return ARM_DRIVER_OK;

      // Enable USART register interface clock
      *usart->clk.reg_cfg |= 1;
      while ((*usart->clk.reg_cfg & 1) == 0);

      // Enable USART peripheral clock
      *usart->clk.peri_cfg |= 1;
      while (( *usart->clk.peri_cfg & 1) == 0);

      // Reset USART peripheral
      *usart->rst.reg_cfg = usart->rst.reg_cfg_val;
      while ((*(usart->rst.reg_stat) & usart->rst.reg_cfg_val) == 0);

      // Disable transmitter
      usart->reg->TER &= ~USART_TER_TXEN;

      // Disable receiver
      usart->reg->RS485CTRL |= USART_RS485CTRL_RXDIS;

      // Disable interrupts
      usart->reg->IER = 0;

      // Configure FIFO Control register
      // Set trigger level      
      val  = usart->trig_lvl & USART_FCR_RXTRIGLVL_MSK;
      val |= USART_FCR_FIFOEN;

      if (usart->dma_rx || usart->dma_tx)
        val |= USART_FCR_DMAMODE;

      usart->reg->FCR = val;

#if (RTE_UART1)
      // Enable modem lines status interrupts (only UART1)
      if (usart->uart_reg) {
        if (usart->capabilities.cts || usart->capabilities.dcd ||
            usart->capabilities.dsr || usart->capabilities.ri) {
          usart->uart_reg->IER |= UART_IER_MSIE;
        }
      }
#endif

      usart->info->flags = USART_FLAG_POWERED | USART_FLAG_INITIALIZED;

      // Clear and Enable USART IRQ
      NVIC_ClearPendingIRQ(usart->irq_num);
      NVIC_EnableIRQ(usart->irq_num);

      break;

    default: return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Send (const void            *data,
                                         uint32_t         num,
                                         USART_RESOURCES *usart)
  \brief       Start sending data to USART transmitter.
  \param[in]   data  Pointer to buffer with data to send to USART transmitter
  \param[in]   num   Number of data items to send
  \param[in]   usart Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Send (const void            *data,
                                 uint32_t         num,
                                 USART_RESOURCES *usart) {
  int32_t stat, source_inc, val;

  if ((data == NULL) || (num == 0)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured (mode not selected)
    return ARM_DRIVER_ERROR;
  }

  if (usart->info->flags & USART_FLAG_SEND_ACTIVE) {
    // Send is not completed yet
    return ARM_DRIVER_ERROR_BUSY;
  }

  // Set Send active flag
  usart->info->flags |= USART_FLAG_SEND_ACTIVE;

  // For DMA mode: source increment
  source_inc = GPDMA_CH_CONTROL_SI;

  // Synchronous mode
  if ((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
      (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {
    if (usart->info->xfer.sync_mode == 0) {
      usart->info->xfer.sync_mode = USART_SYNC_MODE_TX;
      // Start dummy reads
      stat = USART_Receive (&usart->info->xfer.rx_dump_val, num, usart);
      if (stat == ARM_DRIVER_ERROR_BUSY) return ARM_DRIVER_ERROR_BUSY;

    } else if (usart->info->xfer.sync_mode == USART_SYNC_MODE_RX) {
      // Dummy DMA writes (do not increment source address)
      source_inc = 0;
    }
  }

  // Save transmit buffer info
  usart->info->xfer.tx_buf = (uint8_t *)data;
  usart->info->xfer.tx_num = num;
  usart->info->xfer.tx_cnt = 0;

  // DMA mode
  if (usart->dma_tx) {

    // Configure DMA mux
    GPDMA_PeripheralSelect (usart->dma_tx->peripheral, usart->dma_tx->peripheral_sel);

    // Configure DMA channel
    stat = GPDMA_ChannelConfigure (usart->dma_tx->channel,
                                   (uint32_t)data,
                                   (uint32_t)(&(usart->reg->THR)),
                                   GPDMA_CH_CONTROL_TRANSFERSIZE(num)                       |
                                   GPDMA_CH_CONTROL_SBSIZE(GPDMA_BSIZE_1)                   |
                                   GPDMA_CH_CONTROL_DBSIZE(GPDMA_BSIZE_1)                   |
                                   GPDMA_CH_CONTROL_SWIDTH(GPDMA_WIDTH_BYTE)                |
                                   GPDMA_CH_CONTROL_DWIDTH(GPDMA_WIDTH_BYTE)                |
                                   GPDMA_CH_CONTROL_S                                       |
                                   GPDMA_CH_CONTROL_D                                       |
                                   GPDMA_CH_CONTROL_I                                       |
                                   source_inc,
                                   GPDMA_CH_CONFIG_DEST_PERI(usart->dma_tx->peripheral) |
                                   GPDMA_CH_CONFIG_FLOWCNTRL(GPDMA_TRANSFER_M2P_CTRL_DMA)   |
                                   GPDMA_CH_CONFIG_IE                                       |
                                   GPDMA_CH_CONFIG_ITC                                      |
                                   GPDMA_CH_CONFIG_E,
                                   usart->dma_tx->cb_event);
  if (stat == -1) return ARM_DRIVER_ERROR_BUSY;

  // Interrupt mode
  } else {
    // Fill TX FIFO
    if      (usart->reg->LSR & USART_LSR_TEMT) {
      val = 16;
      while ((val--) && (usart->info->xfer.tx_cnt != usart->info->xfer.tx_num)) {
        usart->reg->THR = usart->info->xfer.tx_buf[usart->info->xfer.tx_cnt++];
      }
    }
    // Enable transmit holding register empty interrupt
    usart->reg->IER |= USART_IER_THREIE;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Receive (void            *data,
                                      uint32_t         num,
                                      USART_RESOURCES *usart)
  \brief       Start receiving data from USART receiver.
  \param[out]  data  Pointer to buffer for data to receive from USART receiver
  \param[in]   num   Number of data items to receive
  \param[in]   usart Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Receive (void            *data,
                              uint32_t         num,
                              USART_RESOURCES *usart) {

  int32_t stat, dest_inc;

  if ((data == NULL) || (num == 0)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured (mode not selected)
    return ARM_DRIVER_ERROR;
  }

  // Check if receiver is busy
  if (usart->info->status.rx_busy == 1) 
    return ARM_DRIVER_ERROR_BUSY;

  // Set RX busy flag
  usart->info->status.rx_busy = 1;

  dest_inc = GPDMA_CH_CONTROL_DI;

  // Save number of data to be received
  usart->info->xfer.rx_num = num;

  // Clear RX statuses
  usart->info->status.rx_break          = 0;
  usart->info->status.rx_framing_error  = 0;
  usart->info->status.rx_overflow       = 0;
  usart->info->status.rx_parity_error   = 0;

  // Save receive buffer info
  usart->info->xfer.rx_buf = (uint8_t *)data;
  usart->info->xfer.rx_cnt =            0;

  // Synchronous mode
  if ((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
      (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {
    if (usart->info->xfer.sync_mode == USART_SYNC_MODE_TX) {
      // Dummy DMA reads (do not increment destination address)
      dest_inc = 0;
    }
  }

  // DMA mode
  if (usart->dma_rx) {

    GPDMA_PeripheralSelect (usart->dma_rx->peripheral, usart->dma_rx->peripheral_sel);
    stat = GPDMA_ChannelConfigure (usart->dma_rx->channel,
                                   (uint32_t)&usart->reg->RBR,
                                   (uint32_t)data,
                                   GPDMA_CH_CONTROL_TRANSFERSIZE(num)                       |
                                   GPDMA_CH_CONTROL_SBSIZE(GPDMA_BSIZE_1)                   |
                                   GPDMA_CH_CONTROL_DBSIZE(GPDMA_BSIZE_1)                   |
                                   GPDMA_CH_CONTROL_SWIDTH(GPDMA_WIDTH_BYTE)                |
                                   GPDMA_CH_CONTROL_DWIDTH(GPDMA_WIDTH_BYTE)                |
                                   GPDMA_CH_CONTROL_S                                       |
                                   GPDMA_CH_CONTROL_D                                       |
                                   GPDMA_CH_CONTROL_I                                       |
                                   dest_inc,
                                   GPDMA_CH_CONFIG_SRC_PERI(usart->dma_rx->peripheral)      |
                                   GPDMA_CH_CONFIG_FLOWCNTRL(GPDMA_TRANSFER_P2M_CTRL_DMA)   |
                                   GPDMA_CH_CONFIG_IE                                       |
                                   GPDMA_CH_CONFIG_ITC                                      |
                                   GPDMA_CH_CONFIG_E,
                                   usart->dma_rx->cb_event);
  if (stat == -1) return ARM_DRIVER_ERROR_BUSY;

  // Interrupt mode
  } else {
    // Enable receive data available interrupt
    usart->reg->IER |= USART_IER_RBRIE;
  }

  // Synchronous mode
  if ((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
      (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {
    if (usart->info->xfer.sync_mode == 0) {
      usart->info->xfer.sync_mode = USART_SYNC_MODE_RX;
      // Send dummy data
      stat = USART_Send (&usart->info->xfer.tx_def_val, num, usart);
      if (stat == ARM_DRIVER_ERROR_BUSY) return ARM_DRIVER_ERROR_BUSY;
    }
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t USART_Transfer (const void             *data_out,
                                             void             *data_in,
                                             uint32_t          num,
                                             USART_RESOURCES  *usart)
  \brief       Start sending/receiving data to/from USART transmitter/receiver.
  \param[in]   data_out  Pointer to buffer with data to send to USART transmitter
  \param[out]  data_in   Pointer to buffer for data to receive from USART receiver
  \param[in]   num       Number of data items to transfer
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_Transfer (const void             *data_out,
                                     void             *data_in,
                                     uint32_t          num,
                                     USART_RESOURCES  *usart) {
  int32_t status;

  if ((data_out == NULL) || (data_in == NULL) || (num == 0)) {
    // Invalid parameters
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured
    return ARM_DRIVER_ERROR;
  }

  if ((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
      (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {

    // Set xfer mode
    usart->info->xfer.sync_mode = USART_SYNC_MODE_TX_RX;

    // Receive
    status = USART_Receive (data_in, num, usart);
    if (status != ARM_DRIVER_OK) return status;

    // Send
    status = USART_Send (data_out, num, usart);
    if (status != ARM_DRIVER_OK) return status;

  } else {
    // Only in synchronous mode
    return ARM_DRIVER_ERROR;
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          uint32_t USART_GetTxCount (USART_RESOURCES *usart)
  \brief       Get transmitted data count.
  \param[in]   usart     Pointer to USART resources
  \return      number of data items transmitted
*/
static uint32_t USART_GetTxCount (USART_RESOURCES *usart) {
  return usart->info->xfer.tx_cnt;
}

/**
  \fn          uint32_t USART_GetRxCount (USART_RESOURCES *usart)
  \brief       Get received data count.
  \param[in]   usart     Pointer to USART resources
  \return      number of data items received
*/
static uint32_t USART_GetRxCount (USART_RESOURCES *usart) {
  return usart->info->xfer.rx_cnt;
}

/**
  \fn          int32_t USART_Control (uint32_t          control,
                                      uint32_t          arg,
                                      USART_RESOURCES  *usart)
  \brief       Control USART Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \param[in]   usart    Pointer to USART resources
  \return      common \ref execution_status and driver specific \ref usart_execution_status
*/
static int32_t USART_Control (uint32_t          control,
                              uint32_t          arg,
                              USART_RESOURCES  *usart) {
  uint32_t val, mode;
  uint32_t syncctrl, hden, icr, scictrl, lcr, mcr;

  if ((usart->info->flags & USART_FLAG_POWERED) == 0) {
    // USART not powered
    return ARM_DRIVER_ERROR;
  }

  syncctrl = 0;
  hden     = 0;
  icr      = 0;
  scictrl  = 0;
  lcr      = 0;

  switch (control & ARM_USART_CONTROL_Msk) {
    case ARM_USART_MODE_ASYNCHRONOUS:
      mode = ARM_USART_MODE_ASYNCHRONOUS;
      break;
    case ARM_USART_MODE_SYNCHRONOUS_MASTER:
      if (usart->capabilities.synchronous_master) {
        // Enable synchronous master (SCLK out) mode
        syncctrl = USART_SYNCCTRL_SYNC | USART_SYNCCTRL_CSRC;
      } else return ARM_USART_ERROR_MODE;
      mode = ARM_USART_MODE_SYNCHRONOUS_MASTER;
      break;
    case ARM_USART_MODE_SYNCHRONOUS_SLAVE:
      if (usart->capabilities.synchronous_slave) {
        // Enable synchronous slave (SCLK in) mode
        syncctrl = USART_SYNCCTRL_SYNC;
      } else return ARM_USART_ERROR_MODE;
      mode = ARM_USART_MODE_SYNCHRONOUS_SLAVE;
      break;
    case ARM_USART_MODE_SINGLE_WIRE:
      // Enable Half duplex
      hden = USART_HDEN_HDEN;
      mode = ARM_USART_MODE_SINGLE_WIRE;
      break;
    case ARM_USART_MODE_IRDA:
      if (usart->capabilities.irda) {
        // Enable IrDA mode
        icr = USART_ICR_IRDAEN;
      } else return ARM_USART_ERROR_MODE;
      mode = ARM_USART_MODE_IRDA;
      break;
    case ARM_USART_MODE_SMART_CARD:
      if (usart->capabilities.smart_card) {
        // Enable Smart card mode
        scictrl = USART_SCICTRL_SCIEN;
      } else return ARM_USART_ERROR_MODE;
      mode = ARM_USART_MODE_SMART_CARD;
      break;

    // Default TX value
    case ARM_USART_SET_DEFAULT_TX_VALUE:
      usart->info->xfer.tx_def_val = arg;
      return ARM_DRIVER_OK;

    // IrDA pulse
    case ARM_USART_SET_IRDA_PULSE:
      if (usart->capabilities.irda) {
        if (arg == 0) {
          usart->reg->ICR &= ~(USART_ICR_FIXPULSEEN);
        } else {
          val = 1000000000 / (GetClockFreq (((*usart->clk.base_clk >> 24) & 0x1F)));
          icr = usart->reg->ICR & ~USART_ICR_PULSEDIV_MSK;
          if      (arg <= (2   * val)) icr |= (0 << USART_ICR_PULSEDIV_POS);
          else if (arg <= (4   * val)) icr |= (1 << USART_ICR_PULSEDIV_POS);
          else if (arg <= (8   * val)) icr |= (2 << USART_ICR_PULSEDIV_POS);
          else if (arg <= (6   * val)) icr |= (3 << USART_ICR_PULSEDIV_POS);
          else if (arg <= (2   * val)) icr |= (4 << USART_ICR_PULSEDIV_POS);
          else if (arg <= (64  * val)) icr |= (5 << USART_ICR_PULSEDIV_POS);
          else if (arg <= (128 * val)) icr |= (6 << USART_ICR_PULSEDIV_POS);
          else if (arg <= (256 * val)) icr |= (7 << USART_ICR_PULSEDIV_POS);
          else return ARM_DRIVER_ERROR;
          usart->reg->ICR = icr | USART_ICR_FIXPULSEEN;
        }
      } else return ARM_DRIVER_ERROR;
      return ARM_DRIVER_OK;

    // SmartCard guard time
    case ARM_USART_SET_SMART_CARD_GUARD_TIME:
      if (usart->capabilities.smart_card) {
        if (arg > 0xFF) return ARM_DRIVER_ERROR;
        usart->reg->SCICTRL &= ~USART_SCICTRL_GUARDTIME_MSK;
        usart->reg->SCICTRL |= (arg << USART_SCICTRL_GUARDTIME_POS);
      } else return ARM_DRIVER_ERROR;
      return ARM_DRIVER_OK;

    // SmartCard clock
    case ARM_USART_SET_SMART_CARD_CLOCK:
      if (usart->capabilities.smart_card == 0) return ARM_DRIVER_ERROR;
      if (arg == 0)                            return ARM_DRIVER_OK;
      if (usart->capabilities.smart_card_clock) {
        if ((usart->info->baudrate * 372) != arg)
          return ARM_DRIVER_ERROR;
      } else return ARM_DRIVER_ERROR;
      return ARM_DRIVER_OK;

     // SmartCard NACK
    case ARM_USART_CONTROL_SMART_CARD_NACK:
      if (usart->capabilities.smart_card) {
        if (arg) usart->reg->SCICTRL &= ~USART_SCICTRL_NACKDIS;
        else     usart->reg->SCICTRL |=  USART_SCICTRL_NACKDIS;
      } else return ARM_DRIVER_ERROR;
      return ARM_DRIVER_OK;

    // Control TX
    case ARM_USART_CONTROL_TX:
      if (arg) {
        if (usart->info->mode != ARM_USART_MODE_SMART_CARD) {
          // USART TX pin function selected
          SCU_PinConfigure(usart->pins.tx->port, usart->pins.tx->num,
                           SCU_PIN_CFG_PULLUP_DIS | SCU_PIN_CFG_INPUT_FILTER_DIS |
                           SCU_PIN_CFG_MODE(usart->pins.tx->config_val));
        }
        usart->info->flags |= USART_FLAG_TX_ENABLED;
        usart->reg->TER |= USART_TER_TXEN;
      } else {
        usart->info->flags &= ~USART_FLAG_TX_ENABLED;
        usart->reg->TER &= ~USART_TER_TXEN;
        if (usart->info->mode != ARM_USART_MODE_SMART_CARD) {
          // GPIO pin function selected
          SCU_PinConfigure(usart->pins.tx->port, usart->pins.tx->num,
                           SCU_PIN_CFG_PULLUP_DIS | SCU_PIN_CFG_INPUT_FILTER_DIS |
                           SCU_PIN_CFG_MODE(SCU_CFG_MODE_FUNC0));
        }
      }
      return ARM_DRIVER_OK;

    // Control RX
    case ARM_USART_CONTROL_RX:
      // RX Line interrupt enable (overrun, framing, parity error, break)
      if (arg) {
        if ((usart->info->mode != ARM_USART_MODE_SMART_CARD)   &&
            (usart->info->mode != ARM_USART_MODE_SINGLE_WIRE )) {
          // USART RX pin function selected
          SCU_PinConfigure(usart->pins.rx->port, usart->pins.rx->num,
                           SCU_PIN_CFG_INPUT_BUFFER_EN | SCU_PIN_CFG_INPUT_FILTER_DIS |
                           SCU_PIN_CFG_PULLUP_DIS | SCU_PIN_CFG_MODE(usart->pins.rx->config_val));
        }
        usart->info->flags |= USART_FLAG_RX_ENABLED;
        usart->reg->RS485CTRL &= ~USART_RS485CTRL_RXDIS;
        usart->reg->IER |= USART_IER_RXIE;
      } else {
        usart->info->flags &= ~USART_FLAG_RX_ENABLED;
        usart->reg->RS485CTRL |= USART_RS485CTRL_RXDIS;
        usart->reg->IER &= ~USART_IER_RXIE;
        if ((usart->info->mode != ARM_USART_MODE_SMART_CARD)   &&
            (usart->info->mode != ARM_USART_MODE_SINGLE_WIRE )) {
          // GPIO pin function selected
          SCU_PinConfigure(usart->pins.rx->port, usart->pins.rx->num,
                           SCU_PIN_CFG_PULLUP_DIS | SCU_PIN_CFG_INPUT_FILTER_DIS |
                           SCU_PIN_CFG_MODE(SCU_CFG_MODE_FUNC0));
        }
      }
      return ARM_DRIVER_OK;

    // Control break
    case ARM_USART_CONTROL_BREAK:
      if (arg) usart->reg->LCR |=  USART_LCR_BC;
      else     usart->reg->LCR &= ~USART_LCR_BC;
      return ARM_DRIVER_OK;

    // Abort Send
    case ARM_USART_ABORT_SEND:
      // Disable transmit holding register empty interrupt
      usart->reg->IER &= ~USART_IER_THREIE;

      // Set trigger level
      val  = (usart->trig_lvl & USART_FCR_RXTRIGLVL_MSK) |
              USART_FCR_FIFOEN;
      if (usart->dma_rx || usart->dma_tx)
        val |= USART_FCR_DMAMODE;

      // Transmit FIFO reset
      val |= USART_FCR_TXFIFORES;
      usart->reg->FCR = val;

      // If DMA mode - disable DMA channel
      if ((usart->dma_tx) && (usart->info->flags & USART_FLAG_SEND_ACTIVE)) {
        GPDMA_ChannelDisable (usart->dma_tx->channel);
      }

      // Clear Send active flag
      usart->info->flags &= ~USART_FLAG_SEND_ACTIVE;
      return ARM_DRIVER_OK;

    // Abort receive
    case ARM_USART_ABORT_RECEIVE:
      // Disable receive data available interrupt
      usart->reg->IER &= ~USART_IER_RBRIE;

      // Set trigger level
      val  = (usart->trig_lvl & USART_FCR_RXTRIGLVL_MSK) |
              USART_FCR_FIFOEN;
      if (usart->dma_rx || usart->dma_tx)
        val |= USART_FCR_DMAMODE;

      // Transmit FIFO reset
      val |= USART_FCR_RXFIFORES;
      usart->reg->FCR = val;

      // If DMA mode - disable DMA channel
      if ((usart->dma_rx) && (usart->info->status.rx_busy)) {
        GPDMA_ChannelDisable (usart->dma_tx->channel);
      }

      // Clear RX busy status
      usart->info->status.rx_busy = 0;
      return ARM_DRIVER_OK;

    // Abort transfer
    case ARM_USART_ABORT_TRANSFER:
      // Disable transmit holding register empty and 
      // receive data available interrupts
      usart->reg->IER &= ~(USART_IER_THREIE | USART_IER_RBRIE);

      // If DMA mode - disable DMA channel
      if ((usart->dma_tx) && (usart->info->flags & USART_FLAG_SEND_ACTIVE)) {
        GPDMA_ChannelDisable (usart->dma_tx->channel);
      }
      if ((usart->dma_rx) && (usart->info->status.rx_busy)) {
        GPDMA_ChannelDisable (usart->dma_tx->channel);
      }

      // Set trigger level
      val  = (usart->trig_lvl & USART_FCR_RXTRIGLVL_MSK) |
              USART_FCR_FIFOEN;
      if (usart->dma_rx || usart->dma_tx)
        val |= USART_FCR_DMAMODE;

      // Transmit FIFO reset
      val |= USART_FCR_TXFIFORES | USART_FCR_RXFIFORES;
      usart->reg->FCR = val;

      // Clear busy statuses
      usart->info->status.rx_busy = 0;
      usart->info->flags &= ~USART_FLAG_SEND_ACTIVE;
     
      return ARM_DRIVER_OK;

    // Unsupported command
    default: return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  // Check if Receiver/Transmitter is busy
  if ( usart->info->status.rx_busy ||
      (usart->info->flags & USART_FLAG_SEND_ACTIVE)) {
    return ARM_DRIVER_ERROR_BUSY;
  }

  // USART Data bits
  switch (control & ARM_USART_DATA_BITS_Msk) {
    case ARM_USART_DATA_BITS_5: lcr |= (0 << USART_LCR_WLS_POS); break;
    case ARM_USART_DATA_BITS_6: lcr |= (1 << USART_LCR_WLS_POS); break;
    case ARM_USART_DATA_BITS_7: lcr |= (2 << USART_LCR_WLS_POS); break;
    case ARM_USART_DATA_BITS_8: lcr |= (3 << USART_LCR_WLS_POS); break;
    default: return ARM_USART_ERROR_DATA_BITS;
  }

  // USART Parity
  switch (control & ARM_USART_PARITY_Msk) {
    case ARM_USART_PARITY_NONE:                                  break;
    case ARM_USART_PARITY_EVEN: lcr |= (1 << USART_LCR_PS_POS) |
                                             USART_LCR_PE;       break;
    case ARM_USART_PARITY_ODD:  lcr |= USART_LCR_PE;             break;
  }

  // USART Stop bits
  switch (control & ARM_USART_STOP_BITS_Msk) {
    case ARM_USART_STOP_BITS_1:                       break;
    case ARM_USART_STOP_BITS_2: lcr |= USART_LCR_SBS; break;
    default: return ARM_USART_ERROR_STOP_BITS;
  }

  // USART Flow control (RTS and CTS lines are only available on USART1)
  if (usart->uart_reg) {
    mcr = usart->uart_reg->MCR & ~(UART_MCR_RTSEN | UART_MCR_CTSEN);
    switch (control & ARM_USART_FLOW_CONTROL_Msk) {
      case ARM_USART_FLOW_CONTROL_NONE:
        break;
      case ARM_USART_FLOW_CONTROL_RTS:
        if (usart->capabilities.flow_control_rts)
          mcr |= UART_MCR_RTSEN;
        else return ARM_USART_ERROR_FLOW_CONTROL;
        break;
      case ARM_USART_FLOW_CONTROL_CTS:
        if (usart->capabilities.flow_control_cts)
          mcr |= UART_MCR_CTSEN;
        else return ARM_USART_ERROR_FLOW_CONTROL;
        break;
      case ARM_USART_FLOW_CONTROL_RTS_CTS:
        if (usart->capabilities.flow_control_rts && 
            usart->capabilities.flow_control_cts) {
          mcr |= (UART_MCR_RTSEN | UART_MCR_CTSEN);
        } else return ARM_USART_ERROR_FLOW_CONTROL;
        break;
      default:
        return ARM_USART_ERROR_FLOW_CONTROL;
    }
  }

  // Clock setting for synchronous mode
  if ((mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
      (mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {

    // Only CPOL0 - CPHA1 combination available

    // USART clock polarity
    if ((control & ARM_USART_CPOL_Msk) != ARM_USART_CPOL0)
      return ARM_USART_ERROR_CPOL;

    // USART clock phase
    if ((control & ARM_USART_CPHA_Msk) != ARM_USART_CPHA1)
      return ARM_USART_ERROR_CPHA;
  }

  // USART Baudrate
  if (USART_SetBaudrate (arg, usart) == -1)
    return ARM_USART_ERROR_BAUDRATE;

  // Configuration is OK - Mode is valid
  usart->info->mode = mode;

  // Configure TX pin regarding mode and transmitter state
  val = SCU_PIN_CFG_INPUT_FILTER_DIS | SCU_PIN_CFG_PULLUP_DIS;
  switch (usart->info->mode) {
    case ARM_USART_MODE_SMART_CARD:
      // Pin function = USART TX
      val |= SCU_PIN_CFG_MODE(usart->pins.tx->config_val);
      break;
    default:
      // Synchronous master/slave, asynchronous, single-wire and IrDA mode
      if (usart->info->flags & USART_FLAG_TX_ENABLED) {
        // Pin function = USART TX
        val |= SCU_PIN_CFG_MODE(usart->pins.tx->config_val);
      } else {
        // Pin function = GPIO
        val |= SCU_PIN_CFG_MODE(SCU_CFG_MODE_FUNC0);
      }
  }
  SCU_PinConfigure(usart->pins.tx->port, usart->pins.tx->num, val);

  // Configure RX pin regarding mode and receiver state
  val = SCU_PIN_CFG_INPUT_FILTER_DIS | SCU_PIN_CFG_PULLUP_DIS;
  switch (usart->info->mode) {
    case ARM_USART_MODE_SINGLE_WIRE:
    case ARM_USART_MODE_SMART_CARD:
      // Pin function = GPIO
      val |= SCU_PIN_CFG_MODE(SCU_CFG_MODE_FUNC0);
      break;
    default:
      // Synchronous master/slave, asynchronous and  IrDA mode
       if (usart->info->flags & USART_FLAG_TX_ENABLED) {
        // Pin function = USART RX
        val |= SCU_PIN_CFG_INPUT_BUFFER_EN | 
               SCU_PIN_CFG_MODE(usart->pins.tx->config_val);
       } else {
        // Pin function = GPIO
        val |= SCU_PIN_CFG_MODE(SCU_CFG_MODE_FUNC0);
       }
      break;
  }
  SCU_PinConfigure(usart->pins.rx->port, usart->pins.rx->num, val);

  // Configure CLK pin regarding mode
  if (usart->pins.clk) {
    val = SCU_PIN_CFG_PULLUP_DIS | SCU_PIN_CFG_INPUT_FILTER_DIS;
    switch (usart->info->mode) {
      case ARM_USART_MODE_SMART_CARD:
      case ARM_USART_MODE_SYNCHRONOUS_MASTER:
        // Pin function = USART UCLK (output)
        val |= SCU_PIN_CFG_MODE(usart->pins.clk->config_val);
        break;
      case ARM_USART_MODE_SYNCHRONOUS_SLAVE:
        // Pin function = USART UCLK (input)
        val |= SCU_PIN_CFG_INPUT_BUFFER_EN |
               SCU_PIN_CFG_MODE(usart->pins.clk->config_val);
        break;
      default:
        // Asynchronous, Single-wire and IrDA mode
        // Pin function = GPIO
        val |= SCU_PIN_CFG_INPUT_BUFFER_EN |
               SCU_PIN_CFG_MODE(SCU_CFG_MODE_FUNC0);
    }
    SCU_PinConfigure(usart->pins.clk->port, usart->pins.clk->num, val);
  }

  // Configure SYNCCRTL register (only in synchronous mode)
  if (usart->capabilities.synchronous_master ||
      usart->capabilities.synchronous_slave) {
    usart->reg->SYNCCTRL = USART_SYNCCTRL_FES    |
                           USART_SYNCCTRL_SSSDIS |
                           syncctrl;
  }

  // Configure HDEN register (only in single wire mode)
  if (usart->capabilities.single_wire)
    usart->reg->HDEN = hden;

  // Configure ICR register (only in IrDA mode)
  if (usart->capabilities.irda)
    usart->reg->ICR = (usart->reg->ICR & ~USART_ICR_IRDAEN) | icr;

  // Configure SCICTRL register (only in Smart Card mode)
  if (usart->capabilities.smart_card) {
    usart->reg->SCICTRL = (usart->reg->SCICTRL & ~USART_SCICTRL_SCIEN) |
                           scictrl;
  }

  // Configure MCR register (modem line for USART1)
  if (usart->uart_reg) {
    usart->uart_reg->MCR = ((usart->uart_reg->MCR & ~(UART_MCR_RTSEN |
                             UART_MCR_CTSEN))) | mcr;
  }

  // Configure Line control register
  usart->reg->LCR = ((usart->reg->LCR & (USART_LCR_BC | USART_LCR_DLAB)) | lcr);

  // Set configured flag
  usart->info->flags |= USART_FLAG_CONFIGURED;

  return ARM_DRIVER_OK;
}

/**
  \fn          ARM_USART_STATUS USART_GetStatus (USART_RESOURCES *usart)
  \brief       Get USART status.
  \param[in]   usart     Pointer to USART resources
  \return      USART status \ref ARM_USART_STATUS
*/
static ARM_USART_STATUS USART_GetStatus (USART_RESOURCES *usart) {
  usart->info->status.tx_busy = (usart->reg->LSR & USART_LSR_TEMT ? (0) : (1));
  return usart->info->status;
}

/**
  \fn          int32_t USART_SetModemControl (ARM_USART_MODEM_CONTROL  control,
                                              USART_RESOURCES         *usart)
  \brief       Set USART Modem Control line state.
  \param[in]   control   \ref ARM_USART_MODEM_CONTROL
  \param[in]   usart     Pointer to USART resources
  \return      \ref execution_status
*/
static int32_t USART_SetModemControl (ARM_USART_MODEM_CONTROL  control,
                                      USART_RESOURCES         *usart) {

  if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0) {
    // USART is not configured
    return ARM_DRIVER_ERROR;
  }

  // Only UART1 supports modem lines
  if (usart->uart_reg == NULL) return ARM_DRIVER_ERROR_UNSUPPORTED;

  if (control == ARM_USART_RTS_CLEAR) {
    if (usart->capabilities.rts) usart->uart_reg->MCR &= ~UART_MCR_RTSCTRL;
    else return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  if (control == ARM_USART_RTS_SET) {
    if (usart->capabilities.rts) usart->uart_reg->MCR |=  UART_MCR_RTSCTRL;
    else return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  if (control == ARM_USART_DTR_CLEAR) {
    if (usart->capabilities.dtr) usart->uart_reg->MCR &= ~UART_MCR_DTRCTRL;
    else return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  if (control == ARM_USART_DTR_SET) {
    if (usart->capabilities.dtr) usart->uart_reg->MCR |=  UART_MCR_DTRCTRL;
    else return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          ARM_USART_MODEM_STATUS USART_GetModemStatus (USART_RESOURCES *usart)
  \brief       Get USART Modem Status lines state.
  \param[in]   usart     Pointer to USART resources
  \return      modem status \ref ARM_USART_MODEM_STATUS
*/
static ARM_USART_MODEM_STATUS USART_GetModemStatus (USART_RESOURCES *usart) {
  ARM_USART_MODEM_STATUS modem_status;
  uint32_t msr;

  if (usart->uart_reg &&
     (usart->info->flags & USART_FLAG_CONFIGURED)) {

    msr = usart->uart_reg->MSR;

    if (usart->capabilities.cts) modem_status.cts = (msr & UART_MSR_CTS ? (1) : (0));
    else                         modem_status.cts = 0;
    if (usart->capabilities.dsr) modem_status.dsr = (msr & UART_MSR_DSR ? (1) : (0));
    else                         modem_status.dsr = 0;
    if (usart->capabilities.ri ) modem_status.ri  = (msr & UART_MSR_RI  ? (1) : (0));
    else                         modem_status.ri  = 0;
    if (usart->capabilities.dcd) modem_status.dcd = (msr & UART_MSR_DCD ? (1) : (0));
    else                         modem_status.dcd = 0;
  } else {
     modem_status.cts = 0;
     modem_status.dsr = 0;
     modem_status.ri  = 0;
     modem_status.dcd = 0;
  }

  return modem_status;
}

/**
  \fn          void USART_IRQHandler (UART_RESOURCES *usart)
  \brief       USART Interrupt handler.
  \param[in]   usart     Pointer to USART resources
*/
static void USART_IRQHandler (USART_RESOURCES *usart) {
  uint32_t iir, event, val;

  event = 0;
  iir   = usart->reg->IIR;

  if ((iir & USART_IIR_INTSTATUS) == 0) {

    // Transmit holding register empty
    if ((iir & USART_IIR_INTID_MSK) == USART_IIR_INTID_THRE) {
      val = 16;
      while ((val --) && (usart->info->xfer.tx_num != usart->info->xfer.tx_cnt)) {
        if (((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER)  ||
             (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) &&
             (usart->info->xfer.sync_mode == USART_SYNC_MODE_RX)) {
          // Dummy write in synchronous receive only mode
          usart->reg->THR = usart->info->xfer.tx_def_val;
        } else {
          // Write data to Tx FIFO      
          usart->reg->THR = usart->info->xfer.tx_buf[usart->info->xfer.tx_cnt];
        }
        usart->info->xfer.tx_cnt++;
      }

      // Check if all data is transmitted
      if (usart->info->xfer.tx_num == usart->info->xfer.tx_cnt) {
        // Disable THRE interrupt
        usart->reg->IER &= ~USART_IER_THREIE;

        // Clear TX busy flag
        usart->info->flags &= ~USART_FLAG_SEND_ACTIVE;

        // Set send complete event
        if ((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
            (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {
          if ((usart->info->xfer.sync_mode == USART_SYNC_MODE_TX)    &&
              ((usart->info->flags & USART_FLAG_RX_ENABLED) == 0)) {
            event |= ARM_USART_EVENT_SEND_COMPLETE;
          }
        } else {
          event |= ARM_USART_EVENT_SEND_COMPLETE;
        }       
      }
    }

    // Receive line status
    if ((iir & USART_IIR_INTID_MSK) == USART_IIR_INTID_RLS) {
      event |= USART_RxLineIntHandler(usart);
    }

    // Receive data available and Character time-out indicator interrupt
    if (((iir & USART_IIR_INTID_MSK) == USART_IIR_INTID_RDA)  |
        ((iir & USART_IIR_INTID_MSK) == USART_IIR_INTID_CTI)) {

      // Get all available data from RX FIFO
      while (usart->reg->LSR & USART_LSR_RDR) {
        // Check RX line interrupt for errors
        event |= USART_RxLineIntHandler (usart);

        if (((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER)  ||
             (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) &&
             (usart->info->xfer.sync_mode == USART_SYNC_MODE_TX)) {
          // Dummy read in synchronous transmit only mode
          usart->reg->RBR;
        } else {
          // Read data from RX FIFO into receive buffer
          usart->info->xfer.rx_buf[usart->info->xfer.rx_cnt] = usart->reg->RBR;
        }

        usart->info->xfer.rx_cnt++;

        // Check if requested amount of data is received
        if (usart->info->xfer.rx_cnt == usart->info->xfer.rx_num) {
          // Disable RDA interrupt
          usart->reg->IER &= ~USART_IER_RBRIE;

          // Clear RX busy flag and set receive transfer complete event
          usart->info->status.rx_busy = 0;
          if ((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
              (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {
            val = usart->info->xfer.sync_mode;
            usart->info->xfer.sync_mode = 0;
            switch (val) {
              case USART_SYNC_MODE_TX:
                event |= ARM_USART_EVENT_SEND_COMPLETE;
                break;
              case USART_SYNC_MODE_RX:
                event |= ARM_USART_EVENT_RECEIVE_COMPLETE;
                break;
              case USART_SYNC_MODE_TX_RX:
                event |= ARM_USART_EVENT_TRANSFER_COMPLETE;
                break;
            }
          } else {
            event |= ARM_USART_EVENT_RECEIVE_COMPLETE;
          }
          break;
        }
      }
    }

    // Character time-out indicator
    if ((iir & USART_IIR_INTID_MSK) == USART_IIR_INTID_CTI) {
      if ((usart->info->mode != ARM_USART_MODE_SYNCHRONOUS_MASTER) &&
          (usart->info->mode != ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {
        // Signal RX Time-out event, if not all requested data received
        if (usart->info->xfer.rx_cnt != usart->info->xfer.rx_num) {
          event |= ARM_USART_EVENT_RX_TIMEOUT;
        }
      }
    }

    // Modem interrupt (UART1 only)
#if (RTE_UART1)
    if (usart->uart_reg) {
      if ((iir & USART_IIR_INTID_MSK) == UART_IIR_INTID_MS) {
        // Save modem status register
        val = usart->uart_reg->MSR;
      
        // CTS state changed
        if ((usart->capabilities.cts) && (val & UART_MSR_DCTS))
          event |= ARM_USART_EVENT_CTS;
        // DSR state changed
        if ((usart->capabilities.dsr) && (val & UART_MSR_DDSR))
          event |= ARM_USART_EVENT_DSR;
        // Ring indicator
        if ((usart->capabilities.ri)  && (val & UART_MSR_TERI))
          event |= ARM_USART_EVENT_RI;
        // DCD state changed
        if ((usart->capabilities.dcd) && (val & UART_MSR_DDCD))
          event |= ARM_USART_EVENT_DCD;
      }
    }
#endif
  }
  if (usart->info->cb_event && event)
    usart->info->cb_event (event);
}

#if (((RTE_USART0) && (RTE_USART0_DMA_TX_EN == 1)) || \
     ((RTE_UART1)  && (RTE_UART1_DMA_TX_EN  == 1)) || \
     ((RTE_USART2) && (RTE_USART2_DMA_TX_EN == 1)) || \
     ((RTE_USART3) && (RTE_USART3_DMA_TX_EN == 1)))
/**
  \fn          void USART_GPDMA_Tx_Event (uint32_t event, USART_RESOURCES *usart)
  \brief       UART Interrupt handler.
  \param[in]   usart     Pointer to USART resources
  \param[in]   event     GPDMA_EVENT_TERMINAL_COUNT_REQUEST / GPDMA_EVENT_ERROR
*/
static void USART_GPDMA_Tx_Event (uint32_t event, USART_RESOURCES *usart) {
  switch (event) {
    case GPDMA_EVENT_TERMINAL_COUNT_REQUEST:
      usart->info->xfer.tx_cnt = usart->info->xfer.tx_num;
      // Clear TX busy flag
      usart->info->flags &= ~USART_FLAG_SEND_ACTIVE;

      // Set Send Complete event for asynchronous transfers
      if ((usart->info->mode != ARM_USART_MODE_SYNCHRONOUS_MASTER) &&
          (usart->info->mode != ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {
        if (usart->info->cb_event)
          usart->info->cb_event (ARM_USART_EVENT_SEND_COMPLETE);
      }
      break;
    case GPDMA_EVENT_ERROR:
      break;
  }
}
#endif

#if (((RTE_USART0) && (RTE_USART0_DMA_RX_EN == 1)) || \
     ((RTE_UART1)  && (RTE_UART1_DMA_RX_EN  == 1)) || \
     ((RTE_USART2) && (RTE_USART2_DMA_RX_EN == 1)) || \
     ((RTE_USART3) && (RTE_USART3_DMA_RX_EN == 1)))
/**
  \fn          void USART_GPDMA_Rx_Event (uint32_t event, USART_RESOURCES *usart)
  \brief       UART Interrupt handler.
  \param[in]   event     GPDMA_EVENT_TERMINAL_COUNT_REQUEST / GPDMA_EVENT_ERROR
  \param[in]   usart     Pointer to USART resources
*/
static void USART_GPDMA_Rx_Event (uint32_t event, USART_RESOURCES *usart) {
  uint32_t val, evt;

  evt = 0;

  switch (event) {
    case GPDMA_EVENT_TERMINAL_COUNT_REQUEST:
      usart->info->xfer.rx_cnt    = usart->info->xfer.rx_num; 
      usart->info->status.rx_busy = 0;

    if ((usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_MASTER) ||
          (usart->info->mode == ARM_USART_MODE_SYNCHRONOUS_SLAVE )) {
        val = usart->info->xfer.sync_mode;
        usart->info->xfer.sync_mode = 0;
        switch (val) {
          case USART_SYNC_MODE_TX:
            evt |= ARM_USART_EVENT_SEND_COMPLETE;
            break;
          case USART_SYNC_MODE_RX:
            evt |= ARM_USART_EVENT_RECEIVE_COMPLETE;
            break;
          case USART_SYNC_MODE_TX_RX:
            evt |= ARM_USART_EVENT_TRANSFER_COMPLETE;
             break;
        }
      } else {
        evt |= ARM_USART_EVENT_RECEIVE_COMPLETE;
      }
      if (usart->info->cb_event && evt)
        usart->info->cb_event (evt);
      break;
    case GPDMA_EVENT_ERROR:
      break;
  }
}
#endif


#if (RTE_USART0)
// USART0 Driver Wrapper functions
static ARM_USART_CAPABILITIES USART0_GetCapabilities (void) {
  return USART_GetCapabilities (&USART0_Resources);
}
static int32_t USART0_Initialize (ARM_USART_SignalEvent_t cb_event) {
  return USART_Initialize (cb_event, &USART0_Resources);
}
static int32_t USART0_Uninitialize (void) {
  return USART_Uninitialize(&USART0_Resources);
}
static int32_t USART0_PowerControl (ARM_POWER_STATE state) {
  return USART_PowerControl (state, &USART0_Resources);
}
static int32_t USART0_Send (const void *data, uint32_t num) {
  return USART_Send (data, num, &USART0_Resources);
}
static int32_t USART0_Receive (void *data, uint32_t num) {
  return USART_Receive (data, num, &USART0_Resources);
}
static int32_t USART0_Transfer (const void      *data_out,
                                      void      *data_in,
                                      uint32_t   num) {
  return USART_Transfer (data_out, data_in, num, &USART0_Resources);
}
static uint32_t USART0_GetTxCount (void) {
  return USART_GetTxCount (&USART0_Resources);
}
static uint32_t USART0_GetRxCount (void) {
  return USART_GetRxCount (&USART0_Resources); 
}
static int32_t USART0_Control (uint32_t control, uint32_t arg) {
  return USART_Control (control, arg, &USART0_Resources);
}
static ARM_USART_STATUS USART0_GetStatus (void) {
  return USART_GetStatus (&USART0_Resources);
}
static int32_t USART0_SetModemControl (ARM_USART_MODEM_CONTROL control) {
  return USART_SetModemControl (control, &USART0_Resources);
}
static ARM_USART_MODEM_STATUS USART0_GetModemStatus (void) {
  return USART_GetModemStatus (&USART0_Resources);
}
void UART0_IRQHandler (void) {
  USART_IRQHandler (&USART0_Resources);
}
#if (RTE_USART0_DMA_TX_EN == 1)
void USART0_GPDMA_Tx_Event (uint32_t event) {
  USART_GPDMA_Tx_Event(event, &USART0_Resources);
}
#endif
#if (RTE_USART0_DMA_RX_EN == 1)
void USART0_GPDMA_Rx_Event (uint32_t event) {
  USART_GPDMA_Rx_Event(event, &USART0_Resources);
}
#endif

// USART0 Driver Control Block
ARM_DRIVER_USART Driver_USART0 = {
    USARTx_GetVersion,
    USART0_GetCapabilities,
    USART0_Initialize,
    USART0_Uninitialize,
    USART0_PowerControl,
    USART0_Send, 
    USART0_Receive,
    USART0_Transfer,
    USART0_GetTxCount,
    USART0_GetRxCount,
    USART0_Control,
    USART0_GetStatus,
    USART0_SetModemControl,
    USART0_GetModemStatus
};
#endif

#if (RTE_UART1)
// USART1 Driver Wrapper functions
static ARM_USART_CAPABILITIES USART1_GetCapabilities (void) {
  return USART_GetCapabilities (&USART1_Resources);
}
static int32_t USART1_Initialize (ARM_USART_SignalEvent_t cb_event) {
  return USART_Initialize (cb_event, &USART1_Resources);
}
static int32_t USART1_Uninitialize (void) {
  return USART_Uninitialize(&USART1_Resources);
}
static int32_t USART1_PowerControl (ARM_POWER_STATE state) {
  return USART_PowerControl (state, &USART1_Resources);
}
static int32_t USART1_Send (const void *data, uint32_t num) {
  return USART_Send (data, num, &USART1_Resources);
}
static int32_t USART1_Receive (void *data, uint32_t num) {
  return USART_Receive (data, num, &USART1_Resources);
}
static int32_t USART1_Transfer (const void      *data_out,
                                      void      *data_in,
                                      uint32_t   num) {
  return USART_Transfer (data_out, data_in, num, &USART1_Resources);
}
static uint32_t USART1_GetTxCount (void) {
  return USART_GetTxCount (&USART1_Resources);
}
static uint32_t USART1_GetRxCount (void) {
  return USART_GetRxCount (&USART1_Resources); 
}
static int32_t USART1_Control (uint32_t control, uint32_t arg) {
  return USART_Control (control, arg, &USART1_Resources);
}
static ARM_USART_STATUS USART1_GetStatus (void) {
  return USART_GetStatus (&USART1_Resources);
}
static int32_t USART1_SetModemControl (ARM_USART_MODEM_CONTROL control) {
  return USART_SetModemControl (control, &USART1_Resources);
}
static ARM_USART_MODEM_STATUS USART1_GetModemStatus (void) {
  return USART_GetModemStatus (&USART1_Resources);
}
void UART1_IRQHandler (void) {
  USART_IRQHandler (&USART1_Resources);
}
#if (RTE_UART1_DMA_TX_EN == 1)
void USART1_GPDMA_Tx_Event (uint32_t event) {
  USART_GPDMA_Tx_Event(event, &USART1_Resources);
}
#endif
#if (RTE_UART1_DMA_RX_EN == 1)
void USART1_GPDMA_Rx_Event (uint32_t event) {
  USART_GPDMA_Rx_Event(event, &USART1_Resources);
}
#endif

// USART1 Driver Control Block
ARM_DRIVER_USART Driver_USART1 = {
    USARTx_GetVersion,
    USART1_GetCapabilities,
    USART1_Initialize,
    USART1_Uninitialize,
    USART1_PowerControl,
    USART1_Send, 
    USART1_Receive,
    USART1_Transfer,
    USART1_GetTxCount,
    USART1_GetRxCount,
    USART1_Control,
    USART1_GetStatus,
    USART1_SetModemControl,
    USART1_GetModemStatus
};
#endif

#if (RTE_USART2)
// USART2 Driver Wrapper functions
static ARM_USART_CAPABILITIES USART2_GetCapabilities (void) {
  return USART_GetCapabilities (&USART2_Resources);
}
static int32_t USART2_Initialize (ARM_USART_SignalEvent_t cb_event) {
  return USART_Initialize (cb_event, &USART2_Resources);
}
static int32_t USART2_Uninitialize (void) {
  return USART_Uninitialize(&USART2_Resources);
}
static int32_t USART2_PowerControl (ARM_POWER_STATE state) {
  return USART_PowerControl (state, &USART2_Resources);
}
static int32_t USART2_Send (const void *data, uint32_t num) {
  return USART_Send (data, num, &USART2_Resources);
}
static int32_t USART2_Receive (void *data, uint32_t num) {
  return USART_Receive (data, num, &USART2_Resources);
}
static int32_t USART2_Transfer (const void      *data_out,
                                      void      *data_in,
                                      uint32_t   num) {
  return USART_Transfer (data_out, data_in, num, &USART2_Resources);
}
static uint32_t USART2_GetTxCount (void) {
  return USART_GetTxCount (&USART2_Resources);
}
static uint32_t USART2_GetRxCount (void) {
  return USART_GetRxCount (&USART2_Resources); 
}
static int32_t USART2_Control (uint32_t control, uint32_t arg) {
  return USART_Control (control, arg, &USART2_Resources);
}
static ARM_USART_STATUS USART2_GetStatus (void) {
  return USART_GetStatus (&USART2_Resources);
}
static int32_t USART2_SetModemControl (ARM_USART_MODEM_CONTROL control) {
  return USART_SetModemControl (control, &USART2_Resources);
}
static ARM_USART_MODEM_STATUS USART2_GetModemStatus (void) {
  return USART_GetModemStatus (&USART2_Resources);
}
void UART2_IRQHandler (void) {
  USART_IRQHandler (&USART2_Resources);
}
#if (RTE_USART2_DMA_TX_EN == 1)
void USART2_GPDMA_Tx_Event (uint32_t event) {
  USART_GPDMA_Tx_Event(event, &USART2_Resources);
}
#endif
#if (RTE_USART2_DMA_RX_EN == 1)
void USART2_GPDMA_Rx_Event (uint32_t event) {
  USART_GPDMA_Rx_Event(event, &USART2_Resources);
}
#endif

// USART2 Driver Control Block
ARM_DRIVER_USART Driver_USART2 = {
    USARTx_GetVersion,
    USART2_GetCapabilities,
    USART2_Initialize,
    USART2_Uninitialize,
    USART2_PowerControl,
    USART2_Send, 
    USART2_Receive,
    USART2_Transfer,
    USART2_GetTxCount,
    USART2_GetRxCount,
    USART2_Control,
    USART2_GetStatus,
    USART2_SetModemControl,
    USART2_GetModemStatus
};
#endif

#if (RTE_USART3)
// USART3 Driver Wrapper functions
static ARM_USART_CAPABILITIES USART3_GetCapabilities (void) {
  return USART_GetCapabilities (&USART3_Resources);
}
static int32_t USART3_Initialize (ARM_USART_SignalEvent_t cb_event) {
  return USART_Initialize (cb_event, &USART3_Resources);
}
static int32_t USART3_Uninitialize (void) {
  return USART_Uninitialize(&USART3_Resources);
}
static int32_t USART3_PowerControl (ARM_POWER_STATE state) {
  return USART_PowerControl (state, &USART3_Resources);
}
static int32_t USART3_Send (const void *data, uint32_t num) {
  return USART_Send (data, num, &USART3_Resources);
}
static int32_t USART3_Receive (void *data, uint32_t num) {
  return USART_Receive (data, num, &USART3_Resources);
}
static int32_t USART3_Transfer (const void      *data_out,
                                      void      *data_in,
                                      uint32_t   num) {
  return USART_Transfer (data_out, data_in, num, &USART3_Resources);
}
static uint32_t USART3_GetTxCount (void) {
  return USART_GetTxCount (&USART3_Resources);
}
static uint32_t USART3_GetRxCount (void) {
  return USART_GetRxCount (&USART3_Resources); 
}
static int32_t USART3_Control (uint32_t control, uint32_t arg) {
  return USART_Control (control, arg, &USART3_Resources);
}
static ARM_USART_STATUS USART3_GetStatus (void) {
  return USART_GetStatus (&USART3_Resources);
}
static int32_t USART3_SetModemControl (ARM_USART_MODEM_CONTROL control) {
  return USART_SetModemControl (control, &USART3_Resources);
}
static ARM_USART_MODEM_STATUS USART3_GetModemStatus (void) {
  return USART_GetModemStatus (&USART3_Resources);
}
void UART3_IRQHandler (void) {
  USART_IRQHandler (&USART3_Resources);
}
#if (RTE_USART3_DMA_TX_EN == 1)
void USART3_GPDMA_Tx_Event (uint32_t event) {
  USART_GPDMA_Tx_Event(event, &USART3_Resources);
}
#endif
#if (RTE_USART3_DMA_RX_EN == 1)
void USART3_GPDMA_Rx_Event (uint32_t event) {
  USART_GPDMA_Rx_Event(event, &USART3_Resources);
}
#endif

// USART3 Driver Control Block
ARM_DRIVER_USART Driver_USART3 = {
    USARTx_GetVersion,
    USART3_GetCapabilities,
    USART3_Initialize,
    USART3_Uninitialize,
    USART3_PowerControl,
    USART3_Send, 
    USART3_Receive,
    USART3_Transfer,
    USART3_GetTxCount,
    USART3_GetRxCount,
    USART3_Control,
    USART3_GetStatus,
    USART3_SetModemControl,
    USART3_GetModemStatus
};
#endif
