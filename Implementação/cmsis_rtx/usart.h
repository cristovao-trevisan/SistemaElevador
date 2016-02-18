#ifndef __USART_H__
#define __USART_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* Functions Prototypes ------------------------------------------------------*/
void initUSART(int baudrate);
void sendData(USART_TypeDef* USARTx, volatile uint8_t Data);
void USART_puts(USART_TypeDef* USARTx, volatile char *s);

#endif /* __USART_H__ */