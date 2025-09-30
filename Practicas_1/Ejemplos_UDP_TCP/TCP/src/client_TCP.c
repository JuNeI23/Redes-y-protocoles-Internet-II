// src/client_tcp.c
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "protocol.h"  


static float rand_float_range(float min, float max) {
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

static ssize_t read_exact(int fd, uint8_t *buf, size_t n) {
    size_t got = 0;
    while (got < n) {
        ssize_t r = recv(fd, buf + got, n - got, 0);
        if (r == 0) return 0;          
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        got += (size_t)r;
    }
    return (ssize_t)got;
}

static mensaje_t generate_data(void) {
    mensaje_t msg;
    msg.sensor_id   = 12;
    msg.temperatura = rand_float_range(-20.0f, 100.0f);
    msg.humedad     = rand_float_range(0.0f, 100.0f);

    printf("Generated: sensor=%d temp=%.2fC hum=%.2f%%\n",
           msg.sensor_id, msg.temperatura, msg.humedad);
    return msg;
}

int main(void) {
    const char *server_ip  = "127.0.0.1";
    const int   server_port = 8877;

    srand((unsigned)time(NULL));

    // --- socket TCP + connect ---
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

	// Set socket timeout option
    struct timeval tv = { .tv_sec = TIMEOUT_SEC, .tv_usec = 0 };
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt");
        close(sock);
        return 1;
    }

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port   = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &serv.sin_addr) != 1) {
        fprintf(stderr, "inet_pton failed for %s\n", server_ip);
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }


    mensaje_t msg = generate_data();

    uint8_t outbuf[MENSAJE_MAX_SIZE];
    int outlen = serialize(&msg, outbuf, sizeof(outbuf));
    if (outlen <= 0) {
        fprintf(stderr, "serialize failed\n");
        close(sock);
        return 1;
    }

    ssize_t sent = send(sock, outbuf, (size_t)outlen, 0);
    if (sent != outlen) {
        perror("send");
        close(sock);
        return 1;
    }
    printf("Sent %d bytes to %s:%d\n", outlen, server_ip, server_port);


    uint8_t inbuf[MENSAJE_MAX_SIZE];
    ssize_t r = read_exact(sock, inbuf, 12); 
    if (r <= 0) {
        if (r == 0) fprintf(stderr, "server closed connection\n");
        else perror("recv");
        close(sock);
        return 1;
    }

    mensaje_t ack;
    if (parse(inbuf, (size_t)r, &ack) <= 0) {
        fprintf(stderr, "parse(ACK) failed\n");
        close(sock);
        return 1;
    }

    if (ack.sensor_id == ACK_ID && ack.temperatura == 0.0f && ack.humedad == 0.0f) {
        printf("ACK received from server\n");
        close(sock);
        return 0;
    } else {
        printf("Unexpected response: sensor=%d temp=%.2f hum=%.2f\n",
               ack.sensor_id, ack.temperatura, ack.humedad);
        close(sock);
        return 1;
    }
}
