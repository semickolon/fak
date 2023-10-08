let { tap, hold, .. } = import "../../fak/keycode.ncl" in
let util = import "../../fak/util_functions.ncl" in

let kc = tap.reg.kc in
let ks = tap.reg.ks in
let md = hold.reg.mod in
let tm = tap.reg.mod in
let me = tap.custom.media in
let XXXX = tap.none & hold.none in

let shared_layers = {
  NUM0 = [
            XXXX,    XXXX,    XXXX,    XXXX,        ks.PLUS, kc.N7,   kc.N8,   kc.N9,
            md.lalt, md.lgui, md.lsft, XXXX,        kc.MINS, kc.N4,   kc.N5,   kc.N6,
   md.lctl, XXXX,    XXXX,    XXXX,                          kc.N1,   kc.N2,   kc.N3,   kc.SLSH,
                              XXXX,    XXXX,        kc.E,    kc.N0,
  ],
  NAV0 = [
            XXXX,    XXXX,    XXXX,    me.VOLD,     me.VOLU, kc.BSPC, kc.DEL,  kc.INS,
            md.lalt, md.lgui, md.lsft, me.MUTE,     kc.LEFT, kc.DOWN, kc.UP,   kc.RGHT,
   md.lctl, XXXX,    XXXX,    XXXX,                          kc.HOME, XXXX,    kc.END,  kc.TAB,
                              XXXX,    XXXX,        XXXX,    XXXX,
  ],
  NUM1 = [
            kc.N9,   kc.N8,   kc.N7,   ks.ASTR,     ks.PLUS, kc.N7,   kc.N8,   kc.N9,
            kc.N6,   kc.N5,   kc.N4,   kc.SLSH,     kc.MINS, kc.N4,   kc.N5,   kc.N6,
   kc.N0,   kc.N3,   kc.N2,   kc.N1,                         kc.N1,   kc.N2,   kc.N3,   kc.DOT,
                              XXXX,    XXXX,        kc.E,    kc.N0,
  ],
  NAV1 = [
            kc.INS,  kc.UP,   kc.BSPC, me.VOLD,     me.VOLU, kc.BSPC, kc.DEL,  kc.INS,
            kc.LEFT, kc.DOWN, kc.RGHT, me.MUTE,     kc.LEFT, kc.DOWN, kc.UP,   kc.RGHT,
   md.lctl, kc.HOME, kc.PGDN, kc.END,                        kc.HOME, kc.PGUP, kc.END,  kc.TAB,
                              XXXX,    XXXX,        XXXX,    XXXX,
  ],
  SYM = [
            kc.GRV,  ks.DQUO, kc.DOT,  kc.LBRC,     kc.RBRC, kc.BSLS, ks.PIPE, kc.SLSH,
            ks.AT,   ks.HASH, ks.DLR,  ks.LPRN,     ks.RPRN, ks.AMPR, ks.ASTR, ks.PERC,
   ks.EXLM, ks.TILD, kc.QUOT, ks.UNDS,                       kc.MINS, ks.PLUS, kc.EQL,  kc.TAB,
                              ks.LCBR, ks.RCBR,     XXXX,    XXXX,
  ],
  FUN = [
            XXXX,    XXXX,    XXXX,    XXXX,        kc.F12,  kc.F7,   kc.F8,   kc.F9,
            XXXX,    XXXX,    XXXX,    XXXX,        kc.F11,  kc.F4,   kc.F5,   kc.F6,
   XXXX,    XXXX,    XXXX,    XXXX,                          kc.F1,   kc.F2,   kc.F3,   kc.F10,
                              XXXX,    XXXX,        XXXX,    XXXX,
  ],
} in

let both_side_combos = [
  [kc.CAPS, 0, 7],
  # Caps word combo here once supported
  # [CW, 10, 13],
] in

{
  qwerty = {
    base_layout = "WERTYUIOSDFGHJKLAXCVM,.P",
    thumbs = [kc.ESC, kc.SPC, kc.N, kc.ENT],
    combos = [
      # Left
      [kc.Q,    0,  1],
      [kc.Z,    17, 18], [kc.B,    18, 19],
      # Right
                                            [kc.QUOT, 5, 6],   [kc.SCLN, 6,  7],
                                            [kc.MINS, 20, 21], [kc.SLSH, 21, 22],
    ] @ both_side_combos,
    layers = {
      NUM = shared_layers.NUM0,
      NAV = shared_layers.NAV0,
      SYM = shared_layers.SYM,
      FUN = shared_layers.FUN,
    }
  },

  colemak = {
    base_layout = "WFPKJLUYRSTGMNEIAXCDH,.O",
    thumbs = [kc.ESC, kc.SPC, kc.ENT, kc.BSPC],
    combos = [
      # Left
      [kc.Q,    0,  1],  [kc.B,    1,  2],
      [kc.Z,    17, 18], [kc.V,    18, 19],
      # Right
                                            [kc.QUOT, 5, 6],   [kc.SCLN, 6,  7],
                                            [kc.MINS, 20, 21], [kc.SLSH, 21, 22],
    ] @ both_side_combos,
    layers = {
      NUM = shared_layers.NUM0,
      NAV = shared_layers.NAV0,
      SYM = shared_layers.SYM,
      FUN = shared_layers.FUN,
    }
  },

  aptmak = {
    base_layout = "WFPBJLUYSTHKXNAIRCGDM,.O",
    thumbs = [kc.QUOT, kc.SPC, kc.E, kc.ENT],
    combos = [
      # Left
      [kc.V,    0,  1],  [kc.Q,    1,  2],
      [ks.LCBR, 0,  8],  [kc.LBRC, 1,  9],  [ks.LPRN, 2,  10], [ks.LABK, 3,  11],
      [kc.Z,    17, 18], [kc.ESC,  18, 19],
      # Right
                                            [tm.lctl & kc.BSPC, 5, 7],
                                            [kc.BSPC, 5, 6],   [kc.DEL,  6,  7],
      [ks.RABK, 4,  12], [ks.RPRN, 5,  13], [kc.RBRC, 6,  14], [ks.RCBR, 7,  15],
                                            [kc.MINS, 20, 21], [kc.SLSH, 21, 22],
    ] @ both_side_combos,
    layers = {
      NUM = shared_layers.NUM1,
      NAV = shared_layers.NAV1,
      SYM = shared_layers.SYM,
      FUN = shared_layers.FUN,
    },
  },
}
