let util = import "util_functions.ncl" in

let { Keyboard, .. } = import "keyboard.ncl" in
let kb | Keyboard = import "../keyboard.ncl" in

let _km = import "../keymap.ncl" in
let { Keymap, .. } = (import "keymap.ncl") kb _km in
let km | Keymap = _km in

let gen_ir = import "gen_ir.ncl" in
let gen_code = import "gen_code.ncl" in
let gen_meson_options = import "gen_meson_options.ncl" in

let is_split = std.record.has_field "split" kb in

if is_split then
  # This preprocessing transformation is necessary in order to:
  # - Eliminate unused peripheral side keys
  # - Detect invalid peripheral side keys (e.g., S 9 assuming peripheral of 5 keys)
  # - Ensure both sides are on the same page about the order of key pressed bits

  let periph_key_indices = kb.keys
    |> std.array.filter (fun k => k.type == 'peripheral)
    |> std.array.map (fun k => k.data)
  in

  let transformed_periph_keys = periph_key_indices
    |> std.array.map (fun key_idx => std.array.at key_idx kb.split.peripheral.keys)
  in

  let transformed_central_keys = kb.keys
    |> std.array.map (fun k => 
          if k.type != 'peripheral then k else
          k & {data | force = util.array.index_of k.data periph_key_indices}
       )
  in

  let transformed_kb = kb & { 
    keys | force = transformed_central_keys,
    split.peripheral.keys | force = transformed_periph_keys,
  } in

  let central_ir = gen_ir transformed_kb km 'central in
  let peripheral_ir = gen_ir transformed_kb.split.peripheral {} 'peripheral in

  {
    central = central_ir |> gen_code,
    peripheral = peripheral_ir |> gen_code,
    meson_options = central_ir |> gen_meson_options,
  }

else
  let ir = gen_ir kb km 'self in
  {
    central = ir |> gen_code,
    meson_options = ir |> gen_meson_options,
  }