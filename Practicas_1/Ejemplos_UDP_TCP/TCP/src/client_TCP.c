#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "protocol.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


static float rand_float_range(float min, float max) {
	float scale = rand() / (float) RAND_MAX; // [0, 1.0]
	return min + scale * (max - min);        // [min, max]
}


mensaje_t generate_data() {
	mensaje_t msg;
	// Rellenar el mensaje con datos aleatorios
	msg.sensor_id = 12;

	
	// Seed the random number generator beetween -20 and 50 for temperature and 0 to 100 for humidity
	srand((unsigned)time(NULL));  

    for (int i = 0; i < 10; i++) {
        msg.temperatura = rand_float_range(-20.0f, 100.0f);
		msg.humedad = rand_float_range(0.0f, 100.0f);
    }

	printf("Generated data: Sensor ID %d, Temperature %.2f, Humidity %.2f\n",
	       msg.sensor_id, msg.temperatura, msg.humedad);
	return msg;
}


int main() {
	const char* server_name = "localhost";
	const int server_port = 8877;

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// creates binary representation of server name
	// and stores it as sin_addr
	// http://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html
	inet_pton(AF_INET, server_name, &server_address.sin_addr);

	// htons: port in network order format
	server_address.sin_port = htons(server_port);





	// open a stream socket
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error en creacion de socket\n");
		return 1;
	}

	// Set socket timeout option
	struct timeval tv = { .tv_sec = TIMEOUT_SEC, .tv_usec = 0 };


	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	// Number of times the message has been resent
	int nb_resend = 0;

	// data that will be sent to the server
	mensaje_t data_to_send = generate_data();

	// TCP is connection oriented, a reliable connection
	// **must** be established before any data is exchanged
	if (connect(sock, (struct sockaddr*)&server_address,
	            sizeof(server_address)) < 0) {
		printf("Error en conexion a servidor\n");
		return 1;
	}

	// serialise the data
	uint8_t buffer_send[MENSAJE_MAX_SIZE];
	int serialised = serialize(&data_to_send, buffer_send, sizeof(buffer_send));


	// data that will be sent to the server
	send(sock, buffer_send, sizeof(buffer_send), 0);

	// receive

	int n = 0;
	int len = 0, maxlen = 100;
	char buffer[maxlen];
	char* pbuffer = buffer;

	// will remain open until the server terminates the connection
	while ((n = recv(sock, pbuffer, maxlen, 0)) > 0) {
		pbuffer += n;
		maxlen -= n;
		len += n;

		buffer[len] = '\0';
		printf("Recibido: '%s'\n", buffer);
	}

	// close the socket
	close(sock);
	return 0;
}
