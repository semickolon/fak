let { tap, hold, td, combo, macro, ..} = import "../../ncl/fak/keycode.ncl" in

# convenience aliases
# taps
let kc = tap.reg.kc in
let ks = tap.reg.ks in
let md = tap.reg.mod in
# holds
# let kch = hold.reg.kc in
# let ksh = hold.reg.ks in
let mdh = hold.reg.mod in
# sticky key
let sk = tap.sticky.mod in
# layer control
let lt = hold.reg.layer in
# boot, reset, etc
let fak = tap.custom.fak in
# media and mouse control
let media = tap.custom.media in
let mouse = tap.custom.mouse in
# keymap stuff
let tx = tap.none in
let tt = tap.trans in
let hx = hold.none in
let ht = hold.trans in

let hrm_behavior = {
  timeout_ms = 150,
  timeout_decision = 'hold,
} in

let cu = {
  # copy custom stuff from example
  COPY = md.lctl & kc.C,
  PSTE = md.lctl & kc.V,
  CUT  = md.lctl & kc.X,
  SCH  = macro.make [
    macro.tap kc.S,
    macro.release mdh.rsft,
    macro.tap kc.C,
    macro.tap kc.H,
  ]
} in

# TODO: idea: make umlauts by tap-then-tap-and-holding the respective vowels
#let TD = {
#  let AE = td.make 200 [
#    kc.A,
#    kc.A & mdh.ralt & hold.reg.kc.A,
#  ] in
#  let OE = td.make 200 [
#    kc.O,
#    kc.O & mdh.ralt & hold.reg.kc.O,
#  ] in
#  let UE = td.make 200 [
#    kc.U,
#    kc.U & mdh.ralt & hold.reg.kc.U,
#  ] in
#  let SZ = td.make 200 [
#    kc.S,
#    kc.S & mdh.ralt & hold.reg.kc.S,
#  ] in
#} in

# left hand hrm
let hrmLP = kc.R & mdh.lctl & hold.reg.behavior hrm_behavior in
let hrmLR = kc.S & mdh.lalt & hold.reg.behavior hrm_behavior in
let hrmLM = kc.T & mdh.lgui & hold.reg.behavior hrm_behavior in
let hrmLI = kc.H & mdh.lsft & hold.reg.behavior hrm_behavior in
let hrmLS = kc.K & mdh.ralt & hold.reg.behavior hrm_behavior in

# right hand hrm
let hrmRS = kc.X & mdh.ralt & hold.reg.behavior hrm_behavior in
let hrmRI = kc.N & mdh.rsft & hold.reg.behavior hrm_behavior in
let hrmRM = kc.A & mdh.rgui & hold.reg.behavior hrm_behavior in
let hrmRR = kc.I & mdh.lalt & hold.reg.behavior hrm_behavior in
let hrmRP = kc.O & mdh.rctl & hold.reg.behavior hrm_behavior in

# layer taps
let ltLTT = kc.QUOT & lt 1 & hold.reg.behavior hrm_behavior in
let ltLTR = kc.SPC  & lt 2 & hold.reg.behavior hrm_behavior in
let ltRTR = kc.E    & lt 3 & hold.reg.behavior hrm_behavior in
let ltRTT = kc.TAB  & lt 4 & hold.reg.behavior hrm_behavior in

let combos_base = [
  kc.V, kc.Q, kc.Z, cu.SCH,
  kc.SLSH, kc.MINS, kc.SCLN, ks.COLN,
  ks.LCBR, kc.LBRC, ks.LPRN, ks.LABK, ks.RABK, ks.RPRN, kc.RBRC, ks.RCBR,
  kc.ESC, kc.ENT, kc.BSPC, kc.DEL, md.lctl & kc.BSPC,
  fak.CWTG, kc.CAPS_LOCK,
] in

