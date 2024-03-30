let { tap, hold, td, combo, encoder, .. } = import "../../fak/keycode.ncl" in

let kc = tap.reg.kc in
let md = hold.reg.mod in
let XXXX = tap.none & hold.none in
let TTTT = tap.trans & hold.trans in
let MO = hold.reg.layer in
let me = tap.custom.media in

let cu = {
  SFTZ = td.make 100 [
    md.lsft & kc.Z & hold.reg.behavior {},
    tap.reg.mod.lsft & kc.Z,
  ],
  TH_L = MO 1 & kc.SPC & hold.reg.behavior {},  
  TH_R = MO 2 & kc.SPC & hold.reg.behavior {},
  FN_G = MO 3 & kc.GRV & hold.reg.behavior {},
  BOOT = tap.custom.fak.BOOT,
  REST = tap.custom.fak.REST,
} in

{
  virtual_keys = [
    encoder.ccw 0,
    encoder.cw 0,
    encoder.ccw 1,
    encoder.cw 1,
    combo.make 255 [1, 2, 3],
    combo.make 50 [1, 2],
    combo.make 50 [11, 12],
  ],
  layers = [
    [
                 kc.Q,    kc.W,    kc.E,    kc.R,    kc.T,            kc.Y, kc.U, kc.I, kc.O, kc.P,
        cu.BOOT, kc.A,    kc.S,    kc.D,    kc.F,    kc.G,         kc.H,  kc.J, kc.K, kc.L, kc.QUOT, kc.SCLN,
        md.lctl, kc.Z,    kc.X,    kc.C,    kc.V,    kc.B,    kc.N,   kc.X,  kc.N, kc.M, kc.COMM, kc.DOT, kc.SLSH, md.lalt,
                          kc.N0,   kc.BSPC, md.lsft, md.lalt,                kc.N1, kc.N2, kc.SPC, kc.N4,
        me.PREV, me.NEXT, me.VOLD, me.VOLU,
        kc.Y, kc.Z, kc.G,
    ]
  ]
}
