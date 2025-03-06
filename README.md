# Web Server
Barebones web-server in C

## Anatomy
The webserver listens on a port (`8080` by default) and accepts client
connections. Upon a connection, it reads data sent by the client and attempts to
parse it as a HTTP request. If successful, the server tries to send back the
requested file as HTTP/1.1 response.

The server processes only one connection at a time, and closes the connection
after successful transfer of the response data. Basic details of the request /
response is logged to console.

## Implementation

