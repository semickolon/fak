let { Keyboard, .. } = import "keyboard.ncl" in
let kb | Keyboard = import "../keyboard.ncl" in

let _km = import "../keymap.ncl" in
let { Keymap, .. } = (import "keymap.ncl") kb _km in
let km | Keymap = _km in

let ir = (import "gen_ir.ncl") kb km in
let { h, c } = (import "gen_code.ncl") ir in

{
  user.h = h,
  user.c = c,
}
