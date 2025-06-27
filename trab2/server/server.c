#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>

#define SERVER_PORT 54321
#define MAX_PENDING 5
#define MAX_LINE 32768

int main(int argc, char* argv[]) {
  struct sockaddr_in sin;
  char buf[MAX_LINE];
  int len;
  int s, new_s;
  char* protocol;
  int i = 0;

  /* monta estrutura de dados de endereço */
  bzero((char*)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(SERVER_PORT);
  len = sizeof(sin);

  if (argc == 2) {
    protocol = argv[1];
    while (protocol[i] != '\0') {
      protocol[i] = toupper(protocol[i]);
      i++;
    }
  } else {
    fprintf(stderr, "usage: simplex-talk host\n");
    exit(1);
  }

  if (strcmp(protocol, "TCP") == 0) {
    /* prepara abertura passiva */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      perror("simplex-talk: socket");
      exit(1);
    }

    if ((bind(s, (struct sockaddr*)&sin, sizeof(sin))) < 0) {
      perror("simplex-talk: bind");
      exit(1);
    }

    listen(s, MAX_PENDING);
    fprintf(stdout, "simplex-talk: waiting for %s connections on port %d\n",
            protocol, ntohs(sin.sin_port));

    /* espera conexão, depois recebe e imprime texto */
    while (1) {
      if ((new_s = accept(s, (struct sockaddr*)&sin, &len)) < 0) {
        perror("simplex-talk: accept");
        exit(1);
      }

      while ((len = recv(new_s, buf, sizeof(buf), 0)) > 0) {
        buf[len] = '\0';  // Properly terminate the string
        fputs(buf, stdout);
        ssize_t bytes_sent =
            send(new_s, buf, len, 0);  // Echo back to the client
        fprintf(stdout, "Sent %zd bytes back to client\n", bytes_sent);
      }

      if (len == 0) {
        printf("Client disconnected\n");
      } else if (len < 0) {
        perror("simplex-talk: recv");
      }

      close(new_s);
    }

  } else if (strcmp(protocol, "UDP") == 0) {
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
      perror("simplex-talk: socket");
      exit(1);
    }

    if ((bind(s, (struct sockaddr*)&sin, sizeof(sin))) < 0) {
      perror("simplex-talk: bind");
      exit(1);
    }

    fprintf(stdout, "simplex-talk: waiting for UDP messages on port %d\n",
            ntohs(sin.sin_port));

    // For UDP we need to keep track of client address
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
      memset(&client_addr, 0, sizeof(client_addr));  // Initialize client_addr
      client_len = sizeof(client_addr);              
      memset(buf, 0, sizeof(buf));

      len = recvfrom(s, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&client_addr,
                     &client_len);

      if (len < 0) {
        perror("simplex-talk: recvfrom");
        continue;
      }

      buf[len] = '\0';  // Properly terminate the string
      fprintf(stdout, "Received from %s:%d: %s",
              inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
              buf);

      // Echo back using sendto()
      ssize_t bytes_sent =
          sendto(s, buf, len, 0, (struct sockaddr*)&client_addr, client_len);

      if (bytes_sent < 0) {
        perror("simplex-talk: sendto");
      } else {
        fprintf(stdout, "Sent %zd bytes back to client\n", bytes_sent);
      }
    }
  } else {
    fprintf(stdout, "simplex-talk: unknown protocol: %s. Closing the server\n",
            protocol);
    exit(1);
  }

  close(s);
  return 0;
}
