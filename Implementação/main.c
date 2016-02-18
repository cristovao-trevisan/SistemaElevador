#include "main.h"

ElevatorStatus elevatorStatusLeft;
ElevatorStatus elevatorStatusCenter;
ElevatorStatus elevatorStatusRight;


void write2USART(uint8_t*, uint8_t);
int evaluation(ElevatorStatus elevator, int requestedFloor, int buttonUp);


RxMessage recvMessage;          /* Received messages buffer */
uint8_t msgSize = 0;            /* Received message size */

// new
RxMessage fwMessage;

/*----------------------------------------------------------------------------
Tasks definitions
*---------------------------------------------------------------------------*/
osThreadId tid_controlUnit;     /* Thread id of thread: controlUnit  */
osThreadId tid_leftElevator;    /* Thread id of thread: leftElevator */
osThreadId tid_centerElevator;
osThreadId tid_rightElevator;   /* Thread id of thread: rightElevator */

// new
osThreadId tid_outputSerial;

/*----------------------------------------------------------------------------
Messages queues
*---------------------------------------------------------------------------*/
osMessageQId mqid_rxMessages;           /* Message queue id of: rxMessages   */
osMessageQId mqid_msgBoxLeftElevator;   /* Message queue id of: msgBoxLeftElevator */
osMessageQId mqid_msgBoxCenterElevator;
osMessageQId mqid_msgBoxRightElevator;  /* Message queue id of: msgBoxRightElevator */

// new
osMessageQId mqid_msgOutputSerial;

osMessageQDef(mqid_rxMessages, 8, RxMessage);
osMessageQDef(mqid_msgBoxLeftElevator, 8, unsigned char);
osMessageQDef(mqid_msgBoxCenterElevator, 8, unsigned char);
osMessageQDef(mqid_msgBoxRightElevator, 8, unsigned char);

// new
osMessageQDef(mqid_msgOutputSerial, 8, unsigned char);

/*----------------------------------------------------------------------------
Pool Alloc
*---------------------------------------------------------------------------*/
osPoolId  mpool_rxMessages;             /* Pool Alloc id of: rxMessages   */
osPoolDef(mpool_rxMessages, 8, recvMessage);

// new
osPoolId mpool_txMessages;
osPoolDef(mpool_txMessages, 8, fwMessage);

/*----------------------------------------------------------------------------
Thread 1 'controlUnit': Control Unit output
*---------------------------------------------------------------------------*/
void controlUnit (void const *argument) {
  osEvent evt;
  
  for (;;) {  
    
    evt = osMessageGet(mqid_rxMessages, osWaitForever);
    
    if(evt.status == osEventMessage)
    {
      RxMessage *message = (RxMessage*) evt.value.p;
      RxMessage *forward = (RxMessage*) malloc(sizeof(RxMessage));           
      
      if (message->msg[1] == 'E')
      {
        // Guarda andar em um int
        int andar = 10*(message->msg[2]-'0') + (message->msg[3]-'0');
        
        // TODO decide quem atende
        
        // elevador escolhido
        //REQUEST_FROM... 
        int valueLeft = evaluation(elevatorStatusLeft, andar, (message->msg[4]=='s'));
        int valueRight = evaluation(elevatorStatusRight, andar, (message->msg[4]=='s'));
        int valueCenter = evaluation(elevatorStatusCenter, andar, (message->msg[4]=='s'));
        

          REQUEST_FROM(elevatorStatusLeft, andar);


      }
      else
      {
        char elev = message->msg[0];
        
        // Copia caracteres p/ nova msg
        for (int i = 0; i < message->size && i < 8; i++)
          forward->msg[i] = message->msg[i+1];
        forward->size = message->size - 1;
        
        if(elev == 'l' || elev == 'e')
        {
          osMessagePut(mqid_msgBoxLeftElevator, (uint32_t) forward, osWaitForever/* era 0*/);
        }
        else if(elev == 'c')
        {
          osMessagePut(mqid_msgBoxCenterElevator, (uint32_t) forward, osWaitForever);
        }
        else if(elev == 'r' || elev == 'd')
        {
          osMessagePut(mqid_msgBoxRightElevator, (uint32_t) forward, osWaitForever);
        } 
      }
      
      osPoolFree(mpool_rxMessages, message);
      free(forward);
    }
  }
}

