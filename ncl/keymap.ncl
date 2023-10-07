let kb = import "keyboard.ncl" in
let { td, MO, DF, TG, TO, tap, hold, combo, .. } = import "fak/keycode.ncl" in

let kc = tap.reg.kc in
let key_count = std.array.length kb.keys + 4 in

let B0 = {
  quick_tap_ms = 150,
  global_quick_tap_ms = 120,
  key_interrupts = std.array.replicate key_count { 
    decision = 'hold,
    trigger_on = 'press
  },
} in

let TD0 = td.make 200 [
  kc.SPC,
  kc.ENT & MO 1 & hold.reg.behavior B0,
  tap.custom.fak.BOOT
] in

let TD1 = td.make 320 [ kc.T, tap.custom.media.PLAY_PAUSE, tap.custom.fak.BOOT ] in

let XX = tap.none & hold.none in
let _T = tap.trans & hold.trans in
let kn = fun num => kc."%{std.to_string num}" in

{
  virtual_keys = [
    combo.make 255 [2, 3],
    combo.make 30 [3, 4],
    combo.make 30 [2, 4],
    combo.make 50 [12, 13, 14],
  ],
  layers = [
    [
      kc.Q, kc.W, kc.E, kc.R, kc.T,      kc.Y, kc.U, kc.I, kc.O, kc.P,
      kc.A, kc.S, kc.D, kc.F, kc.G,     kc.H, kc.J, kc.K, kc.L, kc.";",
      kc.Z, kc.X, kc.C, kc.V, kc.B,     kc.N, kc.M, kc.",", kc.".", kc."/",
                      kc.ESC, kc.BSPC,  kc.SPC, MO 1 & kc.ENT & hold.reg.behavior B0,
      kc.Y, kc.U, TD1, TD0
    ],
    std.array.replicate (key_count - 4) _T @ [kc.N1, kc.N2, kc.N3, kc.N4],
  ]
}
