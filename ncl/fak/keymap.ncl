let { MAX_LAYER_COUNT, .. } = import "constants.ncl" in
let { Uint, Uint8, Uint16, BoundedInt, Set, .. } = import "util_types.ncl" in

let Uint13 = Uint 13 in

fun kb km =>

let Modifiers = {
  left_ctrl | Bool | default = false,
  left_shift | Bool | default = false,
  left_alt | Bool | default = false,
  left_gui | Bool | default = false,
  right_ctrl | Bool | default = false,
  right_shift | Bool | default = false,
  right_alt | Bool | default = false,
  right_gui | Bool | default = false,
} in

let physical_key_count = std.array.length kb.keys in
let virtual_key_count = std.array.length km.virtual_keys in
let key_count = physical_key_count + virtual_key_count in

let LayerIndex = BoundedInt 0 (std.array.length km.layers) in
let PhysicalKeyIndex = BoundedInt 0 physical_key_count in

let HoldTapKeyInterrupt = {
  decision | [| 'none, 'hold, 'tap |] | default = 'none,
  trigger_on | [| 'press, 'release |] | default = 'press
} in

let HoldTapKeyInterrupts = (
  let ValidLength = std.contract.from_predicate (fun ints =>
    (std.array.length ints) == key_count
  ) in
  std.contract.Sequence [ Array HoldTapKeyInterrupt, ValidLength ]
) in

let HoldTapBehavior = let
  default_key_interrupts = std.array.replicate key_count {}
in {
  timeout_decision | [| 'hold, 'tap |] | default = 'hold,
  timeout_ms | Uint16 | default = 200,
  eager_decision | [| 'none, 'hold, 'tap |] | default = 'none,
  key_interrupts | HoldTapKeyInterrupts | default = default_key_interrupts,
  quick_tap_ms | Uint8 | default = 0,
  quick_tap_interrupt_ms
    | (if quick_tap_ms > 0 then Uint16 else std.contract.Equal 0)
    | default = 0,
  global_quick_tap_ms | Uint16 | default = 0,
  global_quick_tap_ignore_consecutive
    | (if global_quick_tap_ms > 0 then Bool else std.contract.Equal false)
    | default = false,
} in

let Holdable = {
  type | [| 'none, 'regular, 'nop, 'transparent |],
  data | (match {
    'none => {},
    'regular => {
      behavior | HoldTapBehavior,
      layer | LayerIndex | default = 0,
      mods | Modifiers | default = {}
    },
    'nop => {
      behavior | HoldTapBehavior,
    },
    'transparent => {}
  }) type
} in

let Tappable = {
  type | [| 'none, 'regular, 'layer, 'custom, 'transparent |],
  data | (match {
    'none => {},
    'regular => {
      hid_code | Uint8 | default = 0,
      mods | Modifiers | default = {}
    },
    'layer => {
      operation | [| 'df, 'tog, 'to |],
      layer | LayerIndex
    },
    'custom => {
      type | [| 'fak, 'consumer, 'user |],
      code | Uint 10
    },
    'transparent => {}
  }) type
} in

let rec Keycode = {
  type | [| 'hold_tap, 'tap_dance |],
  data | (match {
    'hold_tap => {
      hold | Holdable | default = { type = 'none, data = {} },
      tap | Tappable | default = { type = 'none, data = {} }
    },
    'tap_dance => {
      tapping_term_ms | Uint 12 | default = 200,
      bindings | Array Keycode,
      max_taps | std.contract.Sequence [ 
        BoundedInt 2 17,
        std.contract.Equal (std.array.length bindings)
      ] | default = (std.array.length bindings)
    }
  }) type
} in

let Layer = (
  let ValidLength = std.contract.from_predicate (fun layer =>
    (std.array.length layer) == key_count
  ) in
  std.contract.Sequence [ Array Keycode, ValidLength ]
) in

let Layers = 
  let ValidLength = std.contract.from_predicate (fun layers =>
    let len = std.array.length layers in len > 0 && len <= MAX_LAYER_COUNT
  ) in
  std.contract.Sequence [ Array Layer, ValidLength ]
in

let ComboKeyIndices =
  let ValidLength = std.contract.from_predicate (fun key_indices =>
    let len = std.array.length key_indices in len >= 2 && len <= 9
  ) in
  std.contract.Sequence [ Set PhysicalKeyIndex, ValidLength ]
in

let VirtualKey = {
  type | [| 'combo |],
  data | (match {
    'combo => {
      timeout_ms | Uint8 | default = 50,
      key_indices | ComboKeyIndices,
      slow_release | Bool | default = false,
    }
  }) type
} in

{
  Keymap = {
    layers | Layers,
    virtual_keys | Set VirtualKey | default = [],
  }
}