/*----------------------------------------------------------------------------
Thread 2 'leftElevator': Left Elevator output
*---------------------------------------------------------------------------*/
void leftElevator (void const *argument) {
  osEvent evt;
  uint8_t lightOn[] = {'e', 'L', ' '};
  uint8_t lightOff[] = {'e', 'D', ' '};
  
  for (;;) {   
    
    // osWaitForever -> 0, nao espera msg
    // precisa tratar os requests em algum momento
    evt = osMessageGet(mqid_msgBoxLeftElevator, 0);                
    
    if(evt.status == osEventMessage)
    {
      
      RxMessage *message = (RxMessage*) evt.value.p;
      
      // TRATA A MENSAGEM
      
      unsigned char key = message->msg[0];
      
      // Botao interno foi apertado
      if(key == 'I')
      {
         // Adiciona andar a lista de requestsf
        REQUEST_FROM(elevatorStatusLeft, message->msg[1]-'a');
        // Acende luz botao
        lightOn[2] = message->msg[1];
        write2USART(lightOn, 3);  
      }
      
      // Chegou em um andar
      else if(key >= '0' && key <= '9')
      {
        // registra novo andar atual
        message->msg[message->size] = '\0';
        int current_floor = atoi((char*)message->msg);
        CURRENT_FLOOR(elevatorStatusLeft, current_floor);
        
        // Checa se o novo andar esta nos requests
        if(CHECK(elevatorStatusLeft))
        {
          write2USART("ep", 2);
          elevatorStatusLeft.state = Stopped;
          
          ATTENDED_REQUEST(elevatorStatusLeft);
          
          lightOff[2] = current_floor+'a';
          write2USART(lightOff, 3);
          
          // Abrir e fechar a porta
          write2USART("ea", 2);
          osDelay(6000);
          write2USART("ef", 2);
          
          if(HAS_REQUESTS(elevatorStatusLeft))
          {
            // Teste para atendimento de requisicoes
            if(((elevatorStatusLeft.requests.floors % elevatorStatusLeft.currentPosition.floors) > 0) && (elevatorStatusLeft.state == GoingDown))
            {              
              write2USART("ed", 2);
              elevatorStatusLeft.state = GoingDown;
            }
            else if((elevatorStatusLeft.requests.floors > elevatorStatusLeft.currentPosition.floors) && (elevatorStatusLeft.state == GoingUp))
            {
              write2USART("es", 2);                            
              elevatorStatusLeft.state = GoingUp;
            }
            else
            {
              if(elevatorStatusLeft.requests.floors < elevatorStatusLeft.currentPosition.floors)
              {
                write2USART("ed", 2);
                elevatorStatusLeft.state = GoingDown;
              }else
              {
                write2USART("es", 2);                
                elevatorStatusLeft.state = GoingUp;
              }
            }
          } 
        }
      }            
    }
    
    // Caso ainda existam requests
    if(HAS_REQUESTS(elevatorStatusLeft) && elevatorStatusLeft.state == Stopped)
    {
      if(CHECK(elevatorStatusLeft))
      {
        int currentFloorNumber = 0;
	for (int i=0; i<16; i++) {
		if ((elevatorStatusLeft.currentPosition.floors >> i) == 1) break;
		else currentFloorNumber++;
	}
        lightOff[2] = currentFloorNumber+'a';
        write2USART(lightOff, 3);
        
        ATTENDED_REQUEST(elevatorStatusLeft);
        
        // Abrir e fechar a porta
        write2USART("ea", 2);
        osDelay(6000);
        write2USART("ef", 2);
        
      }
      else if (HAS_REQUEST_ABOVE(elevatorStatusLeft))
      {
        write2USART("es", 2);        
        elevatorStatusLeft.state = GoingUp;
        
      }
      else // Request a baixo
      {
        write2USART("ed", 2);
        elevatorStatusLeft.state = GoingDown;
      }
      
    }
    
  }
}

