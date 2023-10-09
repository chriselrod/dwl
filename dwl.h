#ifndef dwl_h_INCLUDED
#define dwl_h_INCLUDED


#include <getopt.h>
#include <libinput.h>
#include <limits.h>
#include <linux/input-event-codes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/backend/libinput.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_control_v1.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_export_dmabuf_v1.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_idle.h>
#include <wlr/types/wlr_idle_inhibit_v1.h>
#include <wlr/types/wlr_idle_notify_v1.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_input_inhibitor.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_output_management_v1.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_primary_selection_v1.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_screencopy_v1.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_session_lock_v1.h>
#include <wlr/types/wlr_single_pixel_buffer_v1.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_activation_v1.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>
#ifdef XWAYLAND
#include <X11/Xlib.h>
#include <wlr/xwayland.h>
#include <xcb/xcb_icccm.h>
#endif


/* macros */
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define CLEANMASK(mask) ((mask) & ~WLR_MODIFIER_CAPS)
#define VISIBLEON(C, M)                                                        \
  ((M) && (C)->mon == (M) && ((C)->tags & (M)->tagset[(M)->seltags]))
#define LENGTH(X) (sizeof X / sizeof X[0])
#define END(A) ((A) + LENGTH(A))
#define TAGMASK ((1u << tagcount) - 1)
#define LISTEN(E, L, H) wl_signal_add((E), ((L)->notify = (H), (L)))
#define IDLE_NOTIFY_ACTIVITY                                                   \
  wlr_idle_notify_activity(idle, seat),                                        \
      wlr_idle_notifier_v1_notify_activity(idle_notifier, seat)

/* enums */
enum { CurNormal, CurPressed, CurMove, CurResize };      /* cursor */
enum { XDGShell, LayerShell, X11Managed, X11Unmanaged }; /* client types */
enum {
  LyrBg,
  LyrBottom,
  LyrTile,
  LyrFloat,
  LyrFS,
  LyrTop,
  LyrOverlay,
  LyrBlock,
  NUM_LAYERS
}; /* scene layers */
#ifdef XWAYLAND
enum {
  NetWMWindowTypeDialog,
  NetWMWindowTypeSplash,
  NetWMWindowTypeToolbar,
  NetWMWindowTypeUtility,
  NetLast
}; /* EWMH atoms */
#endif

typedef struct Monitor Monitor;
typedef struct {
  /* Must keep these three elements in this order */
  unsigned int type;   /* XDGShell or X11* */
  struct wlr_box geom; /* layout-relative, includes border */
  Monitor *mon;
  struct wlr_scene_tree *scene;
  struct wlr_scene_rect *border[4]; /* top, bottom, left, right */
  struct wlr_scene_tree *scene_surface;
  struct wl_list link;
  struct wl_list flink;
  union {
    struct wlr_xdg_surface *xdg;
    struct wlr_xwayland_surface *xwayland;
  } surface;
  struct wl_listener commit;
  struct wl_listener map;
  struct wl_listener maximize;
  struct wl_listener unmap;
  struct wl_listener destroy;
  struct wl_listener set_title;
  struct wl_listener fullscreen;
  struct wlr_box prev; /* layout-relative, includes border */
#ifdef XWAYLAND
  struct wl_listener activate;
  struct wl_listener configure;
  struct wl_listener set_hints;
#endif
  unsigned int bw;
  uint32_t tags;
  int isfloating, isurgent, isfullscreen;
  uint32_t resize; /* configure serial of a pending resize */
} Client;

typedef struct {
  struct wl_list link;
  struct wlr_keyboard *wlr_keyboard;

  int nsyms;
  const xkb_keysym_t *keysyms; /* invalid if nsyms == 0 */
  uint32_t mods;               /* invalid if nsyms == 0 */
  struct wl_event_source *key_repeat_source;

  struct wl_listener modifiers;
  struct wl_listener key;
  struct wl_listener destroy;
} Keyboard;

