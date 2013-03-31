#ifndef NITRO_SINPROC_H
#define NITRO_SINPROC_H

#include "socket.h"

int Sinproc_socket_connect(nitro_inproc_socket_t *s, char *location);
int Sinproc_socket_bind(nitro_inproc_socket_t *s, char *location);
void Sinproc_socket_bind_listen(nitro_inproc_socket_t *s);
void Sinproc_socket_enable_writes(nitro_inproc_socket_t *s);
void Sinproc_socket_enable_reads(nitro_inproc_socket_t *s);
void Sinproc_socket_start_connect(nitro_inproc_socket_t *s);
void Sinproc_socket_shutdown(nitro_inproc_socket_t *s);
void Sinproc_socket_close(nitro_inproc_socket_t *s);

void Sinproc_socket_send(nitro_inproc_socket_t *s, nitro_frame_t *fr);
nitro_frame_t *Sinproc_socket_recv(nitro_inproc_socket_t *s);
int Sinproc_socket_reply(nitro_inproc_socket_t *s, nitro_frame_t *snd, nitro_frame_t *fr);
int Sinproc_socket_relay_fw(nitro_inproc_socket_t *s, nitro_frame_t *snd, nitro_frame_t *fr);
int Sinproc_socket_relay_bk(nitro_inproc_socket_t *s, nitro_frame_t *snd, nitro_frame_t *fr);

#endif /* NITRO_SINPROC_H */