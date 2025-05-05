#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define OSInit() WSADATA wsaData; WSAStartup(MAKEWORD(2, 0), &wsaData)
#define OSCleanup() WSACleanup()
#define close(s) closesocket(s)
#define perror(msg) fprintf(stderr, msg ": WSA errno = %d\n", WSAGetLastError())
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/select.h>
#define OSInit()
#define OSCleanup()
#endif

#define PORT "24042"
#define MAX_BUFFER 1000
#define MAX_CLIENTS 100

typedef struct {
    struct sockaddr_storage addr;
    socklen_t addrlen;
    int guess;
} ClientGuess;

int create_udp_socket() {
    struct addrinfo hints, *res;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    freeaddrinfo(res);
    return sockfd;
}

int generate_secret_number() {
    return rand() % 100;
}

int compare_clients(const void *a, const void *b) {
    return ((ClientGuess *)a)->guess - ((ClientGuess *)b)->guess;
}

void play_round(int sockfd) {
    int secret = generate_secret_number();
    printf("\nNieuw te raden getal: %d\n", secret);

    ClientGuess clients[MAX_CLIENTS];
    int client_count = 0;
    int timeout_sec = 16;
    int closest_diff = 100;
    int winner_index = -1;

    while (timeout_sec > 0) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        struct timeval timeout = { timeout_sec, 0 };

        int result = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (result < 0) {
            perror("select");
            break;
        } else if (result == 0) {
            break;  // timeout verlopen
        }

        // Receive new guess
        struct sockaddr_storage addr;
        socklen_t addrlen = sizeof(addr);
        char buffer[MAX_BUFFER];
        int bytes = recvfrom(sockfd, buffer, MAX_BUFFER - 1, 0, (struct sockaddr*)&addr, &addrlen);
        if (bytes < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[bytes] = '\0';
        int guess = atoi(buffer);
        printf("Gok ontvangen: %d\n", guess);

        if (client_count < MAX_CLIENTS) {
            clients[client_count].addr = addr;
            clients[client_count].addrlen = addrlen;
            clients[client_count].guess = guess;

            int diff = abs(secret - guess);
            if (diff < closest_diff) {
                closest_diff = diff;
                winner_index = client_count;
            }

            client_count++;
        }

        timeout_sec /= 2;
    }

    // Stuur resultaten
    for (int i = 0; i < client_count; i++) {
        const char *message = (i == winner_index) ? "You won!" : "You lost!";
        sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr*)&clients[i].addr, clients[i].addrlen);
    }

    if (winner_index != -1) {
        printf("Winnaar is: %d\n", clients[winner_index].guess);
    } else {
        printf("Geen winnaar deze ronde.\n");
    }

    // Late messages afhandelen
    struct timeval grace = { 2, 0 };
    fd_set late_fds;
    FD_ZERO(&late_fds);
    FD_SET(sockfd, &late_fds);

    while (select(sockfd + 1, &late_fds, NULL, NULL, &grace) > 0) {
        struct sockaddr_storage late_addr;
        socklen_t len = sizeof(late_addr);
        char buf[MAX_BUFFER];
        int bytes = recvfrom(sockfd, buf, MAX_BUFFER - 1, 0, (struct sockaddr*)&late_addr, &len);
        if (bytes > 0) {
            const char *late_msg = "You lost!";
            sendto(sockfd, late_msg, strlen(late_msg), 0, (struct sockaddr*)&late_addr, len);
        }

        FD_ZERO(&late_fds);
        FD_SET(sockfd, &late_fds);
    }
}

int main() {
    OSInit();
    srand((unsigned int)time(NULL));

    int sockfd = create_udp_socket();
    printf("UDP-server gestart op poort %s\n", PORT);

    while (1) {
        play_round(sockfd);
        printf("Wachten op nieuwe ronde...\n");
    }

    close(sockfd);
    OSCleanup();
    return 0;
}
