let { tap, hold, td, combo, .. } = import "../../fak/keycode.ncl" in
let util = import "../../fak/util_functions.ncl" in

let config = import "config.ncl" in
let layout = (import "layouts.ncl")."%{config.layout}" in

let ki = {
  hp = { decision = 'hold, trigger_on = 'press },
  tp = { decision = 'tap, trigger_on = 'press },
  hr = { decision = 'hold, trigger_on = 'release },
  tr = { decision = 'tap, trigger_on = 'release },
  xx = { decision = 'none },
} in

let combos = layout.combos in

let virtual_keys' =
  combos |> std.array.map (fun c =>
    combo.make config.combo_timeout_ms (std.array.split_at 1 c).right)
in

let physical_key_count = 28 in
let virtual_key_count = std.array.length virtual_keys' in
let key_count = physical_key_count + virtual_key_count in

let rec get_side_of_key_idx = fun key_idx =>
  if key_idx < physical_key_count then
    let right_indices = [4, 5, 6, 7, 12, 13, 14, 15, 20, 21, 22, 23, 26, 27] in
    if std.array.elem key_idx right_indices then 'right else 'left
  else
    let c = std.array.at (key_idx - physical_key_count) combos in
    let unique_sides = (std.array.split_at 1 c).right
      |> std.array.map get_side_of_key_idx
      |> util.array.unique in
    if std.array.length unique_sides > 1 then
      'both
    else
      std.array.first unique_sides
in

let make_hrm = fun key_idx =>
  let hrm_mods =
    let md = hold.reg.mod
  in [
    md.lctl, md.lalt, md.lgui, md.lsft, md.ralt,
    md.ralt, md.rsft, md.rgui, md.lalt, md.rctl,
  ] in

  let hrm_key_indices = [
    16, 8,  9,  10, 11,
    12, 13, 14, 15, 23,
  ] in

  let htb_hrm_base = {
    timeout_ms = 200,
    quick_tap_ms = 150,
    quick_tap_interrupt_ms = 500,
    global_quick_tap_ms = 100,
  } in

  let htb_hrm = fun side => htb_hrm_base & {
    key_interrupts =
      let hrm_key_indices_on_side = hrm_key_indices |>
        std.array.filter (fun i => get_side_of_key_idx i == side) in

      std.array.generate (fun key_idx =>
        let side_of_key_idx = get_side_of_key_idx key_idx in
        if side_of_key_idx == 'both then
          ki.xx
        else if side_of_key_idx != side then
          ki.hr
        else if std.array.elem key_idx hrm_key_indices_on_side then
          ki.tr
        else
          ki.tp
      ) key_count
  } in

  if std.array.elem key_idx hrm_key_indices then
    let mod_idx = util.array.index_of key_idx hrm_key_indices in
    let mod = std.array.at mod_idx hrm_mods in
    let side = get_side_of_key_idx key_idx in
    mod & hold.reg.behavior (htb_hrm side)
  else
    {}
in

let XXXX = tap.none & hold.none in
let virtual_base = combos |> std.array.map std.array.first in
let virtual_filler = combos |> std.array.map (fun c => XXXX) in

let layer_default =
  let base_keycodes = layout.base_layout
    |> std.string.characters
    |> util.array.enumerate
    |> std.array.map (fun { index, value } => tap.reg.kc."%{value}" & make_hrm index)
  in
  
  let thumb_keycodes =
    let htb_thumb = {
      timeout_ms = 200,
      quick_tap_ms = 150,
      quick_tap_interrupt_ms = 500,
      key_interrupts = std.array.replicate key_count ki.hr,
    } in

    layout.thumbs
    |> util.array.enumerate
    |> std.array.map (fun { index, value } =>
        value 
        & hold.reg.layer (index + 1)
        & hold.reg.behavior htb_thumb)
  in

  base_keycodes @ thumb_keycodes @ virtual_base
in

{
  virtual_keys = virtual_keys',
  layers = [layer_default] @ (
    ["NUM", "NAV", "SYM", "FUN"]
    |> std.array.map (fun name => layout.layers."%{name}")
    |> std.array.map (fun layer => layer @ virtual_filler)
  )
}
