let util = import "util_functions.ncl" in

fun kb km side =>

let keycodes = std.array.flatten km in
let layer_count = std.array.length km in

let deep_keycodes = std.array.flat_map (fun kc => match {
  'hold_tap => [kc],
  'tap_dance => kc.data.bindings
} kc.type) keycodes in

let keycode_is_hold_tap = fun kc => 
  kc.type == 'hold_tap
  && kc.data.tap.type != 'none
  && kc.data.hold.type != 'none
  && kc.data.hold.type != 'transparent in

let _hold_tap_behaviors =
  let hold_tap_keycodes = std.array.filter keycode_is_hold_tap deep_keycodes in
  let all_behaviors = std.array.map (fun kc => kc.data.hold.data.behavior) hold_tap_keycodes in
  util.array.unique all_behaviors in

let _tap_dance_bindings =
  let tap_dance_keycodes = std.array.filter (fun kc => kc.type == 'tap_dance) keycodes in
  let all_bindings = std.array.map (fun kc => kc.data.bindings) tap_dance_keycodes in
  util.array.unique all_bindings in

let encode_modifiers = fun mods => std.array.reduce_left (+) (std.array.map (fun { field, value } => 
  let M = [
    "left_ctrl", "left_shift", "left_alt", "left_gui",
    "right_ctrl", "right_shift", "right_alt", "right_gui",
  ] in
  (if value == true then 1 else 0) * (std.number.pow 2 (util.array.index_of field M))
) (std.record.to_array mods)) in

let encode_tappable = fun { type, data } => match {
  'none => 0,
  'regular => data.hid_code + util.bit.shift (encode_modifiers data.mods) 8,
  'layer => 
    let op = match { 'df => 0, 'tog => 1, 'to => 2 } data.operation in
    192 + data.layer + util.bit.shift op 8,
  'custom =>
    let m = util.bit.shift (util.bit.shift data.custom_code (-8)) 8 in
    224 + data.custom_code - m + util.bit.shift m 3,
  'transparent => 65535
} type in

let encode_holdable = fun { type, data } is_hold_tap => match {
  'none => 0,
  'regular =>
    (
      if is_hold_tap then
        util.bit.shift (util.array.index_of data.behavior _hold_tap_behaviors) 13
      else 0
    )
    + util.bit.shift data.layer 8
    + encode_modifiers data.mods,
  'nop => 8191 + util.bit.shift (util.array.index_of data.behavior _hold_tap_behaviors) 13,
  'transparent => 65535
} type in

let encode_hold_tap = fun { tap, hold } is_hold_tap => encode_tappable tap + util.bit.shift (encode_holdable hold is_hold_tap) 16 in

let encode_tap_dance = fun { tapping_term_ms, bindings, max_taps } =>
  let { Uint8, .. } = import "util_types.ncl" in
  let binding_start = std.array.fold_left (+) 0 (std.array.map (fun i => std.array.length (std.array.at i _tap_dance_bindings)) (std.array.range 0 (util.array.index_of bindings _tap_dance_bindings))) in
  util.bit.shift 14 28
  + util.bit.shift max_taps 20
  + util.bit.shift tapping_term_ms 8
  + (binding_start | Uint8) in

let encode_kc = fun kc => match {
  'hold_tap => encode_hold_tap kc.data (keycode_is_hold_tap kc),
  'tap_dance => encode_tap_dance kc.data,
} kc.type in

let encode_hold_tap_key_interrupt = fun { decision, trigger_on } => 
  if decision == 'none then 0 else (
    1 + (if decision == 'hold then 2 else 0)
    + (if trigger_on == 'press then 4 else 0)
  ) in

let encode_hold_tap_key_interrupts = fun key_interrupts =>
  std.array.map (fun pair =>
    (std.array.at 0 pair) + (util.bit.shift (util.array.at_or 1 0 pair) 4)
  ) (util.array.chunk 2 (std.array.map encode_hold_tap_key_interrupt key_interrupts)) in

let _central_defines = {
  KEY_COUNT = std.array.length kb.keys,
  LAYER_COUNT = layer_count,
  DEBOUNCE_MS = kb.debounce_ms,

  LAYER_TRANSPARENCY_ENABLE = std.array.any (fun kc => 
    kc.type == 'hold_tap
    && (kc.data.tap.type == 'transparent || kc.data.hold.type == 'transparent)
  ) deep_keycodes,

  HOLD_TAP_ENABLE = std.array.length _hold_tap_behaviors > 0,
  HOLD_TAP_EAGER_ENABLE = std.array.any (fun b => b.eager_decision != 'none) _hold_tap_behaviors,
  HOLD_TAP_QUICK_TAP_ENABLE = std.array.any (fun b => b.quick_tap_ms > 0) _hold_tap_behaviors,
  HOLD_TAP_QUICK_TAP_INTERRUPT_ENABLE = std.array.any (fun b => b.quick_tap_interrupt_ms > 0) _hold_tap_behaviors,
  HOLD_TAP_GLOBAL_QUICK_TAP_ENABLE = std.array.any (fun b => b.global_quick_tap_ms > 0) _hold_tap_behaviors,
  HOLD_TAP_GLOBAL_QUICK_TAP_IGNORE_CONSECUTIVE_ENABLE = std.array.any (fun b => b.global_quick_tap_ignore_consecutive) _hold_tap_behaviors,

  TAP_DANCE_ENABLE = std.array.any (fun kc => kc.type == 'tap_dance) keycodes,

  USB_VENDOR_ID = kb.usb_dev.vendor_id,
  USB_PRODUCT_ID = kb.usb_dev.product_id,
  USB_PRODUCT_VER = kb.usb_dev.product_ver,

  USB_EP0_SIZE = 8,
  USB_EP1_SIZE = 8,
  USB_EP_PADDING = 2,
  KEY_EVENT_QUEUE_LEN = 32,

  XADDR_USB_EP0 = 0,
  XADDR_USB_EP1 = XADDR_USB_EP0 + USB_EP0_SIZE + USB_EP_PADDING,
  XADDR_USB_EP1O = XADDR_USB_EP1,
  XADDR_USB_TX_LEN = XADDR_USB_EP1 + USB_EP1_SIZE + USB_EP_PADDING,
  XADDR_USB_EP1I = XADDR_USB_EP1 + 64,

  XADDR_LAST_TAP_TIMESTAMP = "(XADDR_USB_TX_LEN + sizeof(uint8_t))",
  XADDR_TAP_COUNT = "(XADDR_LAST_TAP_TIMESTAMP + sizeof(uint16_t))",

  XADDR_KEY_STATES = XADDR_USB_EP1I + USB_EP1_SIZE + USB_EP_PADDING,
  XADDR_KEY_EVENT_QUEUE = "(XADDR_KEY_STATES + sizeof(fak_key_state_t) * KEY_COUNT)",
} & util.record.only_if (layer_count > 1) {
  XADDR_LAYER_STATE = "(XADDR_TAP_COUNT + sizeof(fak_layer_state_t))",
  XADDR_PERSISTENT_LAYER_STATE = "(XADDR_LAYER_STATE + sizeof(fak_layer_state_t))",
} in

let _defines = {
  SPLIT_ENABLE = side != 'self,
  SPLIT_SIDE_CENTRAL = side != 'peripheral,
  SPLIT_SIDE_PERIPHERAL = side == 'peripheral,
} & util.record.only_if (side != 'self) {
  SPLIT_PERIPH_KEY_COUNT =
    (if side == 'central then kb.split.peripheral else kb).keys
    |> std.array.length,
} & util.record.only_if (side != 'peripheral)
  _central_defines
in

let encode_hold_tap_behavior = fun b => {
  flags =
    (if b.timeout_decision == 'hold then 1 else 0)
    + (match { 'none => 0, 'hold => 2, 'tap => 4 } b.eager_decision)
    + (if b.global_quick_tap_ignore_consecutive then 8 else 0),
  timeout_ms = b.timeout_ms,
  key_interrupts = encode_hold_tap_key_interrupts b.key_interrupts,
} & util.record.only_if _defines.HOLD_TAP_QUICK_TAP_ENABLE {
  quick_tap_ms = b.quick_tap_ms
} & util.record.only_if _defines.HOLD_TAP_QUICK_TAP_INTERRUPT_ENABLE {
  quick_tap_interrupt_ms = b.quick_tap_interrupt_ms
} & util.record.only_if _defines.HOLD_TAP_GLOBAL_QUICK_TAP_ENABLE {
  global_quick_tap_ms = b.global_quick_tap_ms
} in

let Kscan = std.contract.from_predicate (fun k =>
  let all_used_pins = k.ins @ k.outs in
  (util.array.unique all_used_pins) == all_used_pins
) in

let rec _kscan | Kscan = 
  let index_of_in = fun in_pin => util.array.index_of in_pin _kscan.ins in
  let index_of_out = fun out_pin => util.array.index_of out_pin _kscan.outs in
{
  ins = kb.matrix.cols
    @ (std.array.map (fun k => k.data) (std.array.filter (fun k => k.type == 'direct) kb.keys)),
  outs = kb.matrix.rows,
  matrix = {
    ins = std.array.map index_of_in kb.matrix.cols,
    outs = std.array.map index_of_out kb.matrix.rows,
    mapping =
      let force_check = std.array.any (fun k =>
        k.type == 'matrix 
        && k.data.col >= 0 
        && k.data.row >= 0
      ) kb.keys in
      if force_check then (
        std.array.generate (
          fun row => std.array.generate (
            fun col =>
              let k = {
                type = 'matrix,
                data.col = col,
                data.row = row,
              } in
              if std.array.any ((==) k) kb.keys then
                util.array.index_of k kb.keys
              else
                -1
          ) (std.array.length ins)
        ) (std.array.length outs)
      ) else [],
  },
  direct = std.array.map (fun { index, value } => {
    in_idx = index_of_in value.data,
    key_idx = index,
  }) (
    std.array.filter
      (fun { index, value } => value.type == 'direct)
      (util.array.enumerate kb.keys)
  ),
} in

{
  defines = _defines,
  kscan = _kscan,

  key_map = if side == 'peripheral then [] else
    std.array.map (fun layer => std.array.map encode_kc layer) km,

  hold_tap_behaviors = if side == 'peripheral then [] else
    std.array.map encode_hold_tap_behavior _hold_tap_behaviors,

  tap_dance_bindings = if side == 'peripheral then [] else
    std.array.map encode_kc (std.array.flatten _tap_dance_bindings),

  split_periph_key_indices = 
    if side != 'central then [] else
      kb.keys
      |> util.array.enumerate
      |> std.array.filter (fun { index, value } => value.type == 'peripheral)
      |> std.array.map (fun { index, value } => index)
}