typedef struct {
  /* Must keep these three elements in this order */
  unsigned int type; /* LayerShell */
  struct wlr_box geom;
  Monitor *mon;
  struct wlr_scene_tree *scene;
  struct wlr_scene_tree *popups;
  struct wlr_scene_layer_surface_v1 *scene_layer;
  struct wl_list link;
  int mapped;
  struct wlr_layer_surface_v1 *layer_surface;

  struct wl_listener destroy;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener surface_commit;
} LayerSurface;

typedef struct {
  const char *symbol;
  void (*arrange)(Monitor *);
} Layout;

struct Monitor {
  struct wl_list link;
  struct wlr_output *wlr_output;
  struct wlr_scene_output *scene_output;
  struct wlr_scene_rect *fullscreen_bg; /* See createmon() for info */
  struct wl_listener frame;
  struct wl_listener destroy;
  struct wl_listener destroy_lock_surface;
  struct wlr_session_lock_surface_v1 *lock_surface;
  struct wlr_box m;         /* monitor area, layout-relative */
  struct wlr_box w;         /* window area, layout-relative */
  struct wl_list layers[4]; /* LayerSurface::link */
  const Layout *lt[2];
  unsigned int seltags;
  unsigned int sellt;
  uint32_t tagset[2];
  int nmaster;
  double mfact;
  char ltsymbol[16];
};

typedef struct {
  const char *name;
  float mfact;
  int nmaster;
  float scale;
  const Layout *lt;
  enum wl_output_transform rr;
  int x, y;
} MonitorRule;

typedef struct {
  const char *id;
  const char *title;
  uint32_t tags;
  int isfloating;
  int monitor;
} Rule;

typedef struct {
  struct wlr_scene_tree *scene;

  struct wlr_session_lock_v1 *lock;
  struct wl_listener new_surface;
  struct wl_listener unlock;
  struct wl_listener destroy;
} SessionLock;

/* function declarations */
static void applybounds(Client *c, struct wlr_box *bbox);
static void applyrules(Client *c);
static void arrange(Monitor *m);
static void arrangelayer(Monitor *m, struct wl_list *list,
                         struct wlr_box *usable_area, int exclusive);
static void arrangelayers(Monitor *m);
static void axisnotify(struct wl_listener *listener, void *data);
static void buttonpress(struct wl_listener *listener, void *data);
static void chvt(uint32_t ui);
static void checkidleinhibitor(struct wlr_surface *exclude);
static void cleanup(void);
static void cleanupkeyboard(struct wl_listener *listener, void *data);
static void cleanupmon(struct wl_listener *listener, void *data);
static void closemon(Monitor *m);
static void commitlayersurfacenotify(struct wl_listener *listener, void *data);
static void commitnotify(struct wl_listener *listener, void *data);
static void createdecoration(struct wl_listener *listener, void *data);
static void createidleinhibitor(struct wl_listener *listener, void *data);
static void createkeyboard(struct wlr_keyboard *keyboard);
static void createlayersurface(struct wl_listener *listener, void *data);
static void createlocksurface(struct wl_listener *listener, void *data);
static void createmon(struct wl_listener *listener, void *data);
static void createnotify(struct wl_listener *listener, void *data);
static void createpointer(struct wlr_pointer *pointer);
static void cursorframe(struct wl_listener *listener, void *data);
static void destroydragicon(struct wl_listener *listener, void *data);
static void destroyidleinhibitor(struct wl_listener *listener, void *data);
static void destroylayersurfacenotify(struct wl_listener *listener, void *data);
static void destroylock(SessionLock *lock, int unlocked);
static void destroylocksurface(struct wl_listener *listener, void *data);
static void destroynotify(struct wl_listener *listener, void *data);
static void destroysessionlock(struct wl_listener *listener, void *data);
static void destroysessionmgr(struct wl_listener *listener, void *data);
static Monitor *dirtomon(enum wlr_direction dir);
static void focusclient(Client *c, int lift);
static void focusmon(int i);
static void focusstack(int i);
static Client *focustop(Monitor *m);
static void fullscreennotify(struct wl_listener *listener, void *data);
static void handlesig(int signo);
static void incnmaster(int i);
static void inputdevice(struct wl_listener *listener, void *data);
static int keybinding(uint32_t mods, xkb_keysym_t sym);
static void keypress(struct wl_listener *listener, void *data);
static void keypressmod(struct wl_listener *listener, void *data);
static int keyrepeat(void *data);
static void killclient(void);
static void locksession(struct wl_listener *listener, void *data);
static void maplayersurfacenotify(struct wl_listener *listener, void *data);
static void mapnotify(struct wl_listener *listener, void *data);
static void maximizenotify(struct wl_listener *listener, void *data);
static void monocle(Monitor *m);
static void motionabsolute(struct wl_listener *listener, void *data);
static void motionnotify(uint32_t time);
static void motionrelative(struct wl_listener *listener, void *data);
static void moveresize(uint32_t ui);
static void outputmgrapply(struct wl_listener *listener, void *data);
static void outputmgrapplyortest(struct wlr_output_configuration_v1 *config,
                                 int test);
