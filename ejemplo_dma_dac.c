

/*************************************************************************************************
 *  Ejemplo DMA-DAC  [MEMORIA->PERIFERICO]
 *  
 *   - Configura DMA para el DAC
 *   - Configura TIM6 como fuente del trigger
 *   - Dos tipos de salida, canal 1 del DAC, onda senoida, canal 2 del DAC tipo escalera
 *   - Ondas guardadas en vectores
 *
 *************************************************************************************************
*/

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_dac.h"

/**Direcciones de memoria donde el DMA recogera los datos para enviarlos al DAC**/
#define DAC_DHR12R2_ADDRESS    0x40007414 //12bits -> señal senoidal
#define DAC_DHR8R1_ADDRESS     0x40007410 //8 bits -> señal con forma de escalera

DAC_InitTypeDef  DAC_InitStructure;   //Declaramos la estructura global porque la vamos a llamar en dos funciones

/**Señal senoidal de 12 bits [0-4095] almacenada en 32 escalones**/
const uint16_t Sine12bit[32] = {
                      2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
                      3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909, 
                      599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647}; 

/**Señal con forma de escalon de 8 bits [0-255] en 6 escalones**/
const uint8_t Escalator8bit[6] = {0x0, 0x33, 0x66, 0x99, 0xCC, 0xFF};


//Definicion de funciones
void TIM6_Config(void);
void DAC_Ch1_EscalatorConfig(void);
void DAC_Ch2_SineWaveConfig(void);

/**************************************************************************************/
//Codigo del main
/**************************************************************************************/

int main(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
/* --------------------------- Configuracion de los Systems Clocks-----------------*/
  /* DMA1 clock and GPIOA clock enable (to be used with DAC) */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_GPIOA, ENABLE);
  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  /* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* TIM6 Configuration ------------------------------------------------------*/
  TIM6_Config();  
  
  /* Escalator Wave generator ------------------------------------------------*/
  DAC_Ch1_EscalatorConfig();

  /* Sine Wave generator -----------------------------------------------------*/
  DAC_Ch2_SineWaveConfig();
  while (1)
  {   
     
    
  }
}

/**************************************************************************************/
// Codigo de configuracion del temporizador 6, la fuente de disparo del DAC
// TIM6 configuration is based on CPU @168MHz and APB1 @42MHz
// (42 Mhz / Period) = Frecuencia del temporizador 6, 
//  
// 6  * Ftim6 = F señal escalera
// 32 * Ftim6 = F señal senoidal
/**************************************************************************************/

void TIM6_Config(void)
{
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  /* TIM6 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = 0xFF;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

  /* TIM6 TRGO selection */
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
  
  /* TIM6 enable counter */
  TIM_Cmd(TIM6, ENABLE);
}

/**************************************************************************************/
//Codigo Señal senoidal DMA -> DAC
/**************************************************************************************/

void DAC_Ch2_SineWaveConfig(void)
{
  DMA_InitTypeDef DMA_InitStructure;
  
  /* DAC channel2 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);

  /* DMA1_Stream5 channel7 configuration **************************************/
  DMA_DeInit(DMA1_Stream5);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;                              // Canal 4 -> Consultar tabla 20 y 21
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12R2_ADDRESS;   // Especifica la direccion base periferica de DMA y Streamx.
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&Sine12bit;               // Asociamos la variable donde vamos a guardar los datos que enviamos al DAC
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                     // De la memoria al periferico
  DMA_InitStructure.DMA_BufferSize = 32;                                      // Reservar en memoria el tamaño del buffer 32*uint32_T
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // Especifica si el registro de direccion periferica no debe ser incrementado o no
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // Especifica si el registro de direcciones de memoria debe ser incrementado o no.
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // Especifica el tamaño de datos del perifericos [16 bits]
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         // Especifica el tamaño de datos de la memoria. [ bits]
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             // Especifica el modo de funcionamiento de la DMAy Streamx
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;                         // Especifica la prioridad de software para el DMAy Streamx
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                      // Especifica que el modo directo se utilizara para la secuencia especificada
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;           // Especifica el nivel del umbral FIFO.
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;                 // Se especifica la cantidad de datos a transferir en una sola transaccion no interrumpible
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;         // Especifica la configuracion de transferencia de rafaga para los traslados perifericos
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);

  /* Activa DMA1_Stream5 */
  DMA_Cmd(DMA1_Stream5, ENABLE);

  /* Activa DAC Channel2 */
  DAC_Cmd(DAC_Channel_2, ENABLE);

  /* Activa DMA for DAC Channel2 */
  DAC_DMACmd(DAC_Channel_2, ENABLE);
}

/**************************************************************************************/
//Codigo Señal escalera DMA -> DAC
/**************************************************************************************/

void DAC_Ch1_EscalatorConfig(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  /* DAC Configuracion Canal 1 */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

  /* DMA1_Stream6 Configuracion del canal 7 de DMA **************************************/  
  DMA_DeInit(DMA1_Stream6);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;                          // Canal 4 -> Consultar tabla 20 y 21
  DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR8R1_ADDRESS;          // Especifica la direccion base periferica de DMA y Streamx.
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&Escalator8bit;       // Asociamos la variable donde vamos a guardar los datos que enviamos al DAC
  DMA_InitStructure.DMA_BufferSize = 6;                                   // Reservar en memoria el tamaño del buffer 6*byte
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // Especifica el tamaño de datos del perifericos [8 bits]
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // Especifica el tamaaño de datos de la memoria. [8 bits]
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                 // De la memoria al periferico 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // Especifica si el registro de direccion periferica no debe ser incrementado o no
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // Especifica si el registro de direcciones de memoria debe ser incrementado o no.
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         // Especifica el modo de funcionamiento de la DMAy Streamx
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;                     // Especifica la prioridad de software para el DMAy Streamx
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                  // Especifica si el modo FIFO o el modo directo se utilizaran para la secuencia especificada
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull        // Especifica el nivel del umbral FIFO.
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;             // Se especifica la cantidad de datos a transferir en una sola transaccion no interrumpible
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;     // Especifica la configuracion de transferencia de rafaga para los traslados perifericos
 
  DMA_Init(DMA1_Stream6, &DMA_InitStructure);    

  /* Activa DMA1_Stream6 */
  DMA_Cmd(DMA1_Stream6, ENABLE);
  
  /* Activa DAC Canal 1 */
  DAC_Cmd(DAC_Channel_1, ENABLE);

  /* Activa DMA para el Canal 1 del DAC*/
  DAC_DMACmd(DAC_Channel_1, ENABLE);
}