### Keymap itself! ###
{
  virtual_keys = [
    # alpha
    combo.make 25 [0, 1], # V
    combo.make 25 [1, 2], # Q
    combo.make 25 [18, 19], # Z
    combo.make 25 [0, 2], # SCH
    # sym
    combo.make 25 [22, 23], # SLSH
    combo.make 25 [21, 22], # MINS
    combo.make 25 [22, 27], # SEMI
    combo.make 25 [23, 27], # COLON
    # parentheticals
    combo.make 25 [0, 9],  # {
    combo.make 25 [1, 10], # [
    combo.make 25 [2, 11], # (
    combo.make 25 [3, 12], # <
    combo.make 25 [4, 13], # >
    combo.make 25 [5, 14], # )
    combo.make 25 [6, 15], # ]
    combo.make 25 [7, 16], # }
    # nav
    combo.make 25 [19, 20], # ESC
    combo.make 25 [21, 23], # ENT
    combo.make 25 [5, 6], # BSPC
    combo.make 25 [6, 7], # DEL
    combo.make 25 [5, 7], # C-BSPC
    # caps
    combo.make 25 [11, 14], # caps word
    combo.make 25 [0, 7], # caps lock
  ],
  layers = [
    # ALPHAS
    [
               kc.W,    kc.F,    kc.P,    kc.B,    kc.J,    kc.L,    kc.U,    kc.Y,
      hrmLP,   hrmLR,   hrmLM,   hrmLI,   hrmLS,   hrmRS,   hrmRI,   hrmRM,   hrmRR,   hrmRP,
               kc.C,    kc.G,    kc.D,                      kc.M,    kc.COMM, kc.DOT,
                                 ltLTT,   ltLTR,   ltRTR,   ltRTT,
    ] @ combos_base,
    # NUM
    [
                  kc.N6,      kc.N5,      kc.N4,      ks.ASTR,      ks.PLUS, kc.N7,   kc.N8,   kc.N9,
      kc.N0 & ht, kc.N3 & ht, kc.N2 & ht, kc.N1 & ht, kc.SLSH & ht, kc.MINS, kc.N4,   kc.N5,   kc.N6,   kc.DOT,
                  kc.N9,      kc.N8,      kc.N7,                             kc.N1,   kc.N2,   kc.N3,
                                          tt & ht,    tt & ht,      kc.E,    kc.N0,
    ] @ combos_base,
    # NAV
    [
                  md.lalt,      mouse.UP,     md.lsft,      media.VOLU,   cu.CUT,     cu.COPY,    cu.PSTE,  tt & ht,
      md.lctl,    mouse.LEFT,   mouse.DOWN,   mouse.RGHT,   media.VOLD,   media.MUTE, kc.LEFT,    kc.DOWN,  kc.UP,      kc.RGHT,
                  kc.HOME,      kc.PGDN,      kc.END,                                 kc.HOME,    kc.PGUP,  kc.END,
                                              tt & ht,      tt & ht,      mouse.BTN1, mouse.BTN2,
    ] @ combos_base,
    # SYM
    [
               kc.GRV,  ks.DQUO, kc.P,    tx & hx, tx & hx, kc.BSLS, ks.PIPE, tx & hx,
      ks.EXLM, ks.AT,   ks.HASH, ks.DLR,  tx & hx, tx & hx, ks.AMPR, ks.ASTR, ks.PERC, ks.CIRC,
               ks.TILD, kc.QUOT, ks.UNDS,                   kc.MINS, ks.PLUS, kc.EQL,
                                 tt & ht, tt & ht, tt & ht, tt & ht,
    ] @ combos_base,
    # FUN
    [
               fak.BOOT, tt & ht, tt & ht, tt & ht, tt & ht, kc.F7,   kc.F8,   kc.F8,
      tt & ht, kc.F12,   kc.F11,  kc.F10,  tt & ht, tt & ht, kc.F4,   kc.F5,   kc.F6,   tt & ht,
               kc.F9,    kc.F8,   kc.F7,                     kc.F1,   kc.F2,   kc.F3,
                                  tt & ht, tt & ht, tt & ht, tt & ht,
    ] @ combos_base,
  ]
}
