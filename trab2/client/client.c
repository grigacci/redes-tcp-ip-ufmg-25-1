#include "client.h" // Assuming this contains your includes and constants

// Forward declarations for your helper functions
void generate_random_string(char *s, const int len);
int tcp_socket(int* s, struct sockaddr_in sin);
int tcp_send(int s, const char* buf, int len);
int tcp_receive(int s, char* buf, int buf_size);
int udp_receive(int s, struct sockaddr_in sin, char* buf, int buf_size);


int main(int argc, char* argv[]) {
    FILE* fp;
    struct hostent* hp;
    struct sockaddr_in sin;
    char* host;
    int msg_size = 8;
    int s;
    int len;
    char* protocol;
    int i = 0;
    int sent_messages = 0;
    int received_messages = 0;
    CTimer timer;
    double elapsed_time;
    double total_time = 0.0;
    struct timeval timeout;
    timeout.tv_sec = 2; // 2 seconds
    timeout.tv_usec = 0; // 0 microseconds

    // Seed the random number generator once at the beginning
    srand((unsigned int)time(NULL));

    if (argc == 4) {
        host = argv[1];
        protocol = argv[2];
        msg_size = atoi(argv[3]);
        while (protocol[i] != '\0') {
            protocol[i] = toupper(protocol[i]);
            i++;
        }
    } else {
        fprintf(stderr, "usage: simplex-talk host [TCP|UDP] [message_size]\n");
        exit(1);
    }

    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
    }

    // Using malloc for the buffer is safer for large sizes to avoid stack overflow
    char* buf = (char*)malloc(msg_size + 1);
    if (!buf) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        exit(1);
    }
    
    bzero((char*)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr_list[0], (char*)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);
    fprintf(stdout, "simplex-talk: sending to %s port %d\n", host,
            ntohs(sin.sin_port));

    generate_random_string(buf, msg_size);

    if (strcmp(protocol, "TCP") == 0) {
        fprintf(stdout, "simplex-talk: using TCP protocol\n");

        if (tcp_socket(&s, sin) < 0) {
            fprintf(stderr, "simplex-talk: error creating TCP socket\n");
            free(buf);
            exit(1);
        }

        while (sent_messages < MAX_MESSSAGES) {
            start_timer(&timer);
            
            // Send the full message
            if (tcp_send(s, buf, msg_size) < 0) {
                fprintf(stderr, "Failed to send message\n");
                break;
            }

            // Receive the full message back
            if (tcp_receive(s, buf, msg_size) <= 0) {
                fprintf(stderr, "Failed to receive message or connection closed\n");
                break;
            }

            stop_timer(&timer);
            elapsed_time = get_elapsed_time_microseconds(&timer);
            sent_messages++;
            total_time += elapsed_time;
            fflush(stdout);
        }

        fprintf(stdout, "Sent %d messages, with size %d bytes using TCP protocol\n", sent_messages, msg_size);
        if (sent_messages > 0) {
            fprintf(stdout, "Total Time: %.2f [s] | Avg latency: %.2f [us] | Avg Throughput: %.2f [bps] / %.2f [kbps]\n",
                    total_time / 1000000.0, total_time / sent_messages, sent_messages * msg_size * 8 /
                    (total_time / 1000000.0), sent_messages * msg_size * 8 /
                    (total_time / 1000.0));
        }

    } else if (strcmp(protocol, "UDP") == 0) {
        fprintf(stdout, "simplex-talk: using UDP protocol\n");

        if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("simplex-talk: socket");
            free(buf);
            exit(1);
        }

        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("ERROR: package timeout");
            free(buf);
            exit(1);
        }

        while (sent_messages < MAX_MESSSAGES) {
            start_timer(&timer);
            sendto(s, buf, msg_size, 0, (struct sockaddr*)&sin, sizeof(sin));
            sent_messages++;
            if(udp_receive(s, sin, buf, msg_size + 1) > 0) {
                stop_timer(&timer);
                elapsed_time = get_elapsed_time_microseconds(&timer);
                received_messages++;
                total_time += elapsed_time;
                fflush(stdout);
            } else {      
                fprintf(stdout, "> ERROR: Package %d timeout\n", sent_messages);
                fflush(stdout);
            }
        }
        fprintf(stdout, "Sent %d messages, with size %d bytes using UDP protocol | Packet loss: %.2f%%\n", sent_messages, msg_size, (1.0 - (double)received_messages / sent_messages) * 100.0);
        if (received_messages > 0) {
            fprintf(stdout, "Total Time: %.2f [s] | Avg latency: %.2f [us] | Avg Throughput: %.2f [bps] / %.2f [kbps]\n",
                    total_time / 1000000.0, total_time / received_messages, received_messages * msg_size * 8 /
                    (total_time / 1000000.0), received_messages * msg_size * 8 /
                    (total_time / 1000.0));
        }
    } 

    close(s);
    free(buf); // Don't forget to free the allocated memory
    return 0;
}

// +-----------------------------------------------------+
// |                  HELPER FUNCTIONS                   |
// +-----------------------------------------------------+

int tcp_socket(int* s, struct sockaddr_in sin) {
    fprintf(stdout, "simplex-talk: creating TCP socket\n");
    if ((*s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        return -1;
    }

    fprintf(stdout, "simplex-talk: connecting to server\n");
    if (connect(*s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("simplex-talk: connect");
        close(*s);
        return -1;
    }
    fprintf(stdout, "simplex-talk: connected to server\n");
    return *s;
}

// CORRECTED sending function
int tcp_send(int s, const char* buf, int len) {
    ssize_t bytes_sent;
    int total_bytes_sent = 0;

    while (total_bytes_sent < len) {
        bytes_sent = send(s, buf + total_bytes_sent, len - total_bytes_sent, 0);
        if (bytes_sent < 0) {
            perror("simplex-talk: send error");
            return -1;
        }
        if (bytes_sent == 0) {
            fprintf(stderr, "simplex-talk: connection closed by peer during send\n");
            return -1;
        }
        total_bytes_sent += bytes_sent;
    }
    return total_bytes_sent;
}

// CORRECTED receiving function
int tcp_receive(int s, char* buf, int buf_size) {
    ssize_t bytes_received;
    int total_bytes_received = 0;

    while (total_bytes_received < buf_size) {
        bytes_received = recv(s, buf + total_bytes_received, buf_size - total_bytes_received, 0);

        if (bytes_received < 0) {
            perror("simplex-talk: recv error");
            return -1;
        }
        if (bytes_received == 0) {
            fprintf(stderr, "simplex-talk: connection closed by peer\n");
            return 0;
        }
        total_bytes_received += bytes_received;
    }

    buf[total_bytes_received] = '\0';
    return total_bytes_received;
}

void generate_random_string(char *s, const int len) {
    if (len <= 0) {
        if (s) s[0] = '\0';
        return;
    }
    // This function can generate null bytes, which is fine for fixed-length
    // transfers but would be a problem for string functions like strlen.
    for (int i = 0; i < len; ++i) {
        s[i] = (char)(rand() % 256);
    }
    s[len] = '\0';
}

int udp_receive(int s, struct sockaddr_in sin, char* buf, int buf_size) {
    socklen_t sin_len = sizeof(sin);
    ssize_t bytes_received = recvfrom(s, buf, buf_size, 0, (struct sockaddr*)&sin, &sin_len);
    
    if (bytes_received >= 0) {
      buf[bytes_received] = '\0';
    }

    return bytes_received;
}