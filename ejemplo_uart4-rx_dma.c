

/*************************************************************************************************
 *  Ejemplo DMA-UART-Rx [PERIFERICO->MEMORIA]
 *  
 *   - Configura DMA para los UART_Rx
 *   - Configura interrupciones para Medio Buffer 16 bytes o 32 bytes buffer completo
 *   
 *   - Configuracion util para procesar e interpretar tramas largas de comunicacion
 *
 *************************************************************************************************
*/

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_usart.h"
 
__IO uint8_t Buffer[32]; //Variable donde la DMA almacenara los datos del UART_Rx automaticamente
 
/**************************************************************************************/
//Codigo Configuracion UART4, DMA-UART4_Rx, e interrupciones DMA Buffer Lleno y Medio
/**************************************************************************************/
 
void UART4_RX_DMA(void)
{
  /* Creamos estructuras de datos para la configuracion de los perifericos -----------------*/
  GPIO_InitTypeDef    GPIO_InitStructure;
  USART_InitTypeDef   USART_InitStructure;
  NVIC_InitTypeDef    NVIC_InitStructure;
  DMA_InitTypeDef     DMA_InitStructure;// Inicializar la estructura del DMA

  /* Activa los relojes de los perifericos: ------------------------------------------------*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);/* Activar reloj UART */  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);/* Activar reloj GPIOA */ 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); /* Activar reloj DMA1 */

  /* Configuracion NVIC Prioridades Interrupciones-----------------------------------------*/
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
 
  /* Activa la interrupcion UART4 RX DMA */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Configuracion GPIO--------------------------------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  /* Conectar los pins de USART con Alternative Functions */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4);

  /*Configuracion UART---------------------------------------------------------------------*/
  USART_InitStructure.USART_BaudRate = 4800;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
 
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(UART4, &USART_InitStructure);
  USART_Cmd(UART4, ENABLE);

  /* Configuracion DMA UART_Rx--------------------------------------------------------------*/
  DMA_DeInit(DMA1_Stream2);//Buscamos en la tabla 20 y 21 UART1_RX y localizamos el canal y el stream
 
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;                          // Canal 4 -> Consultar tabla 20 y 21
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                 // Del periferico a la memoria
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Buffer;               // Asociamos la variable donde vamos a guardar los datos recibidos del UART
  DMA_InitStructure.DMA_BufferSize = (uint16_t)sizeof(Buffer);            // Reservar en memoria el tamaño del buffer 32bytes
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;        // Especifica la direccion base periferica de DMA y Streamx.
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // Especifica si el registro de direccion periferica no debe ser incrementado o no
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // Especifica si el registro de direcciones de memoria debe ser incrementado o no.
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // Especifica el tamaño de datos del perifericos [8 bits]
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // Especifica el tamaaño de datos de la memoria. [8 bits]
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         // Especifica el modo de funcionamiento de la DMAy Streamx
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;                     // Especifica la prioridad de software para el DMAy Streamx
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;                   // Especifica si el modo FIFO o el modo directo se utilizaran para la secuencia especificada
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;           // Especifica el nivel del umbral FIFO.
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;             // Se especifica la cantidad de datos a transferir en una sola transaccion no interrumpible
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;     // Especifica la configuracion de transferencia de rafaga para los traslados perifericos
 
  DMA_Init(DMA1_Stream2, &DMA_InitStructure);
 
  /* Activa la solicitud DMA desde USART-----------------------------------------------*/
  USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
 
  /* Activa las interrupciones de buufer completo y medio buffer-----------------------*/
  DMA_ITConfig(DMA1_Stream2, DMA_IT_TC, ENABLE);
  DMA_ITConfig(DMA1_Stream2, DMA_IT_HT, ENABLE);
 
  /* Activa DMA RX Stream -------------------------------------------------------------*/
  DMA_Cmd(DMA1_Stream2, ENABLE);
}
 
/**************************************************************************************/
//Codigo de la interrupcion
/**************************************************************************************/
 
void DMA1_Stream2_IRQHandler(void)
{
  /* Comprobacion de buffer completo */
  if (DMA_GetITStatus(DMA1_Stream2, DMA_IT_TCIF2))
  {
    /* Borrar la interrupcion del DMA*/
    DMA_ClearITPendingBit(DMA1_Stream2, DMA_IT_TCIF2);
    //Manda un caracter
    USART_SendData(UART4, 'C');
    //AQUI PROCESARIAMOS EL MENSAJE

  }
 
  /* Comprobacion de buffer completo */
  if (DMA_GetITStatus(DMA1_Stream2, DMA_IT_HTIF2))
  {
    /* Borrar la interrupcion del DMA*/
    DMA_ClearITPendingBit(DMA1_Stream2, DMA_IT_HTIF2);
    //Manda un caracter
    USART_SendData(UART4, 'H');
    //AQUI PROCESARIAMOS EL MENSAJE
  }
}
 

/**************************************************************************************/
//Codigo del main
/**************************************************************************************/
 
int main(void)
{
  UART4_RX_DMA(); //Activa UART4 con DMA, a partir de aqui si llegan datos, estaran en -> Buffer[]
 
 
  // Mientras el flag de la transmision no este vacio__(inicializacion)
  while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  // Enviamos un caracter de prueba
  USART_SendData(UART4, '*');
 

  // La interrupcion se encarga de llenar el buffer y cuando este lleno o a mitad enviara caracteres de informacion
  // Podemos utilizar en cualquier momento el buffer desde la variable Buffer[32]
  while(1); 
}



