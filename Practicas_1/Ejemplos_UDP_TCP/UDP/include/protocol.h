// We define the most basic application layer protocol for the discussion beetween the sensor and the server


#include <stdint.h>
#include <stdlib.h>
typedef struct mensaje
{
    //cada sensor tiene un id unico
    char sensor_id[16];

    // los sensores envian su temperatura y humedad
    float temperatura;
    float humedad;
} mensaje_t;

// arbitrary limit for the maximum size of a serialized mensaje_t
#define MENSAJE__MAX_SIZE 512;

// These functions serialize and parse the mensaje_t structure to/from a byte buffer to avoid losing data due to padding or alignment issues
int proto_serialize(const mensaje_t *msg, uint8_t *buf, size_t buflen);
int proto_parse(const uint8_t *buf, size_t buflen, mensaje_t *msg);

