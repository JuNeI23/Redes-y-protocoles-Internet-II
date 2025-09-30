#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "protocol.h"

int main(int argc, char *argv[]) {


	// port to start the server on
	int SERVER_PORT = 8877;

	// socket address used for the server
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// htons: host to network short: transforms a value in host byte
	// ordering format to a short value in network byte ordering format
	server_address.sin_port = htons(SERVER_PORT);

	// htons: host to network long: same as htons but to long
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	// create a UDP socket, creation returns -1 on failure
	int sock;
	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Error en creacion de socket\n");
		return 1;
	}

	// bind it to listen to the incoming connections on the created server
	// address, will return -1 on error
	if ((bind(sock, (struct sockaddr *)&server_address,
	          sizeof(server_address))) < 0) {
		printf("Error en bind\n");
		return 1;
	}

	// socket address used to store client address
	struct sockaddr_in client_address;
	socklen_t client_address_len = sizeof(client_address);

	// run indefinitely
	while (true) {
		uint8_t buffer[MENSAJE_MAX_SIZE];

		// read content into buffer from an incoming client
		int len = recvfrom(sock, buffer, sizeof(buffer), 0,
		                   (struct sockaddr *)&client_address,
		                   &client_address_len);

		mensaje_t msg_recieved;
		parse(buffer, len, &msg_recieved);
		if (verify_data(&msg_recieved)) {
			printf("Recibido datos del cliente %s : ",
		       inet_ntoa(client_address.sin_addr));
			printf("Sensor %d: Temperatura %.2f, Humedad %.2f\n",
			       msg_recieved.sensor_id, msg_recieved.temperatura,
			       msg_recieved.humedad);

			// send acknowledgment back to the client
			uint8_t ack_buf[MENSAJE_MAX_SIZE];
			acknowledge(ack_buf, sizeof(ack_buf));
			sendto(sock, ack_buf, sizeof(ack_buf), 0, (struct sockaddr *)&client_address,
			       sizeof(client_address));
		} else {
			printf("Datos corruptos recibidos : ID %d, Temperatura %.2f, Humedad %.2f\n",
			       msg_recieved.sensor_id, msg_recieved.temperatura,
			       msg_recieved.humedad);
		}
	}

	return 0;
}
