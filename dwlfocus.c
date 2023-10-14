
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// Currently, we only support sending single `int` commands over the socket.
int main(int argc, char **argv) {
  int pid, socketfd;
  const char *socketpath;
  struct sockaddr_un saddr;
  if (argc != 2) return 1;
  pid = atoi(argv[1]);
  socketpath = getenv("DWLSOCK");
  if (!socketpath) exit(1);
  if ((socketfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) return 1;
  saddr.sun_family = AF_UNIX;
  strncpy(saddr.sun_path, socketpath, sizeof(saddr.sun_path) - 1);
  saddr.sun_path[sizeof(saddr.sun_path) - 1] = 0;
  if (connect(socketfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    return 1;
  write(socketfd, &pid, sizeof(pid));

  return 0;
}
