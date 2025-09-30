// We define the most basic application layer protocol for the discussion beetween the sensor and the server


#include <stdint.h>
#include <stdlib.h>
typedef struct mensaje
{
    //cada sensor tiene un id unico
    int sensor_id;

    // los sensores envian su temperatura y humedad
    float temperatura;
    float humedad;
} mensaje_t;

// arbitrary limit for the maximum size of a serialized mensaje_t
#define MENSAJE_MAX_SIZE 12


// Only the server will send back an acknowledgment message with this ID
#define ACK_ID 1 

// A sensor will resend the message up to MAX_RESEND times if no acknowledgment is received
#define MAX_RESEND 3

// A response is expected within TIMEOUT_SEC seconds
#define TIMEOUT_SEC 10


// These functions serialize and parse the mensaje_t structure to/from a byte buffer to avoid losing data due to padding or alignment issues
int serialize(const mensaje_t *msg, uint8_t *buf, size_t buflen);
int parse(const uint8_t *buf, size_t buflen, mensaje_t *msg);
int acknowledge(uint8_t *buf, size_t buflen);
int verify_data(const mensaje_t *msg);

