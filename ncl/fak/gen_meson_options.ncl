let util = import "util_functions.ncl" in

fun ir =>

{
  split = ir.defines.SPLIT_ENABLE,
  extra_sources = 
    {
      "src/hold_tap.c" = ir.defines.HOLD_TAP_ENABLE,
      "src/tap_dance.c" = ir.defines.TAP_DANCE_ENABLE,
      "src/combo.c" = ir.defines.COMBO_COUNT > 0,
      "src/mouse.c" = ir.defines.MOUSE_KEYS_ENABLE,
      "src/macro.c" = ir.defines.MACRO_KEYS_ENABLE,
      "src/caps_word.c" = ir.defines.CAPS_WORD_ENABLE,
    }
    |> std.record.filter (fun _k pred => pred)
    |> std.record.to_array
    |> std.array.map (fun e => e.field)
    |> util.array.join ",",
}
