// src/server_tcp.c
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "protocol.h"   // MENSAJE_MAX_SIZE, mensaje_t, parse(), acknowledge(), verify_data(), TIMEOUT_SEC

static ssize_t read_exact(int fd, uint8_t *buf, size_t n) {
    size_t got = 0;
    while (got < n) {
        ssize_t r = recv(fd, buf + got, n - got, 0);
        if (r == 0) return 0;            // peer closed
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        got += (size_t)r;
    }
    return (ssize_t)got;
}

int main(void) {
    const int SERVER_PORT = 8877;

    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) { perror("socket"); return 1; }

    // Rebind rapide après un redémarrage
    int one = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    // Timeout réception optionnel sur sockets acceptés (appliqué après accept)
    struct timeval tv = { .tv_sec = TIMEOUT_SEC, .tv_usec = 0 };

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons(SERVER_PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind"); close(listen_sock); return 1;
    }
    if (listen(listen_sock, 16) < 0) {
        perror("listen"); close(listen_sock); return 1;
    }

    printf("TCP server listening on 0.0.0.0:%d\n", SERVER_PORT);

    for (;;) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int sock = accept(listen_sock, (struct sockaddr *)&client_address, &client_len);
        if (sock < 0) { perror("accept"); continue; }

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        char ip[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &client_address.sin_addr, ip, sizeof(ip));
        printf("Client connected: %s:%d\n", ip, ntohs(client_address.sin_port));

        // Lire exactement 12 octets (int32 + float32 + float32)
        uint8_t inbuf[MENSAJE_MAX_SIZE];
        ssize_t r = read_exact(sock, inbuf, 12);
        if (r <= 0) {
            if (r == 0) fprintf(stderr, "Client closed before sending data\n");
            else perror("recv");
            close(sock);
            continue;
        }

        mensaje_t msg;
        if (parse(inbuf, (size_t)r, &msg) <= 0) {
            fprintf(stderr, "parse failed\n");
            close(sock);
            continue;
        }

        if (verify_data(&msg)) {
            printf("Received: sensor=%d temp=%.2f hum=%.2f\n",
                   msg.sensor_id, msg.temperatura, msg.humedad);

            uint8_t ack_buf[MENSAJE_MAX_SIZE];
            int ack_len = acknowledge(ack_buf, sizeof(ack_buf)); // doit renvoyer 12
            if (ack_len > 0) {
                ssize_t s = send(sock, ack_buf, (size_t)ack_len, 0);
                if (s != ack_len) perror("send");
            }
        } else {
            fprintf(stderr, "Invalid data: sensor=%d temp=%.2f hum=%.2f\n",
                    msg.sensor_id, msg.temperatura, msg.humedad);
        }

        close(sock);
    }

    close(listen_sock);
    return 0;
}
