En estos 3 ejemplos vamos a ver como configurar la DMA para 3 tareas sencillas:

1. ADC->DMA: El ADC3 Ch12 recoge datos a una frecuencia determinada (ver sección ADC) El DMA es el encargado de coger los datos del ADC y guardarlos en una variable automáticamente.

2. UART_Rx ¿> DMA: Recibimos datos por UART_4 y el DMA los guarda en un Buffer de 32 bytes. Activa interrupciones de llenado de buffer completo o buffer semi-completo, para procesar mensajes u órdenes por ejemplo.

3. DMA -> DAC Tenemos unas señales (senoidal y escalera) guardadas en tablas, vectores o cualquier tipo de variable y utilizamos un temporizador, TIM6, como trigger, el cual su frecuencia de oscilación marcara las frecuencias de las señales almacenadas en memoria.
