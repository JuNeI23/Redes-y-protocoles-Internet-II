// We define the most basic application layer protocol for the discussion beetween the sensor and the server

#include "protocol.h"
#include <string.h>

// returns the message written in binary format in buf
int serialize(const mensaje_t *msg, uint8_t *buf, size_t buflen)
{
    if (buflen < sizeof(mensaje_t))
    {
        return -1;
    }
    // copy the message to the buffer (goes from struct to binary)
    memcpy(buf, msg, sizeof(mensaje_t));
    htonl(msg->sensor_id);
    htonl(msg->temperatura);
    htonl(msg->humedad);
    return sizeof(mensaje_t);
}

// returns the message read from the binary buffer
int parse(const uint8_t *buf, size_t buflen, mensaje_t *msg)
{
    if (buflen < sizeof(mensaje_t))
    {
        return -1;
    }
    // copy the buffer to the message (goes from binary to struct)
    memcpy(msg, buf, sizeof(mensaje_t));
    return sizeof(mensaje_t);
}


