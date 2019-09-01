#include "sapi.h"
#include "driverLCD.h"

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