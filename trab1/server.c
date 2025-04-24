#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SERVER_PORT 54321
#define MAX_PENDING 5
#define MAX_LINE 256

int main() {
  struct sockaddr_in sin;
  char buf[MAX_LINE];
  int len;
  int s, new_s;

  /* monta estrutura de dados de endereço */
  bzero((char*)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(SERVER_PORT);
  len = sizeof(sin);

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
  fprintf(stdout, "simplex-talk: waiting for connections on port %d\n",
          ntohs(sin.sin_port));

  /* espera conexão, depois recebe e imprime texto */
  while (1) {
    if ((new_s = accept(s, (struct sockaddr*)&sin, &len)) < 0) {
      perror("simplex-talk: accept");
      exit(1);
    }

    while ((len = recv(new_s, buf, sizeof(buf), 0)) > 0) {
      buf[len] = '\0';  // Properly terminate the string
      fputs(buf, stdout);
      ssize_t bytes_sent = send(new_s, buf, len, 0);  // Echo back to the client
      fprintf(stdout, "Sent %zd bytes back to client\n", bytes_sent);
    }

    if (len == 0) {
      printf("Client disconnected\n");
    } else if (len < 0) {
      perror("simplex-talk: recv");
    }

    close(new_s);
  }
}
