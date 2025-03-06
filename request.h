#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "d_array.h"
#include <string.h>
#define BUFFER_SIZE 1024

struct hs_request {
    char           raw[BUFFER_SIZE];
    char          *method;
    char          *url;
    char          *proto;
    struct d_array headers;
};

int parse_request(char *buf, struct hs_request *req) {
    strcpy(req->raw, buf);
    char *saveptr = 0;
    req->method   = strtok_r(buf, " ", &saveptr);
    if (req->method == NULL)
        return -1;

    req->url = strtok_r(NULL, " ", &saveptr);
    if (req->url == NULL)
        return -1;

    req->proto = strtok_r(NULL, "\r\n", &saveptr);
    if (req->proto == NULL)
        return -1;

    char *header = strtok_r(NULL, "\r\n", &saveptr);
    char *header_key, *header_val;
    while (header != NULL && strlen(header) != 1) {
        char *h_saveptr = 0;
        header_key      = strtok_r(header, ": ", &h_saveptr);
        header_val      = strtok_r(NULL, "\0", &h_saveptr);
        memmove(header_val, header_val + 1, strlen(header_val));

        d_array_append(&req->headers, header_key);
        d_array_append(&req->headers, header_val);

        header = strtok_r(NULL, "\r\n", &saveptr);
    }

    return 0;
}

#endif // __REQUEST_H__
