let util = import "util_functions.ncl" in

let { Keyboard, .. } = import "keyboard.ncl" in
let kb | Keyboard = import "../keyboard.ncl" in

let _km = import "../keymap.ncl" in
let { Keymap, .. } = (import "keymap.ncl") kb _km in
let km | Keymap = _km in

let gen_ir = import "gen_ir.ncl" in
let gen_code = import "gen_code.ncl" in

let is_split = std.record.has_field "split" kb in

{
  central =
    let side = if is_split then 'central else 'self in
    gen_ir kb km side
    |> gen_code,
} & util.record.only_if is_split {
  peripheral =
    gen_ir kb.split.peripheral {} 'peripheral
    |> gen_code
}
