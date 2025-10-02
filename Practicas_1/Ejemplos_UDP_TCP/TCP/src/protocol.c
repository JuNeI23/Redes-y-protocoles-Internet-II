// We define the most basic application layer protocol for the discussion beetween the sensor and the server

#include "protocol.h"
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>






// returns the message written in binary format in buf
int serialize(const mensaje_t *msg, uint8_t *buf, size_t buflen)
{
    if (!msg || !buf || buflen < 12) return -1;

    // int
    uint32_t sid_be = htonl((uint32_t)msg->sensor_id);
    memcpy(buf + 0, &sid_be, 4);

    // float temperatura
    uint32_t tmp;
    memcpy(&tmp, &msg->temperatura, 4);  // copier les bits
    tmp = htonl(tmp);
    memcpy(buf + 4, &tmp, 4);

    // float humedad
    memcpy(&tmp, &msg->humedad, 4);
    tmp = htonl(tmp);
    memcpy(buf + 8, &tmp, 4);

    return 12;
}

int parse(const uint8_t *buf, size_t buflen, mensaje_t *msg)
{
    if (!buf || !msg || buflen < 12) return -1;

    // int
    uint32_t sid_be;
    memcpy(&sid_be, buf + 0, 4);
    msg->sensor_id = (int32_t)ntohl(sid_be);

    // float temperatura
    uint32_t tmp;
    memcpy(&tmp, buf + 4, 4);
    tmp = ntohl(tmp);
    memcpy(&msg->temperatura, &tmp, 4);

    // float humedad
    memcpy(&tmp, buf + 8, 4);
    tmp = ntohl(tmp);
    memcpy(&msg->humedad, &tmp, 4);

    return 12;
}



// Create an acknowledgment message inside a provided buffer where the ID is ACK_ID and the other fields are zeroed
int acknowledge(uint8_t *buf, size_t buflen)
{
    if (buflen < sizeof(mensaje_t))
    {
        return -1;
    }
    // create the acknowledgment message
    mensaje_t ack_msg;
    ack_msg.sensor_id = ACK_ID;
    ack_msg.temperatura = 0.0;
    ack_msg.humedad = 0.0;

    // serialize the acknowledgment message into the provided buffer
    if(serialize(&ack_msg, (uint8_t *)buf, buflen)){
        return 0;
    }
    return -1;
}


int verify_data(const mensaje_t *msg) {
    // the ID 1 is reserved for the server
    if (msg->sensor_id < 2 || msg->sensor_id > 1000) {
        return -1; // Invalid sensor ID
    }
    return 0; // Data is valid
}

int debug_print_msg(const uint8_t *buf, size_t len) {
    printf("Buffer dump (%zu bytes):\n", len);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", buf[i]);   // hexadécimal sur 2 chiffres
        if ((i + 1) % 16 == 0) {   // retour à la ligne tous les 16 octets
            printf("\n");
        }
    }
    if (len % 16 != 0) {
        printf("\n");
    }
    return 0;
}