static void outputmgrtest(struct wl_listener *listener, void *data);
static void pointerfocus(Client *c, struct wlr_surface *surface, double sx,
                         double sy, uint32_t time);
static void printstatus(void);
static void quit(void);
static void rendermon(struct wl_listener *listener, void *data);
static void requeststartdrag(struct wl_listener *listener, void *data);
static void resize(Client *c, struct wlr_box geo, int interact);
static void run(char *startup_cmd);
static void setcursor(struct wl_listener *listener, void *data);
static void setfloating(Client *c, int floating);
static void setfullscreen(Client *c, int fullscreen);
static void setlayout(const Layout *layout);
static void setmfact(int i);
static void setmon(Client *c, Monitor *m, uint32_t newtags);
static void setpsel(struct wl_listener *listener, void *data);
static void setsel(struct wl_listener *listener, void *data);
static void setup(void);
static void spawn(const char *cmd);
static void spawn1(const char *cmd, const char *arg);
static void startdrag(struct wl_listener *listener, void *data);
static void tag(uint32_t ui);
static void tagmon(int i);
static void tile(Monitor *m);
static void column(Monitor *m);
static void togglefloating(void);
static void togglefullscreen(void);
static void toggletag(uint32_t ui);
static void toggleview(uint32_t ui);
static void unlocksession(struct wl_listener *listener, void *data);
static void unmaplayersurfacenotify(struct wl_listener *listener, void *data);
static void unmapnotify(struct wl_listener *listener, void *data);
static void updatemons(struct wl_listener *listener, void *data);
static void updatetitle(struct wl_listener *listener, void *data);
static void urgent(struct wl_listener *listener, void *data);
static void view(uint32_t ui);
static void virtualkeyboard(struct wl_listener *listener, void *data);
static Monitor *xytomon(double x, double y);
static void xytonode(double x, double y, struct wlr_surface **psurface,
                     Client **pc, LayerSurface **pl, double *nx, double *ny);
static void zoom(void);

/* variables */
static const char broken[] = "broken";
static const char *cursor_image = "left_ptr";
static pid_t child_pid = -1;
static int locked;
static void *exclusive_focus;
static struct wlr_backend *backend;
static struct wlr_scene *scene;
static struct wlr_scene_tree *layers[NUM_LAYERS];
static struct wlr_scene_tree *drag_icon;
/* Map from ZWLR_LAYER_SHELL_* constants to Lyr* enum */
static const int layermap[] = {LyrBg, LyrBottom, LyrTop, LyrOverlay};
static struct wlr_renderer *drw;
static struct wlr_allocator *alloc;
static struct wlr_compositor *compositor;

