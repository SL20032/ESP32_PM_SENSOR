#ifndef _GET_REQ_H_
#define _GET_REQ_H_

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

void http_rest_with_url(char* buf, char* host_ip, char* host_path);

#endif