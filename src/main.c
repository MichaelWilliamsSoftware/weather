#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

/*https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html*/
typedef struct {
  char *response;
  size_t size;
} memory;

static size_t write_callback(void *clientp, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = nmemb;
    memory *mem = (memory *)userp;
    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if(!ptr)
        return 0;  /* out of memory */
    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), clientp, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
    return realsize;
}

int main(void)
{
    CURL *curl;
    CURLcode res;
    memory chunk;
    chunk.response = malloc(1);
    chunk.size = 0;
    const char *baseurl = "https://api.open-meteo.com/v1/forecast";
    double latitude = 42.3601;
    double longitude = -71.0589;
    const char *params = "current_weather=true";
    char url[256];
    snprintf(url, sizeof(url), "%s?latitude=%.4f&longitude=%.4f&%s",
             baseurl, latitude, longitude, params);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "Failed\n");
        return 1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "Failed: %s\n",
                curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(chunk.response);
        return 1;
    }
    printf("Raw JSON response:\n%s\n", chunk.response);
    cJSON *json = cJSON_Parse(chunk.response);
    if(!json) {
        fprintf(stderr, "Failed\n");
        curl_easy_cleanup(curl);
        free(chunk.response);
        return 1;
    }
    cJSON *current_weather = cJSON_GetObjectItemCaseSensitive(json, "current_weather");
    if(current_weather) {
        cJSON *temp = cJSON_GetObjectItemCaseSensitive(current_weather, "temperature");
        cJSON *windspeed = cJSON_GetObjectItemCaseSensitive(current_weather, "windspeed");
        cJSON *winddir = cJSON_GetObjectItemCaseSensitive(current_weather, "winddirection");
        cJSON *weathercode = cJSON_GetObjectItemCaseSensitive(current_weather, "weathercode");
        cJSON *time = cJSON_GetObjectItemCaseSensitive(current_weather, "time");
        printf("Boston Current Weather:\n");
        if(time && cJSON_IsString(time))
            printf("Time: %s\n", time->valuestring);
        if(temp && cJSON_IsNumber(temp))
            printf("Temperature: %.2f\n", temp->valuedouble);
        if(windspeed && cJSON_IsNumber(windspeed))
            printf("Wind Speed: %.2f\n", windspeed->valuedouble);
        if(winddir && cJSON_IsNumber(winddir))
            printf("Wind Direction: %.2f\n", winddir->valuedouble);
        if(weathercode && cJSON_IsNumber(weathercode))
            printf("Weather Code: %d\n", weathercode->valueint);
    } else {
        printf("current_weather field not found\n");
    }
    cJSON_Delete(json);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free(chunk.response);
    return 0;
}