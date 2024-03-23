let { tap, hold, td, combo, encoder, .. } = import "../../fak/keycode.ncl" in

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
  BOOT = tap.custom.fak.BOOT,
} in

{
  virtual_keys = [
    combo.make 50 [0, 1],
    encoder.cw 1,
    combo.make 50 [0, 2],
    encoder.ccw 1,
  ],
  layers = [
    [
      kc.Q, kc.W, kc.E, kc.R,
      kc.A, kc.S, kc.D, kc.F,
      kc.N0, kc.J, kc.N1, kc.K,
    ],
  ]
}
