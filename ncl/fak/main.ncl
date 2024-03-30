fun _kb _km =>

let util = import "util_functions.ncl" in

let { Keyboard, .. } = import "keyboard.ncl" in
let kb | Keyboard = _kb in

let { Keymap, .. } = (import "keymap.ncl") kb _km in
let km | Keymap = _km in

let gen_ir = import "gen_ir.ncl" in
let gen_code = import "gen_code.ncl" in
let gen_meson_options = import "gen_meson_options.ncl" in

let is_split = std.record.has_field "split" kb in

let used_encoders = kb.encoders
  |> util.array.enumerate
  |> std.array.filter (fun { index, .. } => 
      std.array.any (fun vk => vk.type == 'encoder && vk.data.index == index) km.virtual_keys)
  |> std.array.map (fun { value, .. } => value)
in

let transformed_virtual_keys = km.virtual_keys
  |> std.array.map (fun vk =>
      if vk.type != 'encoder then vk else
        let enc_def = std.array.at vk.data.index kb.encoders in
        {
          type = 'encoder,
          data = vk.data & { index | force = util.array.index_of enc_def used_encoders },
        }
  )
in

let transformed_km = km & {
  virtual_keys | force = transformed_virtual_keys,
} in

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

  let periph_encoder_indices = used_encoders
    |> std.array.filter (fun e => e.type == 'peripheral)
    |> std.array.map (fun e => e.data)
  in

  let transformed_periph_encoders = periph_encoder_indices
    |> std.array.map (fun encoder_idx => std.array.at encoder_idx kb.split.peripheral.encoders)
  in

  let transformed_central_encoders = used_encoders
    |> std.array.map (fun e =>
        if e.type != 'peripheral then e else
        e & { data | force = util.array.index_of e.data periph_encoder_indices }
    )
  in

  let transformed_kb = kb & { 
    keys | force = transformed_central_keys,
    encoders | force = transformed_central_encoders,
    split.peripheral.keys | force = transformed_periph_keys,
    split.peripheral.encoders | force = transformed_periph_encoders,
  } in

  let central_ir = gen_ir transformed_kb transformed_km 'central in
  let peripheral_ir = gen_ir transformed_kb.split.peripheral {} 'peripheral in

  {
    central = central_ir |> gen_code,
    peripheral = peripheral_ir |> gen_code,
    meson_options = central_ir |> gen_meson_options,
  }

else
  let transformed_kb = kb & {
    encoders | force = used_encoders
  } in
  
  let ir = gen_ir transformed_kb transformed_km 'self in
  {
    central = ir |> gen_code,
    meson_options = ir |> gen_meson_options,
  }