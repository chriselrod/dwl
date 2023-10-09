#ifndef dwl_socket_h_INCLUDED
#define dwl_socket_h_INCLUDED

#include "client.h"
#include "dwl.h"
#include "util.h"
#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// This file is based in part on Sway's IPC code.
// https://github.com/swaywm/sway/blob/master/LICENSE
static int ipc_socket = -1;
static struct wl_event_source *ipc_event_source = NULL;
static struct wl_display *dpy;
static struct sockaddr_un saddr;

static inline int find_parent_pid(int cpid) {
  char filename[64];
  // 4th entry is ppid
  sprintf(filename, "/proc/%d/stat", cpid);

  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    perror("fopen");
    return -1;
  }
  // pid (Name Multi Word) S ppid ...
  int ppid = 0;
  fscanf(file, "%*llu (%*[^)]%*[)] %*c %d", &ppid);
  fclose(file);
  return ppid;
}

static inline int handle_connection(int fd, uint32_t mask, void *data) {
  (void)fd;
  assert(mask == WL_EVENT_READABLE);

  int client_fd = accept(ipc_socket, NULL, NULL);
  if (client_fd == -1) return 0;

  // int flags;
  // if ((flags = fcntl(client_fd, F_GETFD)) == -1 ||
  //     fcntl(client_fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
  //   close(client_fd);
  //   return 0;
  // }
  // if ((flags = fcntl(client_fd, F_GETFL)) == -1 ||
  //     fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
  //   close(client_fd);
  //   return 0;
  // }
  int pid_to_focus;
  read(client_fd, &pid_to_focus, sizeof(pid_to_focus));
  if (pid_to_focus <= 0) return 0;

  Client *c;
  int pid = 0;
  wl_list_for_each(c, &clients, link) {
    wl_client_get_credentials(c->surface.xdg->client->client, &pid, NULL, NULL);
    while (pid) {
      if (pid == pid_to_focus) {
        focusclient(c, 1);
        return 0;
      }
      pid = find_parent_pid(pid);
    }
  }
  return 0;
}

static inline void ipc_init(void) {
  ipc_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (ipc_socket == -1) die("Unable to create socket failed");
  if (fcntl(ipc_socket, F_SETFD, FD_CLOEXEC) == -1)
    die("Unable to set CLOEXEC on socket.");
  if (fcntl(ipc_socket, F_SETFL, O_NONBLOCK) == -1)
    die("Unable to set NONBLOCK on IPC socket.");

  saddr.sun_family = AF_UNIX;
  int path_size = sizeof(saddr.sun_path);

  const char *dir = getenv("XDG_RUNTIME_DIR");
  /* Wayland requires XDG_RUNTIME_DIR for creating its communications socket */
  if (!dir) die("XDG_RUNTIME_DIR must be set");
  if (path_size <= snprintf(saddr.sun_path, path_size, "%s/sway-ipc.%u.%i.sock",
                            dir, getuid(), getpid())) {
    die("Socket path won't fit into ipc_sockaddr->sun_path");
  }
  unlink(saddr.sun_path);
  if (bind(ipc_socket, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
    die("Unable to bind IPC socket.");
  }
  if (listen(ipc_socket, 3) == -1) die("Unable to listen on IPC socket.");
  setenv("DWLSOCK", saddr.sun_path, 1);

  ipc_event_source =
      wl_event_loop_add_fd(wl_display_get_event_loop(dpy), ipc_socket,
                           WL_EVENT_READABLE, handle_connection, NULL);
}

#endif // dwl_socket_h_INCLUDED
