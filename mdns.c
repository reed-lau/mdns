#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

void get_ip(char* ip) {
  memset(ip, 0, 100);
  uv_interface_address_t* info;
  int ninfo = 0;
  uv_interface_addresses(&info, &ninfo);

  for (int i = 0; i < ninfo; ++i) {
    if (info[i].address.address4.sin_family == AF_INET &&
        !info[i].is_internal) {
      char name[100] = {0};
      uv_ip4_name(&info[i].address.address4, name, 100);
      strcat(ip, "|");
      strcat(ip, name);
      strcat(ip, "|");
      printf("%s:%s\n", info[i].name, name);
    }
  }
  uv_free_interface_addresses(info, ninfo);
}

void alloc_cb(uv_handle_t* handle, size_t hint, uv_buf_t* buf) {
  buf->base = malloc(hint);
  buf->len = hint;
}

void cl_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf,
                const struct sockaddr* addr, unsigned flags) {
  if (nread == 0) return;
  printf("i recved[%d] nread=%ld: %s\n", flags, nread, buf->base);
}

uv_udp_t client;
uv_udp_send_t send_req;

int idx = 0;
int pid = -1;

char host[100];

void after_send(uv_udp_send_t* req, int status) { free(req->data); }

void timer_cb(uv_timer_t* handle) {
  int r;
  struct sockaddr_in addr;
  char ip[100];

  uv_buf_t buf;
  void* p = malloc(1024);

  get_ip(ip);
  buf = uv_buf_init(p, 1024);
  sprintf(buf.base, "pid:%d host:%s:%s idx:%d", pid, host, ip, ++idx);

  r = uv_ip4_addr("239.255.0.1", 10086, &addr);
  send_req.data = p;

  uv_udp_send(&send_req, &client, &buf, 1, (const struct sockaddr*)&addr,
              after_send);
}

int main(int argc, char* argv[]) {
  int r;
  uv_udp_t server;
  struct sockaddr_in addr;
  uv_timer_t timer;
  int port;
  uv_loop_t* loop;
  size_t len = 0;

  if (argc != 2) {
    port = 10086;
  } else {
    port = atoi(argv[1]);
  }
  printf("multicast %d\n", port);

  pid = getpid();
  gethostname(host, 100);

  loop = uv_default_loop();
  r = uv_ip4_addr("0.0.0.0", port, &addr);
  r = uv_udp_init(loop, &server);
  r = uv_udp_init(uv_default_loop(), &client);
  r = uv_udp_bind(&server, (const struct sockaddr*)&addr, UV_UDP_REUSEADDR);
  r = uv_udp_set_membership(&server, "239.255.0.1", NULL, UV_JOIN_GROUP);

  r = uv_timer_init(loop, &timer);
  r = uv_timer_start(&timer, timer_cb, 0, 1000);
  r = uv_udp_recv_start(&server, alloc_cb, cl_recv_cb);

  uv_run(loop, UV_RUN_DEFAULT);
}
