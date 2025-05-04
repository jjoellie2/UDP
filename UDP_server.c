#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <unistd.h> // voor sleep functie
#define OSInit() WSADATA wsaData; WSAStartup( MAKEWORD( 2, 0 ), &wsaData );
#define OSCleanup() WSACleanup();
#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#define OSInit()
#define OSCleanup()
#endif

int generate_random_number() {
    srand(time(NULL)); // initialiseer random getal generator
    return rand() % 100; // genereer een random getal tussen 0 en 99
}

void compare_guess_with_number(int guess, int number_to_guess, int client_socket, struct sockaddr *client_internet_address, socklen_t client_internet_address_length) {
    if (guess == number_to_guess) {
        printf("Guess is correct! You won!\n");
        sendto(client_socket, "You won!", strlen("You won!"), 0, client_internet_address, client_internet_address_length);
    } else {
        printf("Guess is incorrect! The correct number was: %d\n", number_to_guess);
        sendto(client_socket, "You lost. The correct number was: ", strlen("You lost. The correct number was: "), 0, client_internet_address, client_internet_address_length);
    }
}

int initialization();
void execution(int internet_socket);
void cleanup(int internet_socket);

int main(int argc, char *argv[]) {
    OSInit();

    int internet_socket = initialization();

    while (1) {
        execution(internet_socket);
    }

    cleanup(internet_socket);

    OSCleanup();

    return 0;
}

int initialization() {
    // Step 1.1
    struct addrinfo internet_address_setup;
    struct addrinfo *internet_address_result;
    memset(&internet_address_setup, 0, sizeof internet_address_setup);
    internet_address_setup.ai_family = AF_UNSPEC;
    internet_address_setup.ai_socktype = SOCK_DGRAM;
    internet_address_setup.ai_flags = AI_PASSIVE;
    int getaddrinfo_return = getaddrinfo(NULL, "24042", &internet_address_setup, &internet_address_result);
    if (getaddrinfo_return != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return));
        exit(1);
    }

    int internet_socket = -1;
    struct addrinfo *internet_address_result_iterator = internet_address_result;
    while (internet_address_result_iterator != NULL) {
        // Step 1.2
        internet_socket = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol);
        if (internet_socket == -1) {
            perror("socket");
        } else {
            // Step 1.3
            int bind_return = bind(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);
            if (bind_return == -1) {
                close(internet_socket);
                perror("bind");
            } else {
                break;
            }
        }
        internet_address_result_iterator = internet_address_result_iterator->ai_next;
    }

    freeaddrinfo(internet_address_result);

    if (internet_socket == -1) {
        fprintf(stderr, "socket: no valid socket address found\n");
        exit(2);
    }

    return internet_socket;
}

void execution(int internet_socket) {
    int number_to_guess = generate_random_number(); // genereer het te raden getal
    printf("The number to guess is: %d\n", number_to_guess);

    // Set timeout starting from 16 seconds
    struct timeval timeout;
    timeout.tv_sec = 16;
    timeout.tv_usec = 0;
    if (setsockopt(internet_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
    }

    int closest_guess = -1;
    int closest_distance = 100; // Begin met een hoge waarde
    int winner_selected = 0; // Variabele om bij te houden of er al een winnaar is geselecteerd

    while (!winner_selected) {
        int number_of_bytes_received = 0;
        char buffer[1000];
        struct sockaddr_storage client_internet_address;
        socklen_t client_internet_address_length = sizeof(client_internet_address);
        number_of_bytes_received = recvfrom(internet_socket, buffer, (sizeof buffer) - 1, 0, (struct sockaddr *)&client_internet_address, &client_internet_address_length);
        if (number_of_bytes_received == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) { // Timeout occurred
                if (closest_guess != -1) {
                    printf("You Won?\n");
                    sendto(internet_socket, "You won!", strlen("You won!"), 0, (struct sockaddr *)&client_internet_address, client_internet_address_length);
                    winner_selected = 1; // Selecteer de winnaar
                } else {
                    printf("You lost?\n");
                    sendto(internet_socket, "You lost. The correct number was: ", strlen("You lost. The correct number was: "), 0, (struct sockaddr *)&client_internet_address, client_internet_address_length);
                    break;
                }
            } else {
                perror("recvfrom");
            }
        } else {
            buffer[number_of_bytes_received] = '\0';
            int client_guess = atoi(buffer); // converteer ontvangen gok naar integer
            printf("Received guess from client: %d\n", client_guess);
            int distance = abs(client_guess - number_to_guess); // Afstand tot het te raden getal
            if (distance < closest_distance) {
                closest_guess = client_guess;
                closest_distance = distance;
            }

            compare_guess_with_number(client_guess, number_to_guess, internet_socket, (struct sockaddr *)&client_internet_address, client_internet_address_length); // vergelijk gok met te raden getal

            // Halveer de timeout
            timeout.tv_sec /= 2;
            if (setsockopt(internet_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
                perror("setsockopt");
            }
        }
    }
}

void cleanup(int internet_socket) {
    // Step 3.1
    close(internet_socket);
}
