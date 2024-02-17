let { MAX_LAYER_COUNT, .. } = import "constants.ncl" in
let { Uint, Uint8, Uint16, BoundedInt, Set, .. } = import "util_types.ncl" in
let util = import "util_functions.ncl" in

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
let virtual_key_count = std.array.length (util.record.at_or km "virtual_keys" []) in
let key_count = physical_key_count + virtual_key_count in

let LayerIndex = BoundedInt 0 (std.array.length km.layers) in
let PhysicalKeyIndex = BoundedInt 0 physical_key_count in

let NonConditionalLayerIndex =
  let ValidIndex = std.contract.from_predicate (fun i =>
    let conditional_layer_indices =
      util.record.at_or km "conditional_layers" {}
      |> std.record.fields
      |> std.array.map std.string.to_number
    in
    !(std.array.elem i conditional_layer_indices)
  ) in
  std.contract.Sequence [ LayerIndex, ValidIndex ]
in

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

let rec Keycode = 
  let Holdable = {
    type | [| 'none, 'regular, 'nop, 'transparent |],
    data | (match {
      'none => {},
      'regular => {
        behavior | HoldTapBehavior,
        layer | NonConditionalLayerIndex | default = 0,
        mods | Modifiers | default = {}
      },
      'nop => {
        behavior | HoldTapBehavior,
      },
      'transparent => {}
    }) type
  } in

  let MacroSteps = 
    let MacroStep = {
      inst | [| 'press, 'release, 'tap, 'wait, 'pause_for_release |],
      arg | (match {
        'pause_for_release => { none | Uint 0 | default = 0 },
        'wait => { duration_ms | Uint16 },
        _ => { keycode | Keycode },
      }) inst
    } in

    let HasAtMostOnePauseForRelease = std.contract.from_predicate (fun value =>
      (value
        |> std.array.filter (fun step => step.inst == 'pause_for_release)
        |> std.array.length
      ) <= 1)
    in
    
    std.contract.Sequence [ Array MacroStep, HasAtMostOnePauseForRelease ]
  in

  let Tappable = {
    type | [| 'none, 'regular, 'sticky, 'layer, 'custom, 'transparent |],
    data | (match {
      'none => {},
      'regular => {
        hid_code | Uint8 | default = 0,
        mods | Modifiers | default = {}
      },
      'sticky => {
        mods | Modifiers | default = {},
      },
      'layer => {
        operation | [| 'df, 'tog, 'to |],
        layer | NonConditionalLayerIndex
      },
      'custom => {
        type | [| 'fak, 'consumer, 'user, 'mouse, 'macro |],
        data | (match {
          'macro => { steps | MacroSteps },
          _ => { code | Uint 10 },
        }) type
      },
      'transparent => {}
    }) type
  } in

  {
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

let ConditionalLayers = std.contract.from_predicate (fun cl => cl
  |> std.record.to_array
  |> std.array.all (fun { field, value } =>
      let then_layer | LayerIndex = std.string.to_number field in
      let if_layers | Set NonConditionalLayerIndex = value in
      !(std.array.elem then_layer if_layers)
  )
) in

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
      require_prior_idle_ms | Uint16 | default = 0,
    }
  }) type
} in

{
  Keymap = {
    layers | Layers,
    conditional_layers | ConditionalLayers | default = {},
    virtual_keys | Set VirtualKey | default = [],
    mouse = {
      move_speed | Uint8 | default = 4,
      scroll_interval_ms | Uint16 | default = 20,
    },
  }
}
