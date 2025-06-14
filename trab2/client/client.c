#include "client.h"

int main(int argc, char* argv[]) {
  FILE* fp;
  struct hostent* hp;
  struct sockaddr_in sin;
  char* host;
  char buf[MAX_LINE];
  int s;
  int len;
  char* protocol;
  int i = 0;
  int sent_messages = 0;
  CTimer timer;
  double elapsed_time;
  double total_time = 0.0;

  if (argc == 3) {
    host = argv[1];
    protocol = argv[2];
    while (protocol[i] != '\0') {
      protocol[i] = toupper(protocol[i]);
      i++;
    }
  } else {
    fprintf(stderr, "usage: simplex-talk host\n");
    exit(1);
  }

  /*traduz nome do host para endereço IP do host */
  hp = gethostbyname(host);

  if (!hp) {
    fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
    exit(1);
  }

  /*monta estrutura de dados do endereço */
  bzero((char*)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr_list[0], (char*)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(SERVER_PORT);
  fprintf(stdout, "simplex-talk: sending to %s port %d\n", host,
          ntohs(sin.sin_port));

  if (strcmp(protocol, "TCP") == 0) {
    fprintf(stdout, "simplex-talk: using TCP protocol\n");

    if (tcp_socket(&s, sin) < 0) {
        fprintf(stderr, "simplex-talk: error creating TCP socket\n");
        exit(1);
      }

    /*laço de envio de mensagens aleatórias */
    while (sent_messages < MAX_MESSSAGES) {
      generate_random_string(buf, MESSAGE_SIZE - 1);
      start_timer(&timer);
      tcp_send(s, buf, MESSAGE_SIZE);
      tcp_receive(s, buf, sizeof(buf));
      stop_timer(&timer);
      elapsed_time = get_elapsed_time_microseconds(&timer);
      fprintf(stdout, "> %d random bytes sent | Latency = %.4f us | seq = %d\n",
              MESSAGE_SIZE, elapsed_time, sent_messages + 1);
      sent_messages++;
      total_time += elapsed_time;
      fflush(stdout);
    }
    fprintf(stdout, "Sent and received %d messages\n", sent_messages);
    fprintf(stdout, "Total Time: %.2f [s] | Avg latency: %.2f [us] | Avg Throughput: %.2f [bps] / %.2f [kbps]\n",
            total_time / 1000000.0, total_time / sent_messages, sent_messages * MESSAGE_SIZE * 8 /
            (total_time / 1000000.0), sent_messages * MESSAGE_SIZE * 8 /
            (total_time / 1000.0));
    close(s);
  } else if (strcmp(protocol, "UDP") == 0) {
    fprintf(stdout, "simplex-talk: using UDP protocol\n");

    /* Create UDP socket */
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("simplex-talk: socket");
      exit(1);
    }

    fprintf(stdout, "Waiting for input ...\n");
    fflush(stdout);

    while (fgets(buf, sizeof(buf), stdin)) {
      buf[MAX_LINE - 1] = '\0';
      len = strlen(buf) + 1;

      /* Send message to server using sendto() */
      ssize_t bytes_sent =
          sendto(s, buf, len, 0, (struct sockaddr*)&sin, sizeof(sin));
      fprintf(stdout, "simplex-talk: sent %zd bytes\n", bytes_sent);

      /* Receive echo from server using recvfrom() */
      char recv_buf[MAX_LINE];
      socklen_t sin_len = sizeof(sin);
      ssize_t bytes_received = recvfrom(s, recv_buf, sizeof(recv_buf), 0,
                                        (struct sockaddr*)&sin, &sin_len);

      if (bytes_received > 0) {
        recv_buf[bytes_received] = '\0';
        fprintf(stdout, "Received echo: %s", recv_buf);
      } else if (bytes_received < 0) {
        perror("simplex-talk: recvfrom");
      }

      fflush(stdout);
    }
  } else {
    fprintf(stdout, "simplex-talk: unknown protocol: %s. Closing the client\n",
            protocol);
    exit(1);
  }

  close(s);
  return 0;
}


int tcp_socket(int* s, struct sockaddr_in sin) {
    /* Cria um socket TCP */
    fprintf(stdout, "simplex-talk: creating TCP socket\n");
    if ((*s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      perror("simplex-talk: socket");
      exit(1);
    }

    /* Conecta ao servidor */
    fprintf(stdout, "simplex-talk: connecting to server\n");
    if (connect(*s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
      fprintf(stderr, "simplex-talk: connection error\n");
      perror("simplex-talk: connect");
      close(*s);
      exit(1);
    }
    fprintf(stdout, "simplex-talk: connected to server\n");
    return *s;
}

int tcp_send(int s, char* buf, int len) {
    /* Envia dados para o servidor */
    ssize_t bytes_sent = send(s, buf, len, 0);
    if (bytes_sent < 0) {
      perror("simplex-talk: send");
      close(s);
      exit(1);
    }
    //fprintf(stdout, "simplex-talk: sent %zd bytes\n", bytes_sent);
    return bytes_sent;
}

int tcp_receive(int s, char* buf, int buf_size) {
    /* Recebe dados do servidor */
    ssize_t bytes_received = recv(s, buf, buf_size, 0);

    while (bytes_received < 0) {
        bytes_received = recv(s, buf, buf_size, 0);
    }

    buf[bytes_received] = '\0';  // Properly terminate the string
    //fprintf(stdout, "simplex-talk: received %zd bytes\n", bytes_received);
    return bytes_received;
}

void generate_random_string(char *s, const int len) {
    if (len <= 0) {
        if (s) s[0] = '\0';
        return;
    }
    
    // Seed the random number generator for better randomness
    srand((unsigned int)time(NULL));

    for (int i = 0; i < len; ++i) {
        s[i] = (char)(rand() % 256);
    }
    s[len] = '\0'; // Null-terminate the string
}