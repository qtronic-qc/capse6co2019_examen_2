/*=============================================================================
 * Author: Carlos Quintana <carloseliasq@gmail.com>
 * Date: 2019/08/31
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "sapi.h"
#include "ascensor.h"
#include "stdio.h"
//#include <driverLCD.h>
//#include "driverTeclado.h"

/*=====[Definition macros of private constants]==============================*/
#define UART_SELECTED   UART_USB

#define COMMON_CATHODE
//#define COMMON_ANODE

#ifdef COMMON_CATHODE
#define DIGIT_ON       1
#define DIGIT_OFF      0
#define PIN_VALUE_ON   1
#define PIN_VALUE_OFF  0
#endif

#ifdef COMMON_ANODE
#define DIGIT_ON       1
#define DIGIT_OFF      0
#define PIN_VALUE_ON   0
#define PIN_VALUE_OFF  1
#endif

#define TIEMPO_DISP 100
#define UN_SEGUNDO 1000
#define T_ESPERA 5
#define LED_ascensor_moviendo LEDB
#define LED_ascensor_detenido LED3
#define LED_puerta_abierta LEDG
#define LED_abriendo_puerta LED1
#define LED_cerrando_puerta LED2
#define LED_alarma_puerta LEDR
#define SI 1
#define NO 0
#define APAGADO 25
#define SIGNO_MENOS 23
#define P 22
#define b 11
#define s 5
#define ES_SUBSUELO -6

CONSOLE_PRINT_ENABLE
/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

//void actualizar_LCD(char letra_1, int valor_piso);
// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.

//** MEF teclado

const uint8_t display7SegmentOut[26] = {
   0b00111111, // 0
   0b00000110, // 1
   0b01011011, // 2
   0b01001111, // 3
   0b01100110, // 4
   0b01101101, // 5
   0b01111101, // 6
   0b00000111, // 7
   0b01111111, // 8
   0b01101111, // 9

   0b01011111, // a
   0b01111100, // b
   0b01011000, // c
   0b01011110, // d
   0b01111011, // e
   0b01110001, // f

   0b01110111, // A
   0b00111001, // C
   0b01111001, // E
   0b01110110, // H
   0b00011110, // J
   0b00111000, // L
   0b01110011, // P
   0b01000000, // -

   0b10000000, // .

   0b00000000  // display off
};

gpioMap_t valor[] = {
   GPIO0,  // a
   GPIO1,  // b
   GPIO2,  // c
   GPIO3,  // d
   GPIO4,  // e
   GPIO5,  // f
   GPIO6,  // g
   GPIO7   // h = dp
};

gpioMap_t digito[] = {
   LCD4, // D4
   LCD3, // D3
   LCD2, // D2
   LCD1  // D1
};

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
   1,2,3,'A',
   4,5,6,'B',
   7,8,9,'C',
   '*',0,'#','D',-1
};

//** MEF teclado

void ingreso_piso (void);

uint8_t TecladoP = 0;
int tecla_apretada = NO;
int tecla = -1;
int digito1 = 0, digito1_cargado = 0, digito2 = 0, digito2_cargado = 0;
int piso = 0;
int pisos_pendientes[10] = {0,0,0,0,0,0,0,0,0,0};
int cantidad_almacenada = 0;
int veloc_piso = 1;
int veloc_apertura = 1;
int cantidad_pisos = 16;
int cantidad_subsuelos = 3;

//****** MEF ascensor
typedef enum {
   EN_PLANTA_BAJA,
   MODO_CONFIGURACION,
   BAJANDO,
   SUBIENDO,
   PARADO,
   YENDO_A_PLANTA_BAJA
} estado1_t;

estado1_t estado_ascensor;

typedef enum {
   PUERTAS_CERRADAS,
   ABRIENDO_PUERTAS,
   PUERTAS_ABIERTAS,
   INTENTANDO_CERRAR_PUERTAS,
   ALARMA_PUERTAS_ABIERTAS,
   CERRANDO_PUERTAS
} estado2_t;

estado2_t estado_puertas;

