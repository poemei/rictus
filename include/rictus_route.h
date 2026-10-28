#ifndef RICTUS_ROUTE_H
#define RICTUS_ROUTE_H

/* Transport-neutral Core ingress. Existing ABI 1.5 host layout stays unchanged. */
#define RICTUS_ROUTER_DISPATCH_SERVICE "rictus.router.dispatch"
#define RICTUS_ROUTE_TEXT_MAX 2048
#define RICTUS_ROUTE_REPLY_MAX 512

typedef struct {
    char sender[128];
    char account[128];
    char text[RICTUS_ROUTE_TEXT_MAX];
} rictus_route_request_t;

typedef struct {
    char text[RICTUS_ROUTE_REPLY_MAX];
    int replied;
} rictus_route_reply_t;

#endif
