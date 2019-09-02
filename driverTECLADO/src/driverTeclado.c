#include "sapi.h"
#include "driverTeclado.h"

//int apretada = 0;

/*void tecladoInit( void )
{
    uint8_t pin = 0;
    for( pin=0; pin<4; pin++ ) {
        // Se configuran las filas como salidas
        gpioInit( TecladoPinesFilas[pin], GPIO_OUTPUT );
        gpioWrite( TecladoPinesFilas[pin], OFF );
        // Se configuran las columnas como entradas, con resistencias de pull-up internas
        gpioInit( TecladoPinesColumnas[pin], GPIO_INPUT_PULLUP );
    }
}

int8_t escanearTeclado( void )
{
   //static int apretada = 0; 
   int8_t indice = 16;
    int8_t fil = 0;
    int8_t col = 0;

    for( fil=0; fil<4; fil++ ) {
        gpioWrite( TecladoPinesFilas[fil], ON ); // se pone a 1 la fila
    }

    for( fil=0; fil<4; fil++ ) {
        gpioWrite( TecladoPinesFilas[fil], OFF ); // se pone a 0 la fila
        for( col=0; col<4; col++ ) {
            if(gpioRead(TecladoPinesColumnas[col])==0) { // chequeo si alguna columna vale 0
                indice = fil*4 + col;
                delay(50);
                return indice;
            }
        }
    }
    apretada = 0;
    return 16;
}*/