typedef enum {
   EN_ESPERA_DE_DIGITO_1,
   EN_ESPERA_DE_DIGITO_2_O_LETRA,
   EN_ESPERA_DE_LETRA,
   GUARDAR_PISO
} estado3_t;

estado3_t estado_ingreso_piso;

typedef enum {
   UNO,
   DOS,
   TRES,
   CUATRO,
   CINCO
} estado4_t;

estado4_t estado_configuracion;

int peticion_subir = NO;
int peticion_bajar = NO;
int peticion_configurar = NO;
int secuencia_apertura_puertas = NO;
int secuencia_cerrado_puertas = NO;
int piso_actual = 0;
int piso_final = 0;
int estado_ascensor_anterior = EN_PLANTA_BAJA;


void mefAscensor_inicializar(void);
void mefAscensor(void);
void mefPuertas(void);
void mefIngresoPiso(void);
void mefModoConfiguracion(void);
void comparar(void);
int en_piso = NO;
int subsuelo = NO;
int movimiento_habilitado = NO;
int recorridos = 0;
char indicador = ' ';
int mensaje = 0;
void displayMostrarDigito( uint8_t val, uint8_t dig );
void displayMostrar( char mov, int val, uint32_t tiempo );

int main( void )
{
   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma
   boardConfig();

   int i = 0;

   // Inicializar UART_USB como salida de consola
   consolePrintConfigUart( UART_USB, 115200 );

   mefAscensor_inicializar();

   estado_puertas = PUERTAS_CERRADAS;
   estado_ingreso_piso = EN_ESPERA_DE_DIGITO_1;

   tecladoInit();

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE ) {

      tecla = conversor[escanearTeclado()]; // tecla = conversor[16] = -1
      mefIngresoPiso();
      mefAscensor();
      mefPuertas();
      comparar();

      if( (estado_ingreso_piso == EN_ESPERA_DE_DIGITO_2_O_LETRA) || (estado_ingreso_piso == EN_ESPERA_DE_LETRA) ) {
         displayMostrar( indicador, piso, TIEMPO_DISP);
      }
      if( (estado_ingreso_piso == EN_ESPERA_DE_DIGITO_1) ) {
         displayMostrar( indicador, piso_actual, TIEMPO_DISP); // esta rutina lleva unos 100 ms
      }
   }

// NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
// directamente sobre un microcontroladore y no es llamado por ningun
// Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

