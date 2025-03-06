#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "d_array.h"
#include "request.h"

// the max length of request data read from the client
#define BUFFER_SIZE 1024

// mime types for some common extensions configured below
// each entry corresponds to {extension, mime_type}
#define NUM_MIMETYPES 7
#define MIME_TYPE_MAX_LENGTH 17
const char *mime_types[][2] = {
    {"html", "text/html"},    {"xml", "application/xml"},
    {"css", "text/css"},      {"webp", "image/webp"},
    {"jpg", "image/jpeg"},    {"jpeg", "image/jpeg"},
    {"js", "text/javascript"}};

int main(int argc, char **argv) {
    if (argc < 3) {
        dprintf(2, "bad usage\n");
        printf("usage: %s ROOT_DIR PORT\n", argv[0]);
        return 1;
    }
    int  PORT          = 0;
    char root_dir[120] = {0};
    sscanf(argv[1], "%s", root_dir);
    if (root_dir[strlen(root_dir) - 1] == '/') {
        // remove trailing slash in the root dir
        root_dir[strlen(root_dir) - 1] = '\0';
    }
    sscanf(argv[2], "%d", &PORT);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        return 1;
    }
    struct sockaddr_in addr = {AF_INET, htons(PORT), 0};
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        return 1;
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        return 1;
    }
    printf("listening on %d...\n", PORT);

    // handle connections
    char buf[BUFFER_SIZE];
    char filepath[400] = {0};
    strcpy(filepath, root_dir);
    struct hs_request req;
    d_array_init(16, &req.headers);
    while (true) {
        int    client_fd = accept(server_fd, NULL, NULL);
        size_t n         = read(client_fd, buf, BUFFER_SIZE);
        buf[n]           = 0;
        if (parse_request(buf, &req) < 0) {
            printf("invalid request\n");
            continue;
        }

        if (strcmp(req.method, "GET") != 0) {
            dprintf(client_fd, "HTTP/1.1 405 Method Not Allowed\r\n\r\n");
            close(client_fd);
            printf("%-6s %s - 405 Method Not Allowed\n", req.method, req.url);
            continue;
        }

        // parse out and validate the requested fil7e
        strcpy(filepath + strlen(root_dir), req.url);
        if (filepath[strlen(filepath) - 1] == '/') {
            filepath[strlen(filepath) - 1] = '\0';
        }
        struct stat file_stat = {0};
        lstat(filepath, &file_stat);
        if (S_ISDIR(file_stat.st_mode)) {
            strcat(filepath + strlen(filepath) - 1, "/index.html");
        }
        int file_fd = open(filepath, O_RDONLY);
        if (file_fd < 0) {
            dprintf(client_fd, "HTTP/1.1 404 Not Found\r\n\r\n");
            close(client_fd);
            printf("%-6s %s - 404 Not Found\n", req.method, req.url);
            continue;
        }

        // send file as HTTP/1.1 response
        fstat(file_fd, &file_stat);
        char  mime_type[MIME_TYPE_MAX_LENGTH] = "text/plain";
        char *extension     = strrchr(filepath, '.');
        if (extension != NULL) {
            for (int i = 0; i < NUM_MIMETYPES; i++) {
                if (strcmp(extension + 1, mime_types[i][0]) == 0) {
                    strcpy(mime_type, mime_types[i][1]);
                }
            }
        }
        dprintf(
            client_fd,
            "HTTP/1.1 200 OK\r\nConnection: Close\r\nContent-Type: %s\r\nContent-Length: "
            "%ld\r\n\r\n",
            mime_type, file_stat.st_size);
        ssize_t bytes_written =
            sendfile(client_fd, file_fd, 0, file_stat.st_size);
        dprintf(client_fd, "\r\n");
        close(client_fd);
        close(file_fd);
        printf(
            "%-6s %s - 200 OK - %ld bytes\n", req.method, req.url,
            bytes_written);
        d_array_clear(&req.headers);
    }
    d_array_free(&req.headers);
    return 0;
}