/*----------------------------------------------------------------------------
Thread 3 'rightElevator': Right Elevator output
*---------------------------------------------------------------------------*/
void rightElevator (void const *argument) {
  osEvent evt;
  uint8_t lightOn[] = {'d', 'L', ' '};
  uint8_t lightOff[] = {'d', 'D', ' '};
  
  for (;;) {   
    
    // osWaitForever -> 0, nao espera msg
    // precisa tratar os requests em algum momento
    evt = osMessageGet(mqid_msgBoxRightElevator, 0);                
    
    if(evt.status == osEventMessage)
    {
      
      RxMessage *message = (RxMessage*) evt.value.p;
      
      // TRATA A MENSAGEM
      
      unsigned char key = message->msg[0];
      
      // Botao interno foi apertado
      if(key == 'I')
      {
         // Adiciona andar a lista de requestsf
        REQUEST_FROM(elevatorStatusRight, message->msg[1]-'a');
        // Acende luz botao
        lightOn[2] = message->msg[1];
        write2USART(lightOn, 3);  
      }
      
      // Chegou em um andar
      else if(key >= '0' && key <= '9')
      {
        // registra novo andar atual
        message->msg[message->size] = '\0';
        int current_floor = atoi((char*)message->msg);
        CURRENT_FLOOR(elevatorStatusRight, current_floor);
        
        // Checa se o novo andar esta nos requests
        if(CHECK(elevatorStatusRight))
        {
          write2USART("dp", 2);
          
          ATTENDED_REQUEST(elevatorStatusRight);
          
          lightOff[2] = current_floor+'a';
          write2USART(lightOff, 3);
          
          // Abrir e fechar a porta
          write2USART("da", 2);
          osDelay(6000);
          write2USART("df", 2);
          
          if(HAS_REQUESTS(elevatorStatusRight))
          {
            // Teste para atendimento de requisicoes
            if(((elevatorStatusRight.requests.floors % elevatorStatusRight.currentPosition.floors) > 0) && (elevatorStatusRight.state == GoingDown))
            {              
              write2USART("dd", 2);
              elevatorStatusRight.state = GoingDown;
            }
            else if((elevatorStatusRight.requests.floors > elevatorStatusRight.currentPosition.floors) && (elevatorStatusRight.state == GoingUp))
            {
              write2USART("ds", 2);                            
              elevatorStatusRight.state = GoingUp;
            }
            else
            {
              if(elevatorStatusRight.requests.floors < elevatorStatusRight.currentPosition.floors)
              {
                write2USART("dd", 2);
                elevatorStatusRight.state = GoingDown;
              }else
              {
                write2USART("ds", 2);                
                elevatorStatusRight.state = GoingUp;
              }
            }
          } else{
            elevatorStatusRight.state = Stopped;
          }
        }
      }            
    }
    
    // Caso ainda existam requests
    if(HAS_REQUESTS(elevatorStatusRight) && elevatorStatusRight.state == Stopped)
    {
      if(CHECK(elevatorStatusRight))
      {
        int currentFloorNumber = 0;
	for (int i=0; i<16; i++) {
		if ((elevatorStatusRight.currentPosition.floors >> i) == 1) break;
		else currentFloorNumber++;
	}
        lightOff[2] = currentFloorNumber+'a';
        write2USART(lightOff, 3);
        
        ATTENDED_REQUEST(elevatorStatusRight);
        
        // Abrir e fechar a porta
        write2USART("da", 2);
        osDelay(6000);
        write2USART("df", 2);
        
      }
      else if (HAS_REQUEST_ABOVE(elevatorStatusRight))
      {
        write2USART("ds", 2);        
        elevatorStatusRight.state = GoingUp;
        
      }
      else // Request a baixo
      {
        write2USART("dd", 2);
        elevatorStatusRight.state = GoingDown;
      }
      
    }
    
  }
}


/*----------------------------------------------------------------------------
Thread 4 'centerElevator': Right Elevator output
*---------------------------------------------------------------------------*/

