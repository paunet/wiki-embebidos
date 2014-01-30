  
#define  RTC_STATUS_REG      RTC_BKP_DR0  // Status Register
#define  RTC_STATUS_TIME_OK  0xA3C5       // RTC ya configurado




int main(void)
{
  uint32_t status;

  //Activamos el reloj RTC
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  //Activamos permisos de escritura de la configuracion
  PWR_BackupAccessCmd(ENABLE);

  // Leemos los registros actuales
  status=RTC_ReadBackupRegister(RTC_STATUS_REG);
  if(status==RTC_STATUS_TIME_OK) //si el reloj esta inicializado (despues de un reset por ejemplo)
  {
    //Espera a sincronizar relojes
    RTC_WaitForSynchro();
    //Activa el reloj LSI
    RCC_LSICmd(ENABLE);

	  // Espera hasta que el reloj este preparado
	  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
		
    // Configuramos el RTC con el reloj 
	  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

  	// Activa RTC
  	RCC_RTCCLKCmd(ENABLE);

  	// Espera a sincronizar relojes
  	RTC_WaitForSynchro();

    // Actualiza el tiempo actual
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

  
  else // El RTC no estaba inicializado
  {
    //Activamos reloj LSI
    RCC_LSICmd(ENABLE);

    // Esperamos a que el reloj LSI este preparado
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

    // Asignamos el reloj LSI al RTC
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    // Activamos RTC
    RCC_RTCCLKCmd(ENABLE);

    // Esperamos a que el reloj este sincronizado
    RTC_WaitForSynchro();

    //Establecemos fecha y hora 31/1/14 19:50:00
    RTC_TimeStructure.RTC_Hours = 19;
    RTC_TimeStructure.RTC_Minutes = 50;
    RTC_TimeStructure.RTC_Seconds = 00;

    RTC_DateStructure.RTC_Date = 31;
    RTC_DateStructure.RTC_Month = 1;
    RTC_DateStructure.RTC_Year = 14;
    RTC_DateStructure.RTC_WeekDay = 4; //jueves

    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
    RTC_InitStructure.RTC_SynchPrediv =  0xFF;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    RTC_Init(&RTC_InitStructure);  

    //Cambiamos el registro a establecido
    RTC_WriteBackupRegister(RTC_STATUS_REG, RTC_STATUS_TIME_OK); 
     


  }
  while(1){

    //En cualquier parte del programa podremos comparar elementos de RTC_TimeStructure o DateStructure para poder utilizar la fecha y la hora.
  
  }
  
 
}
