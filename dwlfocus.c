
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// Currently, we only support sending single `int` commands over the socket.
int main(int argc, char **argv) {
  if (argc != 2) return 1;
  int pid = atoi(argv[1]);
  int socketfd;
  const char *socketpath = getenv("DWLSOCK");
  if (!socketpath) exit(1);
  if ((socketfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) return 1;

  struct sockaddr_un saddr = {AF_UNIX};
  strncpy(saddr.sun_path, socketpath, sizeof(saddr.sun_path) - 1);
  saddr.sun_path[sizeof(saddr.sun_path) - 1] = 0;
  if (connect(socketfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    return 1;
  write(socketfd, &pid, sizeof(pid));

  return 0;
}