void centerElevator (void const *argument) {
  osEvent evt;
  uint8_t lightOn[] = {'c', 'L', ' '};
  uint8_t lightOff[] = {'c', 'D', ' '};
  
  for (;;) {   
    
    // osWaitForever -> 0, nao espera msg
    // precisa tratar os requests em algum momento
    evt = osMessageGet(mqid_msgBoxCenterElevator, 0);                
    
    if(evt.status == osEventMessage)
    {
      
      RxMessage *message = (RxMessage*) evt.value.p;
      
      // TRATA A MENSAGEM
      
      unsigned char key = message->msg[0];
      
      // Botao interno foi apertado
      if(key == 'I')
      {
         // Adiciona andar a lista de requestsf
        REQUEST_FROM(elevatorStatusCenter, message->msg[1]-'a');
        // Acende luz botao
        lightOn[2] = message->msg[1];
        write2USART(lightOn, 3);  
      }
      
      // Chegou em um andar
      else if(key >= '0' && key <= '9')
      {
        // registra novo andar atual
        message->msg[message->size] = '\0';
        int current_floor = atoi((char*)message->msg);
        CURRENT_FLOOR(elevatorStatusCenter, current_floor);
        
        // Checa se o novo andar esta nos requests
        if(CHECK(elevatorStatusCenter))
        {

          write2USART("cp", 2);
          
          ATTENDED_REQUEST(elevatorStatusCenter);
          
          lightOff[2] = current_floor+'a';
          write2USART(lightOff, 3);
          
          // Abrir e fechar a porta
          write2USART("ca", 2);
          osDelay(6000);
          write2USART("cf", 2);
          
          if(HAS_REQUESTS(elevatorStatusCenter))
          {
            // Teste para atendimento de requisicoes
            if(((elevatorStatusCenter.requests.floors % elevatorStatusCenter.currentPosition.floors) > 0) && (elevatorStatusCenter.state == GoingDown))
            {              
              write2USART("cd", 2);
              elevatorStatusCenter.state = GoingDown;
            }
            else if((elevatorStatusCenter.requests.floors > elevatorStatusCenter.currentPosition.floors) && (elevatorStatusCenter.state == GoingUp))
            {
              write2USART("cs", 2);                            
              elevatorStatusCenter.state = GoingUp;
            }
            else
            {
              if(elevatorStatusCenter.requests.floors < elevatorStatusCenter.currentPosition.floors)
              {
                write2USART("cd", 2);
                elevatorStatusCenter.state = GoingDown;
              }else
              {
                write2USART("cs", 2);                
                elevatorStatusCenter.state = GoingUp;
              }
            }
          } else{
            elevatorStatusCenter.state = Stopped;
          }
        }
      }            
    }
    
    // Caso ainda existam requests
    if(HAS_REQUESTS(elevatorStatusCenter) && elevatorStatusCenter.state == Stopped)
    {
      if(CHECK(elevatorStatusCenter))
      {
        int currentFloorNumber = 0;
	for (int i=0; i<16; i++) {
		if ((elevatorStatusCenter.currentPosition.floors >> i) == 1) break;
		else currentFloorNumber++;
	}
        lightOff[2] = currentFloorNumber+'a';
        write2USART(lightOff, 3);
        
        ATTENDED_REQUEST(elevatorStatusCenter);
        
        // Abrir e fechar a porta
        write2USART("ca", 2);
        osDelay(6000);
        write2USART("cf", 2);
        
      }
      else if (HAS_REQUEST_ABOVE(elevatorStatusCenter))
      {
        write2USART("cs", 2);        
        elevatorStatusCenter.state = GoingUp;
        
      }
      else // Request a baixo
      {
        write2USART("cd", 2);
        elevatorStatusCenter.state = GoingDown;
      }
      
    }
    
  }
}

/*----------------------------------------------------------------------------
Thread 5 'outputSerial': Thread para comunicacao com simulador
*---------------------------------------------------------------------------*/
void outputSerial(void const *argument) {
  osEvent evt;
  
  for (;;) {        
    evt = osMessageGet(mqid_msgOutputSerial, osWaitForever);  
    
    if(evt.status == osEventMessage)
    {
      RxMessage *message = (RxMessage*) evt.value.p;
      
      for(int i = 0; i < message->size; i++)
        sendData(USART1, message->msg[i]);
      sendData(USART1, 0xd);
    }        
  }
}

