#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

int curl_send_email(const char* to, const char *server_url);

#ifdef __cplusplus
}
#endif

