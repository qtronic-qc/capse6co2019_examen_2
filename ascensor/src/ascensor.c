/*=============================================================================
 * Author: Carlos Quintana <carloseliasq@gmail.com>
 * Date: 2019/08/31
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "sapi.h"
#include "ascensor.h"
//#include <driverLCD.h>
//#include "driverTeclado.h"

/*=====[Definition macros of private constants]==============================*/
#define UART_SELECTED   UART_USB

#define TECLADO_SCAN 40
#define TECLADO_MOSTRAR 200

CONSOLE_PRINT_ENABLE
/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

//void actualizar_LCD(char letra_1, int valor_piso);
// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.

uint8_t TecladoPinesFilas[4] = {
   RS232_TXD,
   CAN_RD,
   CAN_TD,
   T_COL1,
};

uint8_t TecladoPinesColumnas[4] = {
   T_FIL0,
   T_FIL3,
   T_FIL2,
   T_COL0,
};

void tecladoInit( void );
int8_t escanearTeclado( void );

int8_t conversor[]= {
   1,2,3,'+',
   4,5,6,'-',
   7,8,9,'*',
   15,0,'#','/',-1
};

void actualizar_LCD(char letra_1, int valor_piso);

uint8_t TecladoP = 0;
int apretada = 0;
int tecla = -1;

int main( void )
{
   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma
   boardConfig();

   // Inicializar UART_USB como salida de consola
   consolePrintConfigUart( UART_USB, 115200 );

   // Inicializar LCD de 16x2 (caracteres x lineas) con cada caracter de 5x2 pixeles
   lcdInit( 16, 2, 5, 8 );

   lcdCursorSet( OFF ); // Apaga el cursor
   lcdClear();          // Borrar la pantalla
   int piso = 0, i = 0;

   tecladoInit();
   delay_t escaneoTeclado;
   delayInit( &escaneoTeclado, TECLADO_SCAN );

   delay_t mostrarTeclado;
   delayInit( &mostrarTeclado, TECLADO_MOSTRAR );

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE ) {

      if( delayRead(&escaneoTeclado) ) {
         tecla = conversor[escanearTeclado()]; // tecla = conversor[16] = -1
         piso = tecla;
         actualizar_LCD('s', piso);
      }


      /*for(i=-5; i<=20; i++) {
         piso = i;
         actualizar_LCD('s', piso);
         delay(800);
      }
      for(i=20; i>=-5; i--) {
         piso = i;
         actualizar_LCD('b', piso);
         delay(800);
      }*/
   }

// NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
// directamente sobre un microcontroladore y no es llamado por ningun
// Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

void tecladoInit( void )
{
   uint8_t pin = 0;
   for( pin = 0; pin < 4; pin++ ) {
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

   for( fil = 0; fil < 4; fil++ ) {
      gpioWrite( TecladoPinesFilas[fil], ON ); // se pone a 1 la fila
   }

   for( fil = 0; fil < 4; fil++ ) {
      gpioWrite( TecladoPinesFilas[fil], OFF ); // se pone a 0 la fila
      for( col = 0; col < 4; col++ ) {
         if(gpioRead(TecladoPinesColumnas[col]) == 0) { // chequeo si alguna columna vale 0
            indice = fil*4 + col;
            delay(50);
            return indice;
         }
      }
   }
   apretada = 0;
   return 16;
}
void actualizar_LCD(char letra_1, int valor_piso)
{
   char mensaje[] = "    ";
   char num_a_texto[] = {'5','4','3','2','1','0','1','2','3','4','5','6','7','8','9'};

   static int decena = 0, unidad = 0;

   mensaje[0] = letra_1;
   mensaje[1] = ' ';

   if(valor_piso == 0) {//piso = 0 sería planta baja
      mensaje[2] = 'p';
      mensaje[3] = 'b';
   } else {// diferente de cero implica que está en algún piso o subsuelo
      if(valor_piso < 0) {       //piso menor a 0 implica que está en subsuelo
         decena = 0;
         mensaje[2] = '-';
      } else {                   //piso mayor a 0 implica que está en algún piso
         if(valor_piso < 10) {
            decena = 0;
            mensaje[2] = ' ';
         } else {
            decena = valor_piso/10;
            mensaje[2] = num_a_texto[decena + 5];
         }
      }

      unidad = valor_piso - decena*10;
      mensaje[3] = num_a_texto[unidad + 5];
   }

   lcdGoToXY( 0, 0 );
   lcdSendStringRaw( mensaje );
}