osThreadDef(controlUnit, osPriorityNormal, 1, 0);
osThreadDef(leftElevator, osPriorityNormal, 1, 0);
osThreadDef(centerElevator, osPriorityNormal, 1, 0);
osThreadDef(rightElevator, osPriorityNormal, 1, 0);
// new
osThreadDef(outputSerial, osPriorityNormal, 1, 0);

int main()
{  
  osKernelInitialize();
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  initUSART(115200);
  
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
  
  mpool_rxMessages = osPoolCreate(osPool(mpool_rxMessages));
  
  mqid_rxMessages = osMessageCreate(osMessageQ(mqid_rxMessages), NULL);    
  mqid_msgBoxLeftElevator = osMessageCreate(osMessageQ(mqid_msgBoxLeftElevator), NULL);
  mqid_msgBoxCenterElevator = osMessageCreate(osMessageQ(mqid_msgBoxCenterElevator), NULL);
  mqid_msgBoxRightElevator = osMessageCreate(osMessageQ(mqid_msgBoxRightElevator), NULL);
  
  // new
  mqid_msgOutputSerial = osMessageCreate(osMessageQ(mqid_msgOutputSerial), NULL);
  
  tid_controlUnit = osThreadCreate(osThread(controlUnit), NULL);
  tid_leftElevator = osThreadCreate(osThread(leftElevator), NULL);
  tid_centerElevator = osThreadCreate(osThread(centerElevator), NULL);
  tid_rightElevator = osThreadCreate(osThread(rightElevator), NULL);
  
  // new
  tid_outputSerial = osThreadCreate(osThread(outputSerial), NULL);
  
  elevatorStatusLeft.currentPosition.floors = 0;
  elevatorStatusLeft.requests.floors = 0;
  elevatorStatusLeft.state = Stopped;
  
  sendData(USART1, 'e');
  sendData(USART1, 'r');
  sendData(USART1, 0xD);
  sendData(USART1, 'e');
  sendData(USART1, 'f');
  sendData(USART1, 0xD);
  
  sendData(USART1, 'c');
  sendData(USART1, 'r');
  sendData(USART1, 0xD);
  sendData(USART1, 'c');
  sendData(USART1, 'f');
  sendData(USART1, 0xD);
  
  sendData(USART1, 'd');
  sendData(USART1, 'r');
  sendData(USART1, 0xD);
  sendData(USART1, 'd');
  sendData(USART1, 'f');
  sendData(USART1, 0xD);
  
  osKernelStart();
  
  return 0;
}

void write2USART(uint8_t* msg, uint8_t size)
{
  RxMessage *forward = (RxMessage*) malloc(sizeof(RxMessage));   
  
  for (int i = 0; i < size && i < 8; i++)
    forward->msg[i] = msg[i];
  forward->size = size;
  
  osMessagePut(mqid_msgOutputSerial, (uint32_t)forward, osWaitForever);
    
}

// gives points to how good is to this elavator to take the task
int evaluation(ElevatorStatus elevator, int requestedFloor, int buttonUp){
	// calculate the number of the current floor
	int currentFloorNumber = 0;
	for (int i=0; i<16; i++) {
		if ((elevator.currentPosition.floors >> i) == 1) break;
		else currentFloorNumber++;
	}

	// calculate points based on the distance between current and requested floor 
	int distancePoints = requestedFloor - currentFloorNumber;

	// gives or take a point based on the direction and distance to the requested floor
	int directionPoints = -1;
	if ((elevator.state == GoingUp && distancePoints > 0) || (elevator.state == GoingDown && distancePoints < 0))
		directionPoints = 1;

	// gives a point if the floor needs to go up to the floor the elevator is already going
	int buttonDirectionPoints = 0;
	if (elevator.requests.floors > (1 << requestedFloor) && buttonUp)
		buttonDirectionPoints = 1;

	// calculate the abs of the distance
	distancePoints = distancePoints > 0 ? distancePoints : -distancePoints;

	// weighted evalutaion base on the points above
	return directionPoints * 1000 + distancePoints * 10 + buttonDirectionPoints * 20;
}