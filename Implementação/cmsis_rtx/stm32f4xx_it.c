/**
  ******************************************************************************
  * @file    SysTick/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup SysTick_Example
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//volatile unsigned int valor_AD, valor_DA;
//volatile unsigned int Conversao=0;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}

//------------------------------------
// Esta interrupção foi projetada para que o instante de amostragem do A/D e D/A
// sejam sempre os mesmos em relação ao timer que gerou a interrupção
//------------------------------------
//void SysTick_Handler(void)
//{
//  	//-----------le valor da conversao
//	valor_AD = ADC_GetConversionValue( ADC1 );
//	//-----------Reinicia AD
//        ADC_SoftwareStartConv(ADC1);
//	//-----------escreve no DAC
//       /* Set DAC Channel1 DHR12R register */
//        DAC_SetChannel1Data(DAC_Align_12b_R, valor_DA);
//	//-----------manda para a saida analógica
//       /* Start DAC Channel1 conversion by software */
//        DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);
//	Conversao++;
//	// Toogle no LED 4 para verificarmos a correta temporização
//        STM_EVAL_LEDToggle(LED4);  // LED 4 é o pino PD12 do programa principal
//        TimingDelay_Decrement();
//}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/*void USART1_IRQHandler(void) {
    
    static uint16_t RxByte = 0x00; 
    
    if (USART_GetITStatus(USART1, USART_IT_TC) == SET)
    {
        
        if (USART_GetFlagStatus(USART1, USART_FLAG_TC))
        {
            USART_SendData(USART1, RxByte);
            USART_ITConfig(USART1, USART_IT_TC, DISABLE);
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_TC);
    }
    
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
        {
            RxByte = USART_ReceiveData(USART1);
            osMessagePut(mqid_rxMessages, RxByte, 0);
            USART_ITConfig(USART1, USART_IT_TC, ENABLE);
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}*/

void USART1_IRQHandler(void) {
    
    static uint16_t RxByte = 0x00; 
    
    if (USART_GetITStatus(USART1, USART_IT_TC) == SET)
    {
        
        if (USART_GetFlagStatus(USART1, USART_FLAG_TC))
        {
            USART_SendData(USART1, RxByte);
            USART_ITConfig(USART1, USART_IT_TC, DISABLE);
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_TC);
    }
    
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
        {
            RxByte = USART_ReceiveData(USART1);
            
            if((RxByte != 0xd) && (RxByte != 0xa) &&(msgSize < 8))
            {
                recvMessage.msg[msgSize++] = (uint8_t) RxByte;
                recvMessage.size = msgSize;    
            }else {
                RxMessage *msg = (RxMessage*) osPoolAlloc(mpool_rxMessages);
                msg->msg[0] = recvMessage.msg[0];
                msg->msg[1] = recvMessage.msg[1];
                msg->msg[2] = recvMessage.msg[2];
                msg->msg[3] = recvMessage.msg[3];
                msg->msg[4] = recvMessage.msg[4];
                msg->msg[5] = recvMessage.msg[5];
                msg->msg[6] = recvMessage.msg[6];
                msg->msg[7] = recvMessage.msg[7];
                msg->size = recvMessage.size;
                osMessagePut(mqid_rxMessages, (uint32_t) msg, 0);
                recvMessage.msg[0] = 0;
                recvMessage.msg[1] = 0;
                recvMessage.msg[2] = 0;
                recvMessage.msg[3] = 0;
                recvMessage.msg[4] = 0;
                recvMessage.msg[5] = 0;
                recvMessage.msg[6] = 0;
                recvMessage.msg[7] = 0;
                recvMessage.size = 0;
                msgSize = 0;
            }
            
            //USART_ITConfig(USART1, USART_IT_TC, ENABLE);
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
