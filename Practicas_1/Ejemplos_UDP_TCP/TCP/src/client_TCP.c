#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "protocol.h"
#include <time.h>

static float rand_float_range(float min, float max) {
	float scale = rand() / (float) RAND_MAX; // [0, 1.0]
	return min + scale * (max - min);        // [min, max]
}

mensaje_t generate_debug_data() {
	mensaje_t msg;
	// Rellenar el mensaje con datos aleatorios
	msg.sensor_id = 2;
	msg.temperatura = 3.0;
	msg.humedad = 4.0;

	printf("Generated data: Sensor ID %d, Temperature %.2f, Humidity %.2f\n",
	       msg.sensor_id, msg.temperatura, msg.humedad);
	return msg;
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

	// TCP is connection oriented, a reliable connection
	// **must** be established before any data is exchanged
	if (connect(sock, (struct sockaddr*)&server_address,
	            sizeof(server_address)) < 0) {
		printf("Error en conexion a servidor\n");
		return 1;
	}

	// send

	// data that will be sent to the server
	mensaje_t message_send = generate_debug_data();
	uint8_t buffer_send[MENSAJE_MAX_SIZE];

	if(serialize(&message_send, buffer_send, MENSAJE_MAX_SIZE) == 12) {
		printf("Sending data to the server...\n");
		send(sock, buffer_send, sizeof(buffer_send), 0);
		printf("Data sent\n");
	}

	// receive

	int n = 0;
	int len = 0, maxlen = 100;
	uint8_t buffer_recieve[maxlen];
	uint8_t* pbuffer = buffer_recieve;
	printf("Waiting for the server to acknowledge the data...\n");
	// will remain open until the server terminates the connection
	while ((n = recv(sock, pbuffer, maxlen, 0)) > 0) {
		pbuffer += n;
		maxlen -= n;
		len += n;

		if (debug_print_msg(buffer_recieve, len) != 0) {
			printf("Error printing the received message\n");
			return -1;
		}
		mensaje_t message_recieved;
		printf("Data recieved from server \n");
		if(parse(buffer_recieve,MENSAJE_MAX_SIZE,&message_recieved) == 12){
			if(message_recieved.sensor_id == 1){
				// the acknowledgement is correct
				printf("The server has acknowledged receiving the data, exiting... \n");
				close(sock);
				return 0;
			} else {
				printf("Here is the data received from the server: Sensor ID %d, Temperature %.2f, Humidity %.2f\n",
				       message_recieved.sensor_id, message_recieved.temperatura,
				       message_recieved.humedad);
				printf("The server has not acknowledged receiving the data correctly, exiting... \n");
				close(sock);
				return -1;
			} 
		} else {
			printf("Error parsing the message\n");
			return -1;
		}
	}
	printf("Closing the socket\n");
	// close the socket
	close(sock);
	return 0;
}
