#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "protocol.h"
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

mensaje_t generate_data() {
	mensaje_t msg;
	// Rellenar el mensaje con datos aleatorios
	msg.sensor_id = 12;
	msg.temperatura = rand();
	msg.humedad = rand();
	return msg;
}

int main() {
	// server address and port
	const char* server_name = "127.0.0.1";
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

	// open socket
	int sock;
	// Set socket timeout option
	struct timeval tv = { .tv_sec = TIMEOUT_SEC, .tv_usec = 0 };

	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));


	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Error en creacion de socket\n");
		return 1;
	}

	// Number of times the message has been resent
	int nb_resend = 0;

	// data that will be sent to the server
	mensaje_t data_to_send = generate_data();

	// serialize the data (transform to byte array)
	uint8_t buffer_send[MENSAJE_MAX_SIZE];

	int serialized_size = serialize(&data_to_send, buffer_send, sizeof(buffer_send));

	// send data
	int len =
	    sendto(sock, buffer_send, serialized_size, 0,
	           (struct sockaddr*)&server_address, sizeof(server_address));

	// received echoed data back
	uint8_t buffer_recieve[100];
	if (recvfrom(sock, buffer_recieve, len, 0, NULL, NULL) < 0) {
		// If no acknowledgment is received within the timeout period, resend the message
		if (nb_resend < MAX_RESEND) {
			printf("No acknowledgment received. Resending message...\n");
			sendto(sock, buffer_send, serialized_size, 0,
			       (struct sockaddr*)&server_address, sizeof(server_address));
			nb_resend++;
		} else {
			printf("No acknowledgment received after %d attempts. Exiting.\n", MAX_RESEND);
			close(sock);
			return 1;
		}
	}
	// parse the data (transform from byte array to struct)
	mensaje_t data_received;
	parse(buffer_recieve, len, &data_received);
	
	// Verify if the received data is an acknowledgment
	if (data_received.sensor_id == ACK_ID && data_received.temperatura == 0.0 && data_received.humedad == 0.0) {
		printf("Acknowledgment received from server.\n");
		close(sock);
		return 0;
	} else while (nb_resend < MAX_RESEND)
	{
		// resend the message if the received data is not an acknowledgment
		sendto(sock, buffer_send, serialized_size, 0,
			       (struct sockaddr*)&server_address, sizeof(server_address));
		nb_resend++;
		printf("Received data is not an acknowledgment. Message resent...\n");
	}
	 {
		printf("Received data is not an acknowledgment. Resending message...\n");
		sendto(sock, buffer_send, serialized_size, 0,
		       (struct sockaddr*)&server_address, sizeof(server_address));
		nb_resend++;
	}

	// close the socket after number of resends
	close(sock);
	printf("No acknowledgment received after %d attempts. Exiting.\n", MAX_RESEND);
	return 1;
}

