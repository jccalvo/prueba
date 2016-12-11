/*  Programa.c
 *
 *  Programa que se encarga de leer 2 temperaturas mediante peticiones http.
 *  Posteriormente realiza la diferencia de temperatura de los 2 valores leídos.
 *  Por último, almacena el resultado de la diferencia en un fichero de salida.
 *
 *  Autor: Juan Carlos Calvo Sansegundo.  
 *  Fecha: 11/10/2016.
 *
 *  Version 1.1 
 */
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "json.h"
#include "comandos.h"
#include "spi.h"

#define PERIODOITER 10
#define  TAM_BUFFER (256*1024)  /* Equivale a un tamaño de 256 KB */
#define  URL_SIZE 256
#define  URL_FORMAT   "http://visualizee.die.upm.es:8000/render?format=json&target=visualizee.greencpd.b039.rack.r0001.server.host2.temperature.ambient.1.1000&from=-1min"


/** Función comparar_temperatura().
 *
 *  Esta función se encarga de calcular la diferencia de temperatura entre los 2 valores.
 *  Se pasa como parámetro las 2 temperaturas a comparar.
 *  Se devuelve la diferencia de temperatura calculada.
 *
 *  Nombre: Juan Carlos Calvo Sansegundo.   Fecha: 26/10/2016.
 */

 double comparar_temperatura(double ref, double med){
    double diferencia =ref - med;
    return diferencia;
 }

/** Función escribir_fichero().
 *
 *  Esta función se encarga de escribir en un fichero la diferencia 
 *  de temperatura que se ha calculado previamente.
 *  Se pasa como parámetro el nombre del fichero a escribir.
 *  No se devuelve nada. En caso contrario se devuelve null;
 *  
 *  Nombre: Juan Carlos Calvo Sansegundo.   Fecha: 26/10/2016.
 */

 void* escribir_fichero(char *nombre,double diferencia){
   FILE * f;
   double dif= diferencia;
   if((f=fopen(nombre,"a+"))==NULL){
        printf("-----No se puede escribir en el fichero  %s -----\n",nombre);
        return NULL;
   }else{
   	fprintf(f,"La diferencia de temp es: %4.2f\n",dif); 
        fclose(f);  
        return;
   }
 }

 /** Funcion iteracion()
  *
  *  Esta función se encarga de controlar el periodo en el que se realiza la lectura de las 
  *  temperaturas de cada fichero.
  *  Se pasa como dato las estructuras de tiempo necesarias.
  * 
  *  Nombre: Juan Carlos Calvo Sansegundo.   Fecha: 26/10/2016.
  */

 void iteracion(struct timeval* current, struct timeval * next, struct timeval * timeout){

    gettimeofday(current,NULL);
    next->tv_sec = next->tv_sec + PERIODOITER;

    timeout->tv_sec = next->tv_sec - current->tv_sec;
    timeout->tv_usec = next->tv_usec - current->tv_usec;
    if(timeout->tv_usec <0){
       	timeout->tv_usec = timeout->tv_usec + 1000000;
        timeout->tv_sec = timeout->tv_sec -1;
    }
       
    select(0,NULL,NULL,NULL,timeout); 
 }


 /** Función main()
  *
  *  Ejecución principal del programa.
  */

 int main(){
    struct dato_leido* ref; 
    struct dato_leido* medida; 
    double dato_ref;
    double dato_med;
    double dato_dif;

    struct timeval current_iteration = {0,0};
    struct timeval next_iteration = {0,0};
    struct timeval timeout = {0,0};

    gettimeofday(&current_iteration,NULL);
    gettimeofday(&next_iteration,NULL);

    while(1){
	  gettimeofday(&current_iteration,NULL);
          printf("--->Inst entrada: %10.2f seg y %10.2f mseg:\n\n",(double)current_iteration.tv_sec,(double)current_iteration.tv_usec);

          ref = extraer_dato(URL_SIZE,URL_FORMAT,TAM_BUFFER); 
          medida = extraer_dato(URL_SIZE,URL_FORMAT,TAM_BUFFER); 
        
          if(ref==NULL || medida==NULL){
               fprintf(stderr,"Error: No se ha podido extraer el dato \n");
               return 1;
          }
          dato_ref = ref->temp_leida;
          dato_med = 11.0; //medida->temp_leida;

          dato_dif = comparar_temperatura(dato_ref,dato_med);
          
          escribir_fichero("temp_dif.txt",dato_dif);

          int p = seleccionar_comando(dato_dif);
          struct buf_salida * res = crear_comando(p);

          int m = inicializar_SPI(res);

          iteracion(&current_iteration,&next_iteration,&timeout);
    }
 }


