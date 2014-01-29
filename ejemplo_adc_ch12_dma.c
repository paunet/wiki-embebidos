
/*************************************************************************************************
 *  Ejemplo DMA-ADC [PERIFERICO->MEMORIA]
 *  
 *    -Configura el ADC para que el DMA guarde el valor automaticamente,
 *      en una variable.
 *
 *************************************************************************************************
*/

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_adc.h"

//Direccion de memoria del ADC3 --> Donde la DMA consultara los valores para almacenarlos donde queramos
#define ADC3_DR_ADDRESS     ((uint32_t)0x4001224C)

//Variable donde la DMA almacenara los datos del ADC3 automaticamente
__IO uint16_t ADC3ConvertedValue = 0;

/**************************************************************************************/
//Codigo Configuracion ADC3_Canal 12 con DMA
/**************************************************************************************/
 
void ADC3_DMA_CH12_Config(void)
{
  /* Creamos estructuras de datos para la configuracion de los perifericos -------------------------------------------*/
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;

  /* Activa los relojes de los perifericos:  -------------------------------------------------------------------------*/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);        // GPIO para el Pin: PC02 del ADC y Canal 2 de DMA
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);                                // ADC3

  /* DMA2 Stream0 channel0 configuration ------------------------------------------------------------------------------*/

  DMA_InitStructure.DMA_Channel = DMA_Channel_2;                              //Canal 2 -> Consultar tabla 20 y 21
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC3_DR_ADDRESS;       // Del periferico a la memoria
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC3ConvertedValue;      // Asociamos la variable donde vamos a guardar los datos recibidos del ADC3
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                     // Del periferico a la memoria
  DMA_InitStructure.DMA_BufferSize = 1;                                       // Reservar en memoria el tamaño del buffer 1 uint32_t
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // Especifica que el registro de direccion periferica no debe ser incrementado
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;                    // Especifica que el registro de direcciones de memoria no debe ser incrementado
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // Especifica el tamaño de datos del perifericos [16 bits]
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         // Especifica el tamaño de datos de la memoria. [ bits]
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             // Especifica el modo de funcionamiento de la DMAy Streamx
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;                         // Especifica la prioridad de software para el DMAy Streamx
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                      // Especifica que el modo directo se utilizara para la secuencia especificada
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;           // Especifica el nivel del umbral FIFO.
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;                 // Se especifica la cantidad de datos a transferir en una sola transaccion no interrumpible
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;         // Especifica la configuracion de transferencia de rafaga para los traslados perifericos
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);

  /* Configuramos el Canal 12 del ADC3 como entrada analogica ---------------------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* ADC Parametros Comunes --------------------------------------------------------------------------------------------*/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* Configuracion especifica del ADC3 ---------------------------------------------------------------------------------*/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; //12 bits [0 - 4095]Valores
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;         //Conversion continua o automatica -- sin trigger
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //Alineacion LSB
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC3, &ADC_InitStructure);

  /* Configurar el canal 12 del ADC3 -------------------------------------------------------------------------------------*/
  ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);

  /* Activa DMA request after last transfer (Single-ADC mode) ------------------------------------------------------------*/
  ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

  /* Activa ADC3 DMA -----------------------------------------------------------------------------------------------------*/
  ADC_DMACmd(ADC3, ENABLE);

  /* Activa ADC3 ---------------------------------------------------------------------------------------------------------*/
  ADC_Cmd(ADC3, ENABLE);

  /* Start ADC3 Software Conversion---------------------------------------------------------------------------------------*/
  ADC_SoftwareStartConv(ADC3);
}


/**************************************************************************************/
//Codigo del main
/**************************************************************************************/


int main(void)
{
  float Voltaje;

  ADC3_DMA_CH12_Config(void);

  while (1)
  {
      //Podemos utilizar la variable ADC3ConvertedValue en cualquier parte del programa asi como en funciones.      
      Voltaje = ADC3ConvertedValue *3300/0xFFF; 
  }
}
