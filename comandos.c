/**  Fichero comandos.c
  * 
  *  Este fichero contiene las funciones necesarias para que a partir
  *  de la señal obtenida a la salida del controlador PID se pueda
  *  generar el comando necesario para enviar al aire acondicionado.
  *
  *  Nombre: Juan Carlos Calvo Sansegundo.    Fecha: 30/10/2016.
  */

#include "comandos.h"

#define FACTOR_CONV 100.0
#define TEMP_MAXIMA_AC 32.0  // Máxima temperatura aceptada por el aire acondicionado.
#define TEMP_MINIMA_AC 16.0  // Mínima temperatura aceptada por el aire acondicionado.
#define LIMITE_REDONDEO 50

#define URL_FORMAT "trama%d.txt"
#define URL_SIZE 20
#define MASCARA_BITS 0x80

/**  Función seleccionar_comando().
  *
  *  Esta función coge el resultado obtenido a la salida del controlador 
  *  y establece una correspondencia de dicho valor, con los posibles valores
  *  de temperatura que dispone el aire acondicionado.
  *  Toma como parámetro de entrada, la salida del PID
  *  Devuelve un valor de temperatura dentro del rango aceptado por el aire acondicionado.
  *
  *  Nombre: Juan Carlos Calvo Sansegundo.    Fecha: 30/10/2016.
  */

int seleccionar_comando(double temp_sal_PID){

     int temp_int,rest_temp; 

     if(temp_sal_PID>=TEMP_MAXIMA_AC){
	 return (int)TEMP_MAXIMA_AC;
     }
     else if (temp_sal_PID<=TEMP_MINIMA_AC){
	return (int)TEMP_MINIMA_AC;
     }
     else{

         temp_int = (int)(temp_sal_PID*FACTOR_CONV);
         rest_temp = temp_int%(int)FACTOR_CONV; 

         if(rest_temp >= LIMITE_REDONDEO){
             return 1 + temp_int/(int)FACTOR_CONV;
         }
         else{
             return temp_int/(int)FACTOR_CONV;
         }  
     }

}

/**  Funcion elegir_fichero()
  *
  *  Esta función se encarga de generar el nombre del fichero que 
  *  almacena el comando a enviar, de acuerdo con la temperatura
  *  generada por el controlador PID.
  *
  *  Usa como parámetro el valor de la temperatura.
  *  Devuelve un puntero de tipo char con el nombre del fichero.
  *
  *  Nombre: Juan Carlos Calvo Sansegundo.   Fecha: 30/10/2016.
  */

FILE* elegir_fichero(int temperatura,FILE *f){
  
   switch(temperatura){
	case 16:
           f=fopen("trama16.txt","rb");
           break;
	case 17:
           f=fopen("trama17.txt","rb");
           break;
	case 18:
           f=fopen("trama18.txt","rb");
           break;
	case 19:
           f=fopen("trama19.txt","rb");
           break;
	case 20:
           f=fopen("trama20.txt","rb");
           break;
	case 21:
           f=fopen("trama21.txt","rb");
           break;
	case 22:
           f=fopen("trama22.txt","rb");
           break;
	case 23:
           f=fopen("trama23.txt","rb");
           break;
	case 24:
           f=fopen("trama24.txt","rb");
           break;
	case 25:
           f=fopen("trama25.txt","rb");
           break;
	case 26:
           f=fopen("trama26.txt","rb");
           break;
	case 27:
           f=fopen("trama27.txt","rb");
           break;
	case 28:
           f=fopen("trama28.txt","rb");
           break;
	case 29:
           f=fopen("trama29.txt","rb");
           break;
	case 30:
           f=fopen("trama30.txt","rb");
           break;
	case 31:
           f=fopen("trama31.txt","rb");
           break;
	case 32:
           f=fopen("trama32.txt","rb");
           break;
	default:
           f=NULL;
           break;
    }

    return f;
}


/**  Función crear_comando().
  *
  *  Esta función coge el fichero de texto asociado con 
  *  la temperatura obtenida y genera y envía dicho comando 
  *  al módulo de infrarrojos.
  *  Toma como parámetro de entrada, la temperatura a enviar.
  *  Devuelve 0 si se ha realizado correctamente.
  *
  *  Nombre: Juan Carlos Calvo Sansegundo.    Fecha: 30/10/2016.
  */

struct buf_salida * crear_comando(int temperatura){

    FILE * f;
    unsigned char buffer[800];
    unsigned char mask = 0x80;
    unsigned int j = 0;
    int i = 0;
    size_t n;
    struct buf_salida* salida = (struct buf_salida*)calloc(1,sizeof(struct buf_salida));
    salida->datos = (unsigned char*)calloc(100,sizeof(unsigned char));
        
    f = elegir_fichero(temperatura,f);
    if(f == NULL){
       printf("Error: El comando seleccionado no existe\n");
       return NULL;
    }

    n = fread(buffer, 1, sizeof(buffer), f);
    printf("Número de bloques leídos: %u\n",n);


    for(i=0;i<n;i++){
	if(buffer[i]=='1'){
	     salida->datos[j] |= mask;
             salida->posicion = j;
        }
        else if(buffer[i]=='0'){
	     salida->datos[j] &=~mask;
             salida->posicion = j;
        }
        else{
        }

        mask >>=1;
        if(!mask){
	    mask=0x80;
	    j++;
        }   
    }

    salida->datos[j++]='\n';
    salida->posicion=j;

    for(i=0;i<j;i++){
	printf("Byte %d:",i);
        printf("0x%x\n",salida->datos[i]);
    }

    return salida;
}

