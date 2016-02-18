/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_usart.h"
#include "cmsis_os.h"
#include "usart.h"
#include "stdio.h"
#include "stdlib.h"

/* Defines -------------------------------------------------------------------*/
// Altera andar atual
#define CURRENT_FLOOR(elevator,floor) (elevator.currentPosition.floors = 0); (elevator.currentPosition.floors = 1 << floor);

// Adiciona andar a lista de requests
#define REQUEST_FROM(elevator,floor) (elevator.requests.floors |= 1 << floor);

// Checa se o andar atual esta nos requests
#define CHECK(_elevator) (##_elevator.currentPosition.floors & ##_elevator.requests.floors)

// Remove andar atual dos requests
#define ATTENDED_REQUEST(_elevator) (##_elevator.requests.floors &= ~(##_elevator.currentPosition.floors));

// Testa se elevador tem requests nao atendidos
#define HAS_REQUESTS(_elevator) (##_elevator.requests.floors > 0)

// Testa se tem requests acima
#define HAS_REQUEST_ABOVE(_elevator) (##_elevator.currentPosition.floors < ##_elevator.requests.floors)

/* Exported types ------------------------------------------------------------*/

typedef struct{
	uint8_t msg[8];
	uint8_t size;
} RxMessage;

typedef union
{
    struct{
        uint16_t floors;
    };
    struct{
        uint8_t _00 : 1;
        uint8_t _01 : 1;
        uint8_t _02 : 1;
        uint8_t _03 : 1;
        uint8_t _04 : 1;
        uint8_t _05 : 1;
        uint8_t _06 : 1;
        uint8_t _07 : 1;
        uint8_t _08 : 1;
        uint8_t _09 : 1;
        uint8_t _10 : 1;
        uint8_t _11 : 1;
        uint8_t _12 : 1;
        uint8_t _13 : 1;
        uint8_t _14 : 1;
        uint8_t _15 : 1;
    } floor;
} BuildingFloors;

typedef enum {Stopped, GoingUp, GoingDown} ElevatorState;

typedef struct{
    ElevatorState state;
    BuildingFloors currentPosition;
    BuildingFloors requests;
} ElevatorStatus;


/* Exported variables --------------------------------------------------------*/

extern RxMessage recvMessage;          /* Received messages buffer */
extern uint8_t msgSize;                /* Received message size */
extern osMessageQId mqid_rxMessages;   /* Message queue id of: rxMessages   */
extern osPoolId  mpool_rxMessages;     /* Pool Alloc id of: rxMessages   */

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#endif /* __MAIN_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