static struct wlr_xdg_shell *xdg_shell;
static struct wlr_xdg_activation_v1 *activation;
static struct wlr_xdg_decoration_manager_v1 *xdg_decoration_mgr;
static struct wl_list clients; /* tiling order */
static struct wl_list fstack;  /* focus order */
static struct wlr_idle *idle;
static struct wlr_idle_notifier_v1 *idle_notifier;
static struct wlr_idle_inhibit_manager_v1 *idle_inhibit_mgr;
static struct wlr_input_inhibit_manager *input_inhibit_mgr;
static struct wlr_layer_shell_v1 *layer_shell;
static struct wlr_output_manager_v1 *output_mgr;
static struct wlr_virtual_keyboard_manager_v1 *virtual_keyboard_mgr;

static struct wlr_cursor *cursor;
static struct wlr_xcursor_manager *cursor_mgr;

static struct wlr_session_lock_manager_v1 *session_lock_mgr;
static struct wlr_scene_rect *locked_bg;
static struct wlr_session_lock_v1 *cur_lock;

static struct wlr_seat *seat;
static struct wl_list keyboards;
static unsigned int cursor_mode;
static Client *grabc;
static int grabcx, grabcy; /* client-relative */

static struct wlr_output_layout *output_layout;
static struct wlr_box sgeom;
static struct wl_list mons;
static Monitor *selmon;

/* global event handlers */
static struct wl_listener cursor_axis = {.notify = axisnotify};
static struct wl_listener cursor_button = {.notify = buttonpress};
static struct wl_listener cursor_frame = {.notify = cursorframe};
static struct wl_listener cursor_motion = {.notify = motionrelative};
static struct wl_listener cursor_motion_absolute = {.notify = motionabsolute};
static struct wl_listener drag_icon_destroy = {.notify = destroydragicon};
static struct wl_listener idle_inhibitor_create = {.notify =
                                                       createidleinhibitor};
static struct wl_listener idle_inhibitor_destroy = {.notify =
                                                        destroyidleinhibitor};
static struct wl_listener layout_change = {.notify = updatemons};
static struct wl_listener new_input = {.notify = inputdevice};
static struct wl_listener new_virtual_keyboard = {.notify = virtualkeyboard};
static struct wl_listener new_output = {.notify = createmon};
static struct wl_listener new_xdg_surface = {.notify = createnotify};
static struct wl_listener new_xdg_decoration = {.notify = createdecoration};
static struct wl_listener new_layer_shell_surface = {.notify =
                                                         createlayersurface};
static struct wl_listener output_mgr_apply = {.notify = outputmgrapply};
static struct wl_listener output_mgr_test = {.notify = outputmgrtest};
static struct wl_listener request_activate = {.notify = urgent};
static struct wl_listener request_cursor = {.notify = setcursor};
static struct wl_listener request_set_psel = {.notify = setpsel};
static struct wl_listener request_set_sel = {.notify = setsel};
static struct wl_listener request_start_drag = {.notify = requeststartdrag};
static struct wl_listener start_drag = {.notify = startdrag};
static struct wl_listener session_lock_create_lock = {.notify = locksession};
static struct wl_listener session_lock_mgr_destroy = {.notify =
                                                          destroysessionmgr};
#ifdef XWAYLAND
static void activatex11(struct wl_listener *listener, void *data);
static void configurex11(struct wl_listener *listener, void *data);
static void createnotifyx11(struct wl_listener *listener, void *data);
static Atom getatom(xcb_connection_t *xc, const char *name);
static void sethints(struct wl_listener *listener, void *data);
static void xwaylandready(struct wl_listener *listener, void *data);
static struct wl_listener new_xwayland_surface = {.notify = createnotifyx11};
static struct wl_listener xwayland_ready = {.notify = xwaylandready};
static struct wlr_xwayland *xwayland;
static Atom netatom[NetLast];
#endif


#endif // dwl_h_INCLUDED
