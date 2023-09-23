let kb = import "keyboard.ncl" in
let { td, MO, DF, TG, TO, tap, hold, .. } = import "fak/keycode.ncl" in

let kc = tap.reg.kc in
let key_count = std.array.length kb.keys in

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
  kc.ENT & MO 2 & hold.reg.behavior B0,
  tap.custom.BOOT
] in

let XX = tap.none & hold.none in
let _T = tap.trans & hold.trans in
let kn = fun num => kc."%{std.to_string num}" in

[
  [
    kc.Q, kc.W, kc.E, kc.R, kc.T,
    kc.A, kc.S, kc.D, kc.F, kc.G,
    kc.Z, kc.X, kc.C, kc.V, kc.B,
              DF 1 & hold.reg.mod.lsft & hold.reg.behavior B0, TD0,
  ],
  [
    kc.Y, kc.U, kc.I, kc.O, kc.P,
    kc.H, kc.J, kc.K, kc.L, kc.";",
    kc.N, kc.M, kc.",", kc.".", kc."/" & tap.reg.mod.lsft,
              DF 0 & hold.reg.mod.lsft & hold.reg.behavior B0, TD0,
  ],
  [
    XX, kn 7, kn 8, kn 9, XX,
    XX, kn 4, kn 5, kn 6, XX,
    XX, kn 1, kn 2, kn 3, XX,
                    kn 0, _T,
  ]
]
