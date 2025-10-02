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

    
	msg.temperatura = rand_float_range(-20.0f, 100.0f);
	msg.humedad = rand_float_range(0.0f, 100.0f);


	printf("Generated data: Sensor ID %d, Temperature %.2f, Humidity %.2f\n",
	       msg.sensor_id, msg.temperatura, msg.humedad);
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

	

	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Error en creacion de socket\n");
		return 1;
	}

	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	// Number of times the message has been resent
	int nb_resend = 0;

	// data that will be sent to the server
	mensaje_t data_to_send = generate_data();

	// serialize the data (transform to byte array)
	uint8_t buffer_send[MENSAJE_MAX_SIZE];

	int serialized_size = serialize(&data_to_send, buffer_send, sizeof(buffer_send));

	// send data
	printf("Sending data to server %s : \n", server_name);
	int len =
	    sendto(sock, buffer_send, serialized_size, 0,
	           (struct sockaddr*)&server_address, sizeof(server_address));
	
	printf("data sent \n");

	bool NO_MESSAGE_RECEIVED = true;

	// received echoed data back
	uint8_t buffer_recieve[100];
	while (nb_resend < MAX_RESEND && NO_MESSAGE_RECEIVED) {
		wait(0.01); // wait 0.01 seconds before trying to receive
		if (recvfrom(sock, buffer_recieve, len, 0, NULL, NULL) < 0) {
			// If no acknowledgment is received within the timeout period, resend the message
			if (nb_resend < MAX_RESEND) {
				wait(1);
				printf("No acknowledgment received. Resending message...\n");
				sendto(sock, buffer_send, serialized_size, 0,
					(struct sockaddr*)&server_address, sizeof(server_address));
				nb_resend++;
			} else {
				printf("No acknowledgment received after %d attempts. Exiting.\n", MAX_RESEND);
				close(sock);
				return 1;
			}
		} else {
			NO_MESSAGE_RECEIVED = false;
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

	// close the socket after number of resends
	close(sock);
	printf("No acknowledgment received after %d attempts. Exiting.\n", MAX_RESEND);
	return 1;
}

