/**  Fichero JSON.c
  *
  *  Este fichero contiene las funciones necesarias para realizar la
  *  solicitud de petición HTTP al servidor y la obtención del objeto JSON,
  *  así como su almacenamiento y posterior tratamiento.
  *
  *  Nombre: Juan Carlos Calvo Sansegundo.   Fecha: 20/10/2016.
  */

#include "json.h"

/**  Funcion crear_memoria().
  *
  *  Esta función se encarga de crear una memoria dinámica.
  *  Se le pasa como parámetro el tamaño indicado.
  *  Devuelve un puntero de tipo char que almacena la dirección 
  *  en la que se encuentra. En caso contrario devuelve null.
  *
  *  Nombre: Juan Carlos Calvo Sansegundo.  Fecha: 20/10/2016.
  */

  char * crear_memoria(int tam_memoria){

     char * memoria;
     memoria =(char*)calloc(tam_memoria,sizeof(char));

     if(memoria == NULL){
	 fprintf(stderr,"Error: No se ha creado correctamente la memoria \n");
         return NULL;
     }else{
         return memoria;
     }
  } 


/**  Funcion escribir_respuesta().
  *
  *  Esta función se encarga de escribir el objeto JSON en la memoria, además de
  *  comprobar que la memoria creada tiene tamaño suficiente para almacenarlo.
  *  Devuelve el tamaño que ha ocupado el objeto en un size_t (unsigned int).
  *  En caso de no poder escribir, devuelve un 0.
  *
  *  Nombre: Juan Carlos Calvo Sansegundo.  Fecha: 20/10/2016.
  */

  size_t escribir_respuesta(void *ptr, size_t size, size_t nmemb, void *stream, int tam_memoria){
     struct escribir_resultado *result = (struct escribir_resultado *)stream;

     if(result->posicion + size * nmemb >=tam_memoria - 1){
        fprintf(stderr, "error: Búffer demasiado pequeño.\n");
        return 0;
     }

     memcpy(result->datos + result->posicion, ptr, size * nmemb);
     result->posicion += size * nmemb;

     return size * nmemb;
  }


/**  Función solicitar_http().
  *  
  *  Esta función se encarga de realizar la petición http al servidor y obtener el objeto JSON.
  *  Se le pasa la URL de la petición HTTP y el tamaño del buffer de almacenamiento.
  *  Devuelve un puntero a la memoria donde se almacena el objeto JSON.
  *  En caso contrario devuelve null.
  *
  *  Nombre: Juan Carlos Calvo Sansegundo.  Fecha: 20/10/2016.
  */

  char * solicitar_http(const char * url,int tamano_buffer){
      
     CURL * curl = NULL;   // Creación de un puntero a una estructura de tipo curl.
     CURLcode estado;      // Guarda el estado sobre la conexión http.
     long codigo;          // Almacena el código HTTP sobre el resultado de la petición.

     curl_global_init(CURL_GLOBAL_ALL);
     curl=curl_easy_init();               // Creación del manejador de funciones de librería curl.
 
     if(!curl){
        fprintf(stderr,"Error: No se ha podido inicializar el curl\n");
        curl_global_cleanup();
        return NULL;
     }

     struct escribir_resultado resultado={
	.datos = crear_memoria(tamano_buffer),
        .posicion = 0
     };

     if(resultado.datos == NULL){
         fprintf(stderr,"Error: No se ha creado bien el buffer\n");
         free(curl);
         curl_global_cleanup();
         return NULL;
     }
     
     curl_easy_setopt(curl,CURLOPT_URL,url);
     curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,escribir_respuesta);
     curl_easy_setopt(curl,CURLOPT_WRITEDATA,&resultado);
     estado = curl_easy_perform(curl);

     if(estado !=0){
	fprintf(stderr,"Error: No se puede conectar con la url: %s\n",url);	
        fprintf(stderr,"%s\n",curl_easy_strerror(estado));
        
        free(resultado.datos);
        free(curl);
        curl_global_cleanup();
        return NULL;
     }

     curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &codigo);

     if(codigo !=200){
        fprintf(stderr,"Error: El servidor ha respondido con el código %ld\n",codigo);
	free(resultado.datos);
        free(curl);
        curl_global_cleanup();
        return NULL;
     }

     curl_easy_cleanup(curl);
     curl_global_cleanup();
     
     resultado.datos[resultado.posicion]='\0';

     printf("___ Conexión correcta \n\n");
     
     return resultado.datos;
  }


/**  Función extraer_dato().
  *  
  *  Esta función se encarga de extraer la temperatura del objeto JSON.
  *  No se pasa ningún parámetro.
  *  Devuelve un puntero de tipo int con el instante de medida y la temperatura.
  *  Si no puede extraer el dato devuelve null.
  *
  *  Nombre: Juan Carlos Calvo Sansegundo.  Fecha: 25/10/2016.
  */


struct dato_leido* extraer_dato(int url_size,const char* url_format, int tamano_buffer){

     char * texto;
     char url[url_size];
     size_t i = 0;

     struct dato_leido * resultado = (struct dato_leido*)calloc(1,sizeof(struct dato_leido));
     resultado->temp_leida = 0.0;
     resultado->instante = 0;
   
     json_t* obj_completo;
     json_t* obj_objetivo;
     json_error_t errores;

     json_t* temperatura;
     json_t* instante;

   
     snprintf(url, url_size, url_format);
     texto = solicitar_http(url, tamano_buffer);
    
     if(texto == NULL){
	fprintf(stderr, "Error: No se ha podido conectar con el servidor.\n");
        return NULL;
     }

     obj_completo = json_loads(texto,0,&errores);       // Carga del objeto json:

     if(!obj_completo){
	 fprintf(stderr, "Error: en la línea %d: %s\n", errores.line, errores.text);
         return NULL;
     }

     free(texto);

     obj_objetivo = json_array_get(obj_completo,0);

     if(!json_is_object(obj_objetivo)){
         fprintf(stderr, "Error: No hay objeto.\n");
         return NULL;
     }

     obj_objetivo = json_object_get(obj_objetivo,"datapoints");

     if(!json_is_array(obj_objetivo)){
         fprintf(stderr, "Error: No existen el array de datapoints\n");
         return NULL;
     }

     for(i=0;i<json_array_size(obj_objetivo);i++){
           
           json_t * array_lectura;
           int inst;
           double temp;	

           array_lectura = json_array_get(obj_objetivo,i);
           temperatura = json_array_get(array_lectura,0);
           instante = json_array_get(array_lectura,1);

           if(!json_is_number(temperatura)|| !json_is_number(instante)){
		fprintf(stderr,"____Error: No se ha podido obtener el dato.\n\n");
                continue;
           }
          
           inst = (int)json_integer_value(instante);
           temp = json_real_value(temperatura);

           if(inst >= resultado->instante){
		resultado->temp_leida = temp;
                resultado->instante = inst;
           }

           printf(" Extracción correcta \n");
           printf("____Temperatura leída: %4.2f\n",resultado->temp_leida);
           printf("____Instante_lectura: %d\n\n",resultado->instante);
           
     }

     printf(" Resultado final \n");
     printf("____Temperatura leída: %4.2f\n",resultado->temp_leida);
     printf("____Instante_lectura: %d\n\n",resultado->instante);

     return resultado;

 }
