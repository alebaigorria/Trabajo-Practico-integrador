#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <time.h>
    
    //link repo : https://github.com/alebaigorria/Trabajo-Practico-integrador

struct memory {
  char *response;
  size_t size;
};

static size_t cb(char *data, size_t size, size_t nmemb, void *clientp)
{
  size_t realsize = nmemb;
  struct memory *mem = clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if(!ptr)
    return 0;  /* out of memory */

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}

long int parsear_cadena(char *cadena,char *buscar);
void guardar_cadena(char *cadena,char *buscar,char *destino,char max);
void convertir_a_minusculas(char *mensaje,char tam);

int main(void){
  char api_url[256];
  const char *api_base = "https://api.telegram.org/bot";
  char token[50];
  const char *urlUpdate = "/getUpdates?offset=";
  long int update_id = 0;
  long int offset = 0;
  long int chat_id = 0;
  char mensaje_de[50] = {0};
  char mensaje[100] = {0};

  FILE *fp = fopen("token.txt","r");
  if (fp == NULL){
      printf("No se encuentra el token");
      exit(1);
  }
  fscanf(fp,"%s",token);
  fclose(fp);


  CURLcode res;
  CURL *curl = curl_easy_init();
  struct memory chunk = {0};

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    while(1){

   if(chunk.response){
     free(chunk.response);
     chunk.response = NULL;
     chunk.size = 0;
    }
    mensaje_de[0] = '\0'; 
    mensaje[0] = '\0';
 
    snprintf(api_url,256,"%s%s%s%ld",api_base,token,urlUpdate,offset);
    curl_easy_setopt(curl, CURLOPT_URL, api_url);
    res = curl_easy_perform(curl);

    if (res != 0){
      printf("Error Código: %d\n", res);
      sleep(2);
      continue;
    }
    
    update_id = parsear_cadena(chunk.response,"\"update_id\":");
    chat_id = parsear_cadena(chunk.response,"\"chat\":{\"id\":");

    char *from_flag = strstr(chunk.response, "\"from\":");
    
    if(from_flag != NULL){
        guardar_cadena(from_flag, "\"first_name\":",mensaje_de,sizeof(mensaje_de));
        guardar_cadena(chunk.response,"\"text\":",mensaje,sizeof(mensaje));
        time_t t = time(NULL);
        struct tm *tiempoActual = localtime(&t);
        FILE *fj = fopen("historial.json","a");
        if (fj == NULL)
            printf("Error con el historial");
        else{
        fprintf(fj,"[%d:%d:%d]%s envió: %s\n",tiempoActual->tm_hour
        ,tiempoActual->tm_min,tiempoActual->tm_hour,mensaje_de,mensaje);
        fclose(fj);
        }
    }

    if(update_id == 0 || chat_id == 0) {
        printf("%s\n",chunk.response);
        continue;
    }

    convertir_a_minusculas(mensaje,sizeof(mensaje));

    if(strcmp(mensaje,"hola") == 0){
        const char *urlSendMessage =  "/sendMessage?chat_id=";
        const char *rtaHola = "&text=Hola%20";

        snprintf(api_url,256,"%s%s%s%ld%s%s",api_base,token,urlSendMessage
        ,chat_id,rtaHola,mensaje_de);

        curl_easy_setopt(curl, CURLOPT_URL, api_url);
        res = curl_easy_perform(curl);
        if(res != 0){
            printf("Error Código: %d\n",res);
            sleep(2);
            continue;
        }
        time_t t = time(NULL);
        struct tm *tiempoActual = localtime(&t);
        FILE *fj = fopen("historial.json","a");
        if (fj == NULL)
            printf("Error con el historial");
        else{
        fprintf(fj,"[%d:%d:%d]el bot envió: Hola %s\n",tiempoActual->tm_hour
        ,tiempoActual->tm_min,tiempoActual->tm_hour,mensaje_de);
        fclose(fj);
        }
    }
    if(strcmp(mensaje,"chau") == 0 || strcmp(mensaje,"adios") == 0){
        const char *urlSendMessage =  "/sendMessage?chat_id=";
        const char *rtaChau = "&text=Hasta%20luego%20";

        snprintf(api_url,256,"%s%s%s%ld%s%s",api_base,token,urlSendMessage
        ,chat_id,rtaChau,mensaje_de);

        curl_easy_setopt(curl, CURLOPT_URL, api_url);
        res = curl_easy_perform(curl);
        if(res != 0){
            printf("Error Código: %d\n",res);
            sleep(2);
            continue;
        }
        time_t t = time(NULL);
        struct tm *tiempoActual = localtime(&t);
        FILE *fj = fopen("historial.json","a");
        if (fj == NULL)
            printf("Error con el historial");
        else{
        fprintf(fj,"[%d:%d:%d]el bot envió: Hasta luego %s\n",tiempoActual->tm_hour
        ,tiempoActual->tm_min,tiempoActual->tm_hour,mensaje_de);
        fclose(fj);
        }
    }
    offset = update_id + 1; 
   }
  curl_easy_cleanup(curl);
 }                                  //fin loop
 return 0;
}

long int parsear_cadena(char *cadena,char *buscar){
    char *p = strstr(cadena,buscar);

    if(p == NULL)
        return 0;
    p += strlen(buscar);
    while(*p == ' ' || *p == '\t')
        p++;

    char buffer[20];
    for (int i=0;i<20;i++){
        if((*p >= '0' && *p <= '9') || *p == '-'){
            buffer[i] = *p;
            p++;
        }
        else{
            buffer[i] = '\0';
            break;
            }
    }
    return atol(buffer);
}

void guardar_cadena(char *cadena,char *buscar,char *destino,char max){
    char *p = strstr(cadena,buscar);
    if(p == NULL){
        *(destino) = '\0';
        return;
    }
    p += strlen(buscar);
    while(*p == ' ' || *p == '\t')
        p++;
    if(*p == '"')
        p++;
    else
        *(destino) = '\0';
    int c = 0;
    while (*p != '"' && *p != '\0' && c < max-1){
        *(destino+c) = *p;
        p++;
        c++;
    }
    *(destino+c) = '\0';
}

void convertir_a_minusculas(char *mensaje,char tam){
    for (int i = 0; i < tam;i++){
        if (*(mensaje+i)<= 'Z' && *(mensaje+i) >= 'A')
            *(mensaje+i) += 32;
        else
            continue;
    }
}

