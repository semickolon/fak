let { tap, hold, td, combo, .. } = import "../../fak/keycode.ncl" in

let kc = tap.reg.kc in
let me = tap.custom.media in
let XXXX = tap.none & hold.none in
let TTTT = tap.trans & hold.trans in

{
  layers = [
    [
      kc.N0, kc.N1
    ],
  ]
}