void mefAscensor_inicializar(void)
{
   estado_ascensor = EN_PLANTA_BAJA;
   gpioWrite(LED_ascensor_detenido,ON);
}
void mefAscensor(void)
{
   static int tiempo_parado = 0;
   static int tiempo_subida = 0;
   static int tiempo_bajada1 = 0;
   static int tiempo_bajada2 = 0;
   static int primera_vez_PB = SI;
   static int primera_vez_abiertas = SI;
   static int primera_vez_cerrando = SI;
   static int primera_vez_parado = SI;
   static int valor_ingresado = 0;

   switch( estado_ascensor ) {
   case EN_PLANTA_BAJA: {
      estado_ascensor_anterior = EN_PLANTA_BAJA;
      if( primera_vez_PB == SI ) {
         secuencia_apertura_puertas = SI;
         primera_vez_PB = NO;
      }
      if( peticion_subir == SI ) {
         if( secuencia_cerrado_puertas == NO ) {
            secuencia_cerrado_puertas = SI;
            break;
         }
         if( estado_puertas == PUERTAS_CERRADAS ) {
            estado_ascensor = SUBIENDO;
            gpioWrite(LED_ascensor_detenido,OFF);
            gpioWrite(LED_ascensor_moviendo,ON);
            indicador = 's';
            secuencia_cerrado_puertas = NO;
            break;
         }
      }
      if( peticion_bajar == SI ) {
         if( secuencia_cerrado_puertas == NO ) {
            secuencia_cerrado_puertas = SI;
            break;
         }
         if( estado_puertas == PUERTAS_CERRADAS ) {
            estado_ascensor = BAJANDO;
            gpioWrite(LED_ascensor_detenido,OFF);
            gpioWrite(LED_ascensor_moviendo,ON);
            indicador = 'b';
            secuencia_cerrado_puertas = NO;
            break;
         }
      }
      if( peticion_configurar == SI ) {
         if ( secuencia_cerrado_puertas == NO ) {
            secuencia_cerrado_puertas = SI;
            break;
         }
         if( estado_puertas == PUERTAS_CERRADAS ) {
            estado_ascensor = MODO_CONFIGURACION;
            secuencia_cerrado_puertas = NO;
         }
      }
      break;
   }
   case MODO_CONFIGURACION: {

      if ( estado_ascensor_anterior == EN_PLANTA_BAJA ) {

         switch(mensaje) {
         case 0: {
            printf("\r\nEN MODO CONFIGURACION\r\n");
            printf("1 - Configurar velocidad entre piso y piso\r\n");
            printf("2 - Configurar velocidad de apertura o cerrado de puerta\r\n");
            printf("3 - Configurar cantidad de pisos (1 a 20)\r\n");
            printf("4 - Configurar cantidad de subsuelos (1 a 5)\r\n");
            printf("5 - Salir del modo configuracion\r\n\r\n");
            printf("Seleccione la accion a realizar: ");

            fflush(stdout);
            scanf("%d", &mensaje);
            printf("%d:\r\n", mensaje);
            if( (mensaje < 1) || (mensaje > 5 ) ) {
               printf("Valor ingresado fuera de rango!\r\n");
               mensaje = 0;
            }
            break;
         }
         case 1: {
            printf("Ingrese velocidad entre piso y piso, tiempo en segundos (1 a 10): \r\n");
            fflush(stdout);
            scanf("%d", &valor_ingresado);
            printf("El valor ingresado fue %d\r\n", valor_ingresado);

            if( (valor_ingresado >= 1) && (valor_ingresado <= 10 ) ) {
               veloc_piso = valor_ingresado;
            } else {
               printf("Valor ingresado fuera de rango!\r\n");
            }
            mensaje = 0;
            break;
         }
         case 2: {
            printf("Ingrese velocidad de apertura y cierre de puerta, tiempo en segundos (1 a 10): \r\n");
            fflush(stdout);
            scanf("%d", &valor_ingresado);
            printf("El valor ingresado fue %d\r\n", valor_ingresado);

            if( (valor_ingresado >= 1) && (valor_ingresado <= 10 ) ) {
               veloc_apertura = valor_ingresado;
            } else {
               printf("Valor ingresado fuera de rango!\r\n");
            }
            mensaje = 0;
            break;
         }
         case 3: {
            printf("Ingrese la cantidad de pisos (1 a 20): \r\n");
            fflush(stdout);
            scanf("%d", &valor_ingresado);
            printf("El valor ingresado fue %d\r\n", valor_ingresado);

            if( (valor_ingresado >= 1) && (valor_ingresado <= 20 ) ) {
               cantidad_pisos = valor_ingresado;
            } else {
               printf("Valor ingresado fuera de rango!\r\n");
            }
            mensaje = 0;
            break;
         }
         case 4: {
            printf("Ingrese la cantidad subsuelos (1 a 5): \r\n");
            fflush(stdout);
            scanf("%d", &valor_ingresado);
            printf("El valor ingresado fue %d\r\n", valor_ingresado);

            if( (valor_ingresado >= 1) && (valor_ingresado <= 5 ) ) {
               cantidad_subsuelos = valor_ingresado;
            } else {
               printf("Valor ingresado fuera de rango!\r\n");
            }
            mensaje = 0;
            break;
         }
         case 5: {
            printf("Se ha salido del modo CONFIGURACION\r\n");
            estado_ascensor = EN_PLANTA_BAJA;
            piso_actual = 0;
            break;
         }
         default: {
            printf("DEFAULT\r\n");
            break;
         }
         }
      }
      break;
   }
   case BAJANDO: {
      estado_ascensor_anterior = BAJANDO;
      if( en_piso == SI ) {
         estado_ascensor = PARADO;
         gpioWrite(LED_ascensor_detenido,ON);
         gpioWrite(LED_ascensor_moviendo,OFF);
         indicador = ' ';
         printf("EN PISO\r\n");
         tiempo_bajada1 = 0;
         movimiento_habilitado = NO;
      } else {
         if( tiempo_bajada1 >= (veloc_piso*UN_SEGUNDO/TIEMPO_DISP) ) {
            piso_actual--;
            tiempo_bajada1 = 0;
         }
         tiempo_bajada1++;
      }
      break;
   }
   case SUBIENDO: {
      estado_ascensor_anterior = SUBIENDO;
      if( en_piso == SI ) {
         estado_ascensor = PARADO;
         indicador = ' ';
         gpioWrite(LED_ascensor_detenido,ON);
         gpioWrite(LED_ascensor_moviendo,OFF);
         printf("EN PISO\r\n");
         tiempo_subida = 0;
         movimiento_habilitado = NO;
      } else {
         if( tiempo_subida >= (veloc_piso*UN_SEGUNDO/TIEMPO_DISP) ) {
            piso_actual++;
            tiempo_subida = 0;
            printf("piso actual = %d\r\n",piso_actual);
         }
         tiempo_subida++;
      }
      break;
   }
   case PARADO: {
      estado_ascensor_anterior = PARADO;
      if( primera_vez_parado == SI ) {
         secuencia_apertura_puertas = SI;
         tiempo_parado = 0;
         primera_vez_parado = NO;
         movimiento_habilitado = NO;
      }
      if( estado_puertas == PUERTAS_ABIERTAS ) {
         secuencia_cerrado_puertas = SI;
      } else {
         if( (estado_puertas == PUERTAS_CERRADAS ) && (movimiento_habilitado == SI) ) {
            if( peticion_subir == SI ) {
               estado_ascensor = SUBIENDO;
               gpioWrite(LED_ascensor_detenido,OFF);
               gpioWrite(LED_ascensor_moviendo,ON);
               indicador = 's';
               en_piso = NO;
               secuencia_cerrado_puertas = NO;
               primera_vez_parado = SI;
            }
            if( peticion_bajar == SI ) {
               estado_ascensor = BAJANDO;
               gpioWrite(LED_ascensor_detenido,OFF);
               gpioWrite(LED_ascensor_moviendo,ON);
               indicador = 'b';
               en_piso = NO;
               secuencia_cerrado_puertas = NO;
               primera_vez_parado = SI;
            }
         }
      }

      if( (peticion_subir == NO) && (peticion_bajar == NO) ) {
         if( tiempo_parado < ((3*UN_SEGUNDO)/TIEMPO_DISP) ) {  //100*100ms = 10000
            tiempo_parado++;
         }
      }

      if( tiempo_parado >= ((3*UN_SEGUNDO)/TIEMPO_DISP) ) {
         if( estado_puertas == PUERTAS_CERRADAS ) {
            if( estado_ascensor != EN_PLANTA_BAJA ) {
               estado_ascensor = YENDO_A_PLANTA_BAJA;
               gpioWrite(LED_ascensor_detenido,OFF);
               gpioWrite(LED_ascensor_moviendo,ON);
               en_piso = NO;
               primera_vez_parado = SI;
               movimiento_habilitado = SI;
               printf("yendo a planta baja\r\n");
            }
         }
      }
      break;
   }
   case YENDO_A_PLANTA_BAJA: {
      estado_ascensor_anterior = YENDO_A_PLANTA_BAJA;
      if( en_piso == SI ) {
         estado_ascensor = EN_PLANTA_BAJA;
         gpioWrite(LED_ascensor_moviendo,OFF);
         gpioWrite(LED_ascensor_detenido,ON);
         tiempo_bajada2 = 0;
         indicador = ' ';
         primera_vez_PB = SI;
      } else {
         if( tiempo_bajada2 > (veloc_piso*UN_SEGUNDO/TIEMPO_DISP) ) {
            if( subsuelo == SI ) {
               indicador = 's';
               piso_actual++;
            } else {
               piso_actual--;
               indicador = 'b';
               printf("piso actual = %d\r\n", piso_actual);
            }
            tiempo_bajada2 = 0;
         }
         tiempo_bajada2++;
      }
      break;
   }
   default: {
      break;
   }
   }
}
void mefPuertas(void)
{
   static int tiempo1p = 0;
   static int tiempo2p = 0;
   static int tiempo3p = 0;
   static int tiempo4p = 0;
   static int primera_vez_abriendo = SI;
   static int primera_vez_abiertas = SI;
   static int primera_vez_cerrando = SI;

   switch(estado_puertas) {
   case PUERTAS_CERRADAS: {
      if( secuencia_apertura_puertas == SI ) {
         estado_puertas = ABRIENDO_PUERTAS;
         gpioWrite(LED_abriendo_puerta,ON);
         secuencia_apertura_puertas = NO;
         printf("ABRIENDO_PUERTAS...\r\n");
      }
      break;
   }
   case ABRIENDO_PUERTAS: {
      if( primera_vez_abriendo == SI ) {
         tiempo1p = 0;
         primera_vez_abriendo = NO;
      }
      if( tiempo1p < (veloc_apertura*UN_SEGUNDO/TIEMPO_DISP) ) {
         tiempo1p++;
      }
      if( tiempo1p >= (veloc_apertura*UN_SEGUNDO/TIEMPO_DISP) ) {
         estado_puertas = PUERTAS_ABIERTAS;
         movimiento_habilitado = NO;
         primera_vez_abriendo = SI;
         gpioWrite(LED_abriendo_puerta,OFF);
         gpioWrite(LED_puerta_abierta,ON);
         printf("PUERTAS_ABIERTAS...\r\n");
      }
      break;
   }
   case PUERTAS_ABIERTAS: {
      if( primera_vez_abiertas == SI ) {
         tiempo2p = 0;
         primera_vez_abiertas = NO;
      }
      if( tiempo2p < ((2*UN_SEGUNDO)/TIEMPO_DISP) ) {
         tiempo2p++;
      }
      if( tiempo2p >= ((2*UN_SEGUNDO)/TIEMPO_DISP) ) {
         if( secuencia_cerrado_puertas == SI ) {
            estado_puertas = INTENTANDO_CERRAR_PUERTAS;
            primera_vez_abiertas = SI;
            printf("INTENTANDO_CERRAR_PUERTAS...\r\n");
         }
      }
      break;
   }
   case INTENTANDO_CERRAR_PUERTAS: {
      if( gpioRead(TEC1) == 0 ) {   //hay gente
         estado_puertas = ALARMA_PUERTAS_ABIERTAS;
         gpioWrite(LED_puerta_abierta,OFF);
         printf("ALARMA_PUERTAS_ABIERTAS...\r\n");
      } else {                   // no hay gente
         estado_puertas = CERRANDO_PUERTAS;
         gpioWrite(LED_puerta_abierta,OFF);
         gpioWrite(LED_cerrando_puerta,ON);
         printf("CERRANDO_PUERTAS...\r\n");
      }
      break;
   }
   case ALARMA_PUERTAS_ABIERTAS: {
      if( tiempo4p < (UN_SEGUNDO/2)/(TIEMPO_DISP) ) {
         tiempo4p++;
      }
      if( tiempo4p >= (UN_SEGUNDO/2)/(TIEMPO_DISP) ) { // (1000/2)/100 = 5
         gpioToggle(LED_alarma_puerta);
         tiempo4p = 0;
      }
      if( gpioRead( TEC1 ) == 1) {   //no hay gente
         estado_puertas = CERRANDO_PUERTAS;
         gpioWrite( LED_alarma_puerta,OFF );
         printf("CERRANDO_PUERTAS...\r\n");
      }
      break;
   }
   case CERRANDO_PUERTAS: {
      if( primera_vez_cerrando == SI ) {
         tiempo3p = 0;
         primera_vez_cerrando = NO;
      }
      if( tiempo3p < (veloc_apertura*UN_SEGUNDO/TIEMPO_DISP) ) {
         tiempo3p++;
      }
      if( tiempo3p >= (veloc_apertura*UN_SEGUNDO/TIEMPO_DISP) ) {
         if( secuencia_cerrado_puertas == SI ) {
            estado_puertas = PUERTAS_CERRADAS;
            secuencia_cerrado_puertas = NO;
            primera_vez_cerrando = SI;
            gpioWrite( LED_cerrando_puerta,OFF );
            printf("PUERTAS_CERRADAS...\r\n");
         }
      }
      break;
   }
   }
}
void comparar(void)
{
   static int tempo = 0;
   int j = 0;
   int m = 0;
   if( piso_actual < 0 ) {
      subsuelo = SI;
   } else {
      subsuelo = NO;
   }

   if( cantidad_almacenada > 0 )  {
      if( piso_actual == pisos_pendientes[0] ) {//0 == pisos_pendientes[0]
         en_piso = SI;
         peticion_subir = NO;
         peticion_bajar = NO;
         movimiento_habilitado = NO;
         for(j = 1; j < cantidad_almacenada; j++) {
            tempo = pisos_pendientes[j];
            pisos_pendientes[j-1] = tempo;
         }
         pisos_pendientes[cantidad_almacenada - 1] = 0;
         cantidad_almacenada = cantidad_almacenada - 1;
      } else {
         if(piso_actual < pisos_pendientes[0]) {//0 < pisos_pendientes[0]
            peticion_subir = SI;
            peticion_bajar = NO;
            en_piso = NO;
            movimiento_habilitado = SI;
         }
         if(piso_actual > pisos_pendientes[0]) {//0 < pisos_pendientes[0]
            peticion_subir = NO;
            peticion_bajar = SI;
            en_piso = NO;
            movimiento_habilitado = SI;
         }
      }
   }
   if( estado_ascensor == YENDO_A_PLANTA_BAJA ) {// esta opcion es si está yendo a planta y se solicita subir/bajar
      if( piso_actual == 0 ) {
         en_piso = SI;
         peticion_subir = NO;
         peticion_bajar = NO;
         movimiento_habilitado = NO;
      } else {
         if( peticion_subir == SI ) {
            gpioWrite(LED_ascensor_detenido,ON); //----------- ver el ON
            estado_ascensor = SUBIENDO;
         }
         if( peticion_bajar == SI ) {
            gpioWrite(LED_ascensor_detenido,ON);
            estado_ascensor = 8;//BAJANDO;
         }
      }
   }
}
void mefIngresoPiso (void)
{
   switch( estado_ingreso_piso ) {
   case EN_ESPERA_DE_DIGITO_1: {
      digito1 = 0;
      digito2 = 0;
      digito1_cargado = NO;
      digito2_cargado = NO;
      piso = piso_actual;
      if( tecla != -1 ) {                         // se presiona una tecla
         if( tecla == '*' ) {
            estado_ingreso_piso = EN_ESPERA_DE_DIGITO_2_O_LETRA;
         } else {
            if( ( tecla >= 0 ) && ( tecla <= 9 ))  {    // la tecla es un número
               estado_ingreso_piso = EN_ESPERA_DE_DIGITO_2_O_LETRA;        // el * es para indicar subsuelo
            }
         }
      }
      break;
   }
   case EN_ESPERA_DE_DIGITO_2_O_LETRA: {

      static int temporizador1 = 0;

      temporizador1++;
      if( temporizador1 < ((5*UN_SEGUNDO)/TIEMPO_DISP) ) {
         if( tecla != -1) {
            if( (tecla_apretada == NO) && (digito1_cargado == NO) ) { // carga de digito 1, una vez
               if( tecla != '*' ) {
                  digito1 = tecla;
                  digito1_cargado = SI;
                  tecla_apretada = SI;
                  piso = digito1;
                  temporizador1 = 0;
                  break;
               } else {
                  digito1 = ES_SUBSUELO;        //para indicar que es subsuelo
                  digito1_cargado = SI;
                  tecla_apretada = SI;
                  piso = digito1;
                  temporizador1 = 0;
                  printf("digito1 = %d\r\n", digito1);
                  break;
               }
            }
            // la variable tecla_apretada se hace NO en la rutina de lectura de teclado cuando no hay tecla apretada
            if( (tecla_apretada == NO) && (digito1_cargado == SI) ) {
               if( digito1 == ES_SUBSUELO )  {                       // para subsuelo
                  if( (tecla >= 1) && (tecla <= cantidad_subsuelos) ) {
                     estado_ingreso_piso = EN_ESPERA_DE_LETRA;
                     break;
                  }
                  if( tecla == 'B' ) {            // tecla para cancelar
                     estado_ingreso_piso = EN_ESPERA_DE_DIGITO_1;
                     break;
                  }
               }
               if( digito1 == 9 ) {
                  if ( tecla == 9) {
                     estado_ingreso_piso = EN_ESPERA_DE_LETRA;
                     break;
                  }
               }
               if( digito1 == 1 ) {
                  if( (cantidad_pisos >= 10) && (cantidad_pisos <= 19) ) {
                     if( (tecla >= 0) && (tecla <= (cantidad_pisos%10)) ) {
                        estado_ingreso_piso = EN_ESPERA_DE_LETRA;
                        break;
                     }
                  }
                  if( (cantidad_pisos == 20) ) {// sólo para pisos del 10 al 19
                     if( (tecla >= 0) && (tecla <= 9) ) {
                        estado_ingreso_piso = EN_ESPERA_DE_LETRA;
                        break;
                     }
                  }
               }
               if( (digito1 == 2) && (tecla == 0) && (cantidad_pisos == 20)) {       // sólo para el piso 20
                  estado_ingreso_piso = EN_ESPERA_DE_LETRA;
                  break;
               }
               if( digito1 != ES_SUBSUELO ) {
                  if(tecla == 'A') {            // tecla para guardar
                     if ( digito1 <= cantidad_pisos%10) {
                        estado_ingreso_piso = GUARDAR_PISO;
                        printf("digito1_new = %d\r\n", digito1);
                        printf("paso\r\n");
                        break;
                     }
                  }
                  if(tecla == 'B') {            // tecla para cancelar
                     estado_ingreso_piso = EN_ESPERA_DE_DIGITO_1;
                     break;
                  }
               }
            }
         }
      } else {          // temporizador1 = time-out
         estado_ingreso_piso = EN_ESPERA_DE_DIGITO_1;
         temporizador1 = 0;
      }
      break;
   }
   case EN_ESPERA_DE_LETRA: {

      static int temporizador2 = 0;

      temporizador2++;
      if( temporizador2 < ((5*UN_SEGUNDO)/TIEMPO_DISP) ) {
         if( tecla != -1 ) {
            if( (tecla_apretada == NO) && (digito2_cargado == NO) ) {
               if( digito1 == ES_SUBSUELO )  {// -6 indica que es subsuelo
                  digito2 = tecla;
                  digito2_cargado = SI;
                  piso = -1 * digito2;
                  tecla_apretada = SI;
                  temporizador2 = 0;
                  break;
               } else {
                  digito2 = tecla;
                  digito2_cargado = SI;
                  piso = 10*digito1 + digito2;
                  tecla_apretada = SI;
                  temporizador2 = 0;
                  break;
               }
            }
            if( tecla == 'A') {
               if( piso == 99) {
                  estado_ascensor = MODO_CONFIGURACION;
                  estado_ingreso_piso = EN_ESPERA_DE_DIGITO_1;
                  piso_actual = 99;
                  break;
               }
               if(digito1 != ES_SUBSUELO)       // no es subsuelo
                  estado_ingreso_piso = GUARDAR_PISO;
               else {
                  if((digito2_cargado == SI) && (digito2 != 'A'))
                     estado_ingreso_piso = GUARDAR_PISO;
               }
               break;
            }
            if( tecla == 'B') {
               estado_ingreso_piso = EN_ESPERA_DE_DIGITO_1;
               break;
            }
         }
      } else {          // temporizador2 = time-out
         estado_ingreso_piso = EN_ESPERA_DE_DIGITO_1;
         temporizador2 = 0;
      }
      break;
   }
   case GUARDAR_PISO: {
      if( cantidad_almacenada < 10 ) {
         if( digito2_cargado == SI ) {          // el piso es de dos dígitos
            if( digito1 == ES_SUBSUELO ) {
               pisos_pendientes[cantidad_almacenada] = -1*digito2;
            } else {
               pisos_pendientes[cantidad_almacenada] = 10*digito1 + digito2; // el piso es de dos dígitos
            }
         } else {                            // el piso es de un dígito
            pisos_pendientes[cantidad_almacenada] = digito1;              // el piso es de un dígito
         }
         cantidad_almacenada++;
      }
      estado_ingreso_piso = EN_ESPERA_DE_DIGITO_1;
      break;
   }
   }
}
void tecladoInit( void )
{
   int k;
   for( k=0; k<8; k++ ) {
      gpioInit( valor[k], GPIO_OUTPUT );
   }

   for( k=0; k<4; k++ ) {
      gpioInit( digito[k], GPIO_OUTPUT );
   }

   gpioWrite( digito[0], DIGIT_OFF );
   gpioWrite( digito[1], DIGIT_OFF );
   gpioWrite( digito[2], DIGIT_OFF );
   gpioWrite( digito[3], DIGIT_OFF );

   for( k=0; k<8; k++ ) {
      gpioWrite( valor[k], PIN_VALUE_OFF );
   }

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
   int8_t indice = 16;
   int8_t fil = 0;
   int8_t col = 0;

   for( fil = 0; fil < 4; fil++ ) {
      gpioWrite( TecladoPinesFilas[fil], ON ); // se pone a 1 la fila
   }

   for( fil = 0; fil < 4; fil++ ) {
      gpioWrite( TecladoPinesFilas[fil], OFF ); // se pone a 0 la fila
      for( col = 0; col < 4; col++ ) {
         if( gpioRead(TecladoPinesColumnas[col]) == 0 ) { // chequeo si alguna columna vale 0
            indice = fil*4 + col;
            return indice;
         }
      }
   }
   tecla_apretada = NO;
   return 16;
}
void displayMostrarDigito( uint8_t val, uint8_t dig )
{
   uint8_t j = 0;
   gpioWrite( digito[dig], DIGIT_ON );
   for( j=0; j<8; j++ ) {
      if( display7SegmentOut[val] & (1<<j) ) {
         gpioWrite( valor[j], PIN_VALUE_ON );
      } else {
         gpioWrite( valor[j], PIN_VALUE_OFF );
      }
   }
   delay(T_ESPERA);
   gpioWrite( digito[dig], DIGIT_OFF );
}
void displayMostrar( char mov, int val, uint32_t tiempo )
{
   uint8_t i = 0;

   uint8_t disp4 = APAGADO;
   uint8_t disp3 = APAGADO;
   uint8_t disp2 = APAGADO;
   uint8_t disp1 = APAGADO;

   for( i=0; i<4; i++ ) {
      gpioWrite( digito[i], DIGIT_OFF );
   }

   if( val < 0 ) {
      if( val == ES_SUBSUELO ) {
         disp4 = APAGADO;        // se apaga display
         disp2 = SIGNO_MENOS;    // signo menos "-"
         disp1 = APAGADO;
      } else {
         disp4 = APAGADO;        // se apaga display
         disp2 = SIGNO_MENOS;
         disp1 = -1*val;   
      }
   }

   if( (val >= 0)) { 
      disp2 = (val/10);
      if( disp2 == 0 ) {
         disp2 = APAGADO;    // se apaga display
      }
      disp1 = val%10;
   }
   if( (val == 0) && ((estado_ascensor == EN_PLANTA_BAJA) || (estado_ascensor == PARADO)) ) {
      disp4 = APAGADO;    // se apaga display
      disp2 = P;    //letra p
      disp1 = b;    //letra b
   }

   switch(mov) {
   case 's':
      disp4 = s;     //lestra s, para indicar que está subiendo
      break;
   case 'b':
      disp4 = b;    //letra b, para indicar que está bajando
      break;
   }

   for( i=0; i<tiempo/(4*T_ESPERA); i++ ) {       // i<100/20-> i<5    //
      displayMostrarDigito( disp1, 0 ); // tarda 5 ms
      displayMostrarDigito( disp2, 1 ); // tarda 5 ms
      displayMostrarDigito( disp3, 2 ); // tarda 5 ms
      displayMostrarDigito( disp4, 3 ); // tarda 5 ms
   }                                    // 5*20ms = 100 ms
}