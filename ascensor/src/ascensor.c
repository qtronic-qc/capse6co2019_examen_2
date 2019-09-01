/*=============================================================================
 * Author: Carlos Quintana <carloseliasq@gmail.com>
 * Date: 2019/08/31
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "ascensor.h"
#include "sapi.h"
#include "driverLCD.h"

/*=====[Definition macros of private constants]==============================*/
#define UART_SELECTED   UART_USB

CONSOLE_PRINT_ENABLE
/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

//void actualizar_LCD(char letra_1, int valor_piso);
// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.

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

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE ) {

      for(i=-5; i<=20; i++) {
         piso = i;
         actualizar_LCD('s', piso);
         delay(800);
      }
      for(i=20; i>=-5; i--) {
         piso = i;
         actualizar_LCD('b', piso);
         delay(800);
      }
   }

// NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
// directamente sobre un microcontroladore y no es llamado por ningun
// Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}
