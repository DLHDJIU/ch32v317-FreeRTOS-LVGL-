/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_it.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2024/03/06
* Description        : Main Interrupt Service Routines.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32v30x_it.h"

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
  while (1)
  {
  }
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler(void)
{
  uint32_t mcause = __get_MCAUSE();
  uint32_t mepc = __get_MEPC();
  uint32_t mtval = __get_MTVAL();
  uint32_t sp = __get_SP();

  /* Keep the first fault visible. Automatic reset previously hid the cause. */
  printf("\r\n[HARDFAULT] mcause=0x%08lX mepc=0x%08lX "
         "mtval=0x%08lX sp=0x%08lX\r\n",
         (unsigned long)mcause, (unsigned long)mepc,
         (unsigned long)mtval, (unsigned long)sp);
  while (1)
  {
  }
}


