let util = import "util_functions.ncl" in

fun ir =>

let layer_count = std.array.length ir.key_map in

let rec codegen = {
  val = {
    auto = fun value =>
      if std.is_record value then
        struct value
      else if std.is_array value then
        array value
      else
        std.to_string value,
    struct = fun record =>
      "{\n" ++ util.array.join "\n" (
        std.array.map (fun { field, value } =>
          ".%{field} = %{auto value},"
        ) (std.record.to_array record)
      ) ++ "\n}",
    array = fun arr => "{ %{util.array.join ", " (std.array.map auto arr)} }"
  },

  defines = fun defines =>
    let codegen_define = fun { field, value } => if value == false then "" else "#define %{field} %{
      if value == true then "" else std.to_string value
    }" in
    util.array.join "\n" (
      std.array.filter (fun s => s != "") (
        std.array.map codegen_define (std.record.to_array defines)
      )
    ),
  
  pin_def = fun i pin out =>
    let prefix = if out then "OUT" else "IN" in
    let port = std.to_string (std.number.truncate (pin / 10)) in
    let port_idx = std.to_string (pin % 10) in
    m%"
      #define %{prefix}%{std.to_string i}_PIN %{port_idx}
      SBIT(%{prefix}%{std.to_string i}, P%{port}_ADDR, %{prefix}%{std.to_string i}_PIN);
    "%,
} in

let h_file = m%"
  %{codegen.defines ir.defines}

  #include "keyboard.h"

  void keyboard_init_user();
  void keyboard_scan_user();
"% in

let side_central_defs =
m%"
  %{
    if layer_count == 0 then
      "// (No layers. This must be peripheral side.)"
    else if layer_count == 1 then
      "__code uint32_t key_map[KEY_COUNT] = %{codegen.val.array (std.array.at 0 ir.key_map)};"
    else
      "__code uint32_t key_map[LAYER_COUNT][KEY_COUNT] = %{codegen.val.array ir.key_map};"
  }

  %{
    if std.array.length ir.hold_tap_behaviors > 0 then
      m%"
        #include "hold_tap.h"
        __code fak_hold_tap_behavior_t hold_tap_behaviors[] = %{codegen.val.array ir.hold_tap_behaviors};
      "%
    else
      "// (No hold tap behaviors)"
  }

  %{
    if std.array.length ir.tap_dance_bindings > 0 then
      "__code uint32_t tap_dance_bindings[] = %{codegen.val.array ir.tap_dance_bindings};"
    else
      "// (No tap dance bindings)"
  }

  %{
    if std.array.length ir.split_periph_key_indices > 1 then
      "__code uint8_t split_periph_key_indices[SPLIT_PERIPH_KEY_COUNT] = %{codegen.val.array ir.split_periph_key_indices};"
    else
      "// (No split periph key indices)"
  }
"% in

let c_file =
  let key_state_inform = fun key_idx in_pin_idx =>
    "key_state_inform(%{std.to_string key_idx}, !IN%{std.to_string in_pin_idx});" in
  let set_out_pin = fun high out_pin_idx =>
    "OUT%{std.to_string out_pin_idx} = %{if high then "1" else "0"};" in
m%"
  #include "ch552.h"

  %{side_central_defs}

  %{
    util.array.join "\n" (
      std.array.map (fun { index, value } => codegen.pin_def index value false) (util.array.enumerate ir.kscan.ins)
    )
  }

  %{
    util.array.join "\n" (
      std.array.map (fun { index, value } => codegen.pin_def index value true) (util.array.enumerate ir.kscan.outs)
    )
  }

  void keyboard_init_user() {
  %{
    util.array.join "\n" (
      std.array.generate (fun i =>
        set_out_pin true i
      ) (std.array.length ir.kscan.outs)
    )
  }
  }

  static void row_switch_delay() {
    for (uint8_t i = 16; i; i--) {
      __asm__ ("nop");
    }
  }

  void keyboard_scan_user() {
  // Direct pins
  %{
    util.array.join "\n" (
      std.array.map (fun { in_idx, key_idx } =>
        key_state_inform key_idx in_idx
      ) ir.kscan.direct
    )
  } 

  // Matrix
  %{
    let matrix_row_scan = fun out_pin_idx row_mapping =>
      util.array.join "\n" ([
        set_out_pin false out_pin_idx,
        "row_switch_delay();",
      ]
      @ (
        std.array.map (fun { index, value } => 
          let in_pin_idx = std.array.at index ir.kscan.matrix.ins in
          let key_idx = value in
          key_state_inform key_idx in_pin_idx
        ) (
          std.array.filter (fun { index, value } =>
            value >= 0
          ) (
            util.array.enumerate row_mapping
          )
        )
      )
      @ [
        set_out_pin true out_pin_idx,
      ])
    in
    util.array.join "\n\n" (
      std.array.map
        (fun { index, value } => matrix_row_scan (std.array.at index ir.kscan.matrix.outs) value)
        (util.array.enumerate ir.kscan.matrix.mapping)
    )
  }
  }
"% in

{
  h = h_file,
  c = c_file,
}
