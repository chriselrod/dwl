#ifndef keyactions_h_INCLUDED
#define keyactions_h_INCLUDED
#include "client.h"
#include "config.h"
#include "dwl.h"
#include "util.h"
void focus_kakc(void) {
  Client *c;
  wl_list_for_each(c, &clients, link) {
    const char *title = client_get_title(c);
    if ((!title) || (strncmp(title, "kak -c", 6) != 0)) continue;
    focusclient(c, 1);
    return;
  }
}
int keybinding(uint32_t mods, xkb_keysym_t sym) {
  /*
   * Here we handle compositor keybindings. This is when the compositor is
   * processing keys, rather than passing them on to the client for its own
   * processing.
   */
  uint32_t cmods = CLEANMASK(mods);
  int i = 0;
  // case XKB_KEY_i: i = 2; __attribute__((fallthrough));
  // case XKB_KEY_d: incnmaster(--i); return 1;
  if (cmods == CLEANMASK(MODKEY)) {
    switch (sym) {
    case XKB_KEY_l:
    case XKB_KEY_k: i = 1; __attribute__((fallthrough));
    case XKB_KEY_j:
    case XKB_KEY_h: focusstack(i); return 1;
    case XKB_KEY_comma: i = 1; __attribute__((fallthrough));
    case XKB_KEY_period:
      focusmon(i ? WLR_DIRECTION_LEFT : WLR_DIRECTION_RIGHT);
      return 1;
    case XKB_KEY_9: i = 1; __attribute__((fallthrough));
    case XKB_KEY_8: ++i; __attribute__((fallthrough));
    case XKB_KEY_7: ++i; __attribute__((fallthrough));
    case XKB_KEY_6: ++i; __attribute__((fallthrough));
    case XKB_KEY_5: ++i; __attribute__((fallthrough));
    case XKB_KEY_4: ++i; __attribute__((fallthrough));
    case XKB_KEY_3: ++i; __attribute__((fallthrough));
    case XKB_KEY_2: ++i; __attribute__((fallthrough));
    case XKB_KEY_1: view((uint32_t)1 << (uint32_t)i); return 1;
    case XKB_KEY_p: spawn1(menucmd, "--drun-launch=true"); return 1;
    case XKB_KEY_equal: i = 2; __attribute__((fallthrough));
    case XKB_KEY_minus: setmfact(--i); return 1;
    case XKB_KEY_Return: zoom(); return 1;
    case XKB_KEY_0: i = ~0; __attribute__((fallthrough));
    case XKB_KEY_Tab: view(i); return 1;
    // case XKB_KEY_space: i = -3; __attribute__((fallthrough));
    case XKB_KEY_m: ++i; __attribute__((fallthrough));
    // case XKB_KEY_f: ++i; __attribute__((fallthrough));
    case XKB_KEY_t: ++i; __attribute__((fallthrough));
    case XKB_KEY_c: setlayout((enum Layout)i); return 1;
    case XKB_KEY_e: togglefullscreen(); return 1;
    }
    return 1;
  } else if (cmods == CLEANMASK(MODKEY | WLR_MODIFIER_CTRL)) {
    switch (sym) {
    case XKB_KEY_9: i = 1; __attribute__((fallthrough));
    case XKB_KEY_8: ++i; __attribute__((fallthrough));
    case XKB_KEY_7: ++i; __attribute__((fallthrough));
    case XKB_KEY_6: ++i; __attribute__((fallthrough));
    case XKB_KEY_5: ++i; __attribute__((fallthrough));
    case XKB_KEY_4: ++i; __attribute__((fallthrough));
    case XKB_KEY_3: ++i; __attribute__((fallthrough));
    case XKB_KEY_2: ++i; __attribute__((fallthrough));
    case XKB_KEY_1: toggleview((uint32_t)1 << (uint32_t)i); return 1;
    }
  } else if (cmods == CLEANMASK(MODKEY | WLR_MODIFIER_SHIFT)) {
    switch (sym) {
    case XKB_KEY_parenleft: i = 1; __attribute__((fallthrough));
    case XKB_KEY_asterisk: ++i; __attribute__((fallthrough));
    case XKB_KEY_ampersand: ++i; __attribute__((fallthrough));
    case XKB_KEY_asciicircum: ++i; __attribute__((fallthrough));
    case XKB_KEY_percent: ++i; __attribute__((fallthrough));
    case XKB_KEY_dollar: ++i; __attribute__((fallthrough));
    case XKB_KEY_numbersign: ++i; __attribute__((fallthrough));
    case XKB_KEY_at: ++i; __attribute__((fallthrough));
    case XKB_KEY_exclam: tag((uint32_t)1 << (uint32_t)i); return 1;
    case XKB_KEY_Return: spawn(termcmd); return 1;
    case XKB_KEY_L:
    case XKB_KEY_K: i = 1; __attribute__((fallthrough));
    case XKB_KEY_J:
    case XKB_KEY_H: movestack(i); return 1;
    case XKB_KEY_C: killclient(); return 1;
    //	case XKB_KEY_space: togglefloating(); return 1;
    case XKB_KEY_parenright: tag(~0); return 1;
    case XKB_KEY_less: i = 1; __attribute__((fallthrough));
    case XKB_KEY_greater:
      tagmon(i ? WLR_DIRECTION_LEFT : WLR_DIRECTION_RIGHT);
      return 1;
    case XKB_KEY_Q: quit(); return 1;
    }
  } else if (cmods ==
             CLEANMASK(MODKEY | WLR_MODIFIER_CTRL | WLR_MODIFIER_SHIFT)) {
    switch (sym) {
    case XKB_KEY_parenleft: i = 1; __attribute__((fallthrough));
    case XKB_KEY_asterisk: ++i; __attribute__((fallthrough));
    case XKB_KEY_ampersand: ++i; __attribute__((fallthrough));
    case XKB_KEY_asciicircum: ++i; __attribute__((fallthrough));
    case XKB_KEY_percent: ++i; __attribute__((fallthrough));
    case XKB_KEY_dollar: ++i; __attribute__((fallthrough));
    case XKB_KEY_numbersign: ++i; __attribute__((fallthrough));
    case XKB_KEY_at: ++i; __attribute__((fallthrough));
    case XKB_KEY_exclam: toggletag((uint32_t)1 << (uint32_t)i); return 1;
    }
  } else if (cmods == CLEANMASK(WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT)) {
    switch (sym) {
    case XKB_KEY_Terminate_Server: quit(); return 1;
    case XKB_KEY_XF86Switch_VT_12: i = 1; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_11: ++i; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_10: ++i; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_9: ++i; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_8: ++i; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_7: ++i; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_6: ++i; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_5: ++i; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_4: ++i; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_3: ++i; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_2: ++i; __attribute__((fallthrough));
    case XKB_KEY_XF86Switch_VT_1: chvt(++i); return 1;
    }
  } else if (cmods == CLEANMASK(WLR_MODIFIER_CTRL | WLR_MODIFIER_SHIFT)) {
    if ((sym == XKB_KEY_E) || (sym == XKB_KEY_B)) focus_kakc(); // passthrough
  }
  return 0;
}
#endif // keyactions_h_INCLUDED
