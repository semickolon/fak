let { tap, hold, td, combo, .. } = import "../../fak/keycode.ncl" in

let kc = tap.reg.kc in
let md = hold.reg.mod in
let XXXX = tap.none & hold.none in
let TTTT = tap.trans & hold.trans in
let MO = hold.reg.layer in

let cu = {
  SFTZ = td.make 100 [
    md.lsft & kc.Z & hold.reg.behavior {},
    tap.reg.mod.lsft & kc.Z,
  ],
  TH_L = MO 1 & kc.SPC & hold.reg.behavior {},  
  TH_R = MO 2 & kc.SPC & hold.reg.behavior {},
  FN_G = MO 3 & kc.GRV & hold.reg.behavior {},
} in

{
  virtual_keys = [
    combo.make 50 [0, 1],
    combo.make 50 [7, 8],
    combo.make 50 [8, 9],
    combo.make 50 [10, 11],
    combo.make 50 [17, 18],
    combo.make 50 [26, 27],
    combo.make 50 [0, 9, 28, 35],
  ],
  layers = [
    [ # Layer 0 - Base
      kc.Q,    kc.W,    kc.E,    kc.R,    kc.T,    kc.Y,    kc.U,    kc.I,    kc.O,    kc.P,
      kc.A,    kc.S,    kc.D,    kc.F,    kc.G,    kc.H,    kc.J,    kc.K,    kc.L,
      cu.SFTZ, kc.X,    kc.C,    kc.V,    kc.B,    kc.N,    kc.M,    kc.",",  kc.".",  
      md.lctl, md.lgui, md.lalt,          cu.TH_L, cu.TH_R,          cu.FN_G, kc.N2,   kc.N3,
      # Combos
      kc.ESC,
      kc.DEL,
      kc.BSPC,
      kc.TAB,
      kc.ENT,
      kc.SLSH,
      tap.custom.fak.BOOT,
    ],
    [ # Layer 1 - Num
      kc.N1,   kc.N2,   kc.N3,   kc.N4,   kc.N5,   kc.N6,   kc.N7,   kc.N8,   kc.N9,   kc.N0,
      TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,
      TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,
      TTTT,    TTTT,    TTTT,             TTTT,    TTTT,             TTTT,    TTTT,    TTTT,
      # Combos
      TTTT,
      TTTT,
      TTTT,
      TTTT,
      TTTT,
      TTTT,
      XXXX,
    ],
    [ # Layer 2 - Num
      TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,
      TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    kc.LEFT, kc.DOWN, kc.UP,   kc.RGHT,
      TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,    TTTT,
      TTTT,    TTTT,    TTTT,             TTTT,    TTTT,             TTTT,    TTTT,    TTTT,
      # Combos
      TTTT,
      TTTT,
      TTTT,
      TTTT,
      TTTT,
      TTTT,
      XXXX,
    ],
    [ # Layer 3 - Fun
      kc.F1,   kc.F2,   kc.F3,   kc.F4,   kc.F5,   kc.F6,   kc.F7,   kc.F8,   kc.F9,   kc.F10,
      kc.F11,  kc.F12,  XXXX,    XXXX,    XXXX,    kc.HOME, kc.PGDN, kc.PGUP, kc.END,
      XXXX,    XXXX,    XXXX,    XXXX,    XXXX,    XXXX,    XXXX,    XXXX,    XXXX,
      XXXX,    XXXX,    XXXX,             XXXX,    XXXX,             XXXX,    XXXX,    XXXX,
      # Combos
      TTTT,
      TTTT,
      TTTT,
      TTTT,
      TTTT,
      TTTT,
      XXXX,
    ]
  ]
}
