#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "protocol.h"




/**
 * TCP Uses 2 types of sockets, the connection socket and the listen socket.
 * The Goal is to separate the connection phase from the data exchange phase.
 * */

int main(int argc, char *argv[]) {
	// port to start the server on
	int SERVER_PORT = 8877;

	// socket address used for the server
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	printf("Starting server on port %d\n", SERVER_PORT);
	// htons: host to network short: transforms a value in host byte
	// ordering format to a short value in network byte ordering format
	server_address.sin_port = htons(SERVER_PORT);

	// htonl: host to network long: same as htons but to long
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	printf("Creating listening socket...\n");
	// create a TCP socket, creation returns -1 on failure
	int listen_sock;
	if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error en creacion de socket de escucha\n");
		return 1;
	}

	// bind it to listen to the incoming connections on the created server
	// address, will return -1 on error
	if ((bind(listen_sock, (struct sockaddr *)&server_address,
	          sizeof(server_address))) < 0) {
		printf("Error en bind\n");
		close(listen_sock);
		return 1;
	}

	int wait_size = 16;  // maximum number of waiting clients, after which
	                     // dropping begins
	if (listen(listen_sock, wait_size) < 0) {
		printf("Error en listen\n");
		close(listen_sock);
		return 1;
	}

	// socket address used to store client address
	struct sockaddr_in client_address;
	socklen_t client_address_len = sizeof(client_address);

	// run indefinitely
	while (true) {
		// open a new socket to transmit data per connection
		int sock;
		if ((sock =
		         accept(listen_sock, (struct sockaddr *)&client_address,
		                &client_address_len)) < 0) {
			printf("Error en apertura de socket\n");
			close(sock);
			close(listen_sock);
			return 1;
		}


		int n = 0;
		// length of message recieved
		int len = 0, maxlen = 100;
		uint8_t buffer[12];
		uint8_t *pbuffer = buffer;

		printf("Cliente conectado con IP:  %s\n",
		       inet_ntoa(client_address.sin_addr));
		
		printf("Esperando mensajes...\n");



		n = recv(sock, buffer, 12, 0);
		if (n < 0) {
			printf("Error recibiendo datos\n");
			close(sock);
			close(listen_sock);
			return -1;
		}

		// parse the recieved message
		mensaje_t message_recieved;
		if(parse(pbuffer,len,&message_recieved)){
			printf("Error parsing the message\n");
			close(sock);
			close(listen_sock);
			return -1;
		};

		// verify the content of the message recieved

		if (verify_data(&message_recieved) == 0){
			printf("Message recieved : Sensor %d: Temperatura %.2f, Humedad %.2f\n",
					message_recieved.sensor_id, message_recieved.temperatura,
					message_recieved.humedad);
			uint8_t buffer_back[MENSAJE_MAX_SIZE];
			if (acknowledge(buffer_back,MENSAJE_MAX_SIZE)){
				printf("sending acknowledgement....\n");
				send(sock,buffer_back,MENSAJE_MAX_SIZE,0);
				printf("acknowledgement sent\n");
			} else {
				printf("There was a problem with sending the ACK..\n");
				close(sock);
				close(listen_sock);
				return -1;
			}
		} else {
			printf("message corrupted, this message was sent : Sensor %d: Temperatura %.2f, Humedad %.2f, closing the sockets...\n",
					message_recieved.sensor_id, message_recieved.temperatura,
					message_recieved.humedad);
			close(sock);
			close(listen_sock);
			return -1;
		}
		printf("Cliente desconectado\n");
		close(sock);
		break;
	}
	printf("Shutting down the server\n");
	close(listen_sock);
	return 0;
}
