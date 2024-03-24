let util = import "util_functions.ncl" in

fun ir =>

let as_sources = fun record => record
  |> std.record.filter (fun _k pred => pred)
  |> std.record.to_array
  |> std.array.map (fun e => e.field)
  |> util.array.join ","
in

let use_soft_serial = std.record.has_field "SPLIT_SOFT_SERIAL_PIN" ir.defines in

{
  split = ir.defines.SPLIT_ENABLE,
  extra_sources = as_sources 
  {
    "src/hold_tap.c" = ir.defines.HOLD_TAP_ENABLE,
    "src/tap_dance.c" = ir.defines.TAP_DANCE_ENABLE,
    "src/combo.c" = ir.defines.COMBO_COUNT > 0,
    "src/encoder.c" = ir.defines.ENCODER_COUNT > 0,
    "src/mouse.c" = ir.defines.MOUSE_KEYS_ENABLE,
    "src/macro.c" = ir.defines.MACRO_KEYS_ENABLE,
    "src/caps_word.c" = ir.defines.CAPS_WORD_ENABLE,
    "src/soft_serial.c" = use_soft_serial,
  },
  extra_periph_sources = as_sources
  {
    "src/soft_serial.c" = use_soft_serial,
  },
}
