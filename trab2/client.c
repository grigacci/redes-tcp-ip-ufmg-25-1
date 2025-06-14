#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 54321
#define MAX_LINE 256

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

    /* abertura ativa */
    fprintf(stdout, "simplex-talk: opening %s socket\n", protocol);
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      perror("simplex-talk: socket");
      exit(1);
    }

    fprintf(stdout, "simplex-talk: connecting\n");
    if (connect(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
      fprintf(stderr, "simplex-talk: connection error\n");
      perror("simplex-talk: connect");
      close(s);
      exit(1);
    }

    /*laço principal: obtém e envia linhas de texto */
    fprintf(stdout, "Waiting for input ...\n");
    fflush(stdout);

    while (fgets(buf, sizeof(buf), stdin)) {
      buf[MAX_LINE - 1] = '\0';
      len = strlen(buf) + 1;

      ssize_t bytes_sent = send(s, buf, len, 0);
      fprintf(stdout, "simplex-talk: sent %zd bytes\n", bytes_sent);

      char recv_buf[MAX_LINE];
      ssize_t bytes_received = recv(s, recv_buf, sizeof(recv_buf), 0);
      if (bytes_received > 0) {
        recv_buf[bytes_received] = '\0';  // Properly terminate the string
        fprintf(stdout, "Received echo: %s", recv_buf);
      }

      fflush(stdout);
    }

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
