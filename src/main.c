#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

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
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free(chunk.response);
    return 0;
}