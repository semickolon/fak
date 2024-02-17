# 🎉 F.A. Keyboard Firmware

A keyboard firmware for the CH55x series.

🥳 **Come join our [Discord server](https://discord.gg/4Ev8GFZNR2)!**

<a href="https://www.buymeacoffee.com/semickolon"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=☕&slug=semickolon&button_colour=FFDD00&font_colour=000000&font_family=Lato&outline_colour=000000&coffee_colour=ffffff" /></a>

## What the FAK?

FAK stands for F.A. Keyboard. F.A. are the initials of a person who silently crashed into my party and did not even make their presence known to me or my family. **You know who you are.**

## Why FAK?

I live in a country where Pi Picos and Pro Micros are not considered cheap. When I began the search for making keyboards as cheap as possible, I discovered CH55x chips which are *much* cheaper. RP2040 needs an external QSPI flash. ATmega32U4 needs an external clock. As for the CH55x however, all you need to get it running is the chip itself and two capacitors. You can get a complete MCU for less than a dollar.

Besides that, I want to be able to make keyboard configurations **declaratively**. ZMK already does this to an extent, but FAK is way more flexible as it uses a purely[^1] functional programming language designed for configs called [Nickel](https://nickel-lang.org/). This means you can do crazy things like layout parameterization, keycodes as variables, reusing and parameterizing parts of your keymap, and much more. In fact, Nickel is responsible for turning your config into C code!

[^1]: Almost pure. Side-effects are allowed but very constrained.

## Compatible hardware

Currently, it has been tested on CH552T, CH552G. It hasn't been tested on CH552E but it should work. CH558 and CH559 will be supported soon to take advantage of their larger flash and memory.

# Getting started

Requirements:
- [`sdcc` 4.2.0](https://sourceforge.net/projects/sdcc/files)
- [`nickel` 1.3.0](https://github.com/tweag/nickel/releases/tag/1.3.0)
- [`python` 3.11](https://www.python.org/downloads)
- [`meson` 1.2.3](https://mesonbuild.com/)
- [`ninja` 1.11.1](https://github.com/ninja-build/ninja/releases/tag/v1.11.1)
- [`wchisp`](https://github.com/ch32-rs/wchisp/releases/tag/nightly)

Versions listed above are from the Nix flake, representative of my dev environment. You can probably use versions later than these just fine, but if you encounter problems, having these exact versions might help.

If you use Nix, you can simply use the provided flake.

1. Clone this repo
1. Edit `ncl/keyboard.ncl` and `ncl/keymap.ncl`
1. `python fak.py flash`
1. `python fak.py flash_p` for the peripheral side if you have a split

**Note:** It is **recommended** you use `fak.py` instead of entering `meson` commands directly. The build system is streamlined such that Nickel automatically gives you the correct build options. However, Meson can't configure itself for this to work, and we wouldn't want to enter those build options manually everytime they change, so `fak.py` exists as a helper script that derives build options from Nickel then applies them to Meson.

**TL;DR:** Don't use Meson directly. `fak.py` is your friend. Makes life easy.

## Dev Container

For easier setup, you can also use the provided Dev Container.

1. Clone this repo
1. Create a new codespace
1. Wait for the environment to be loaded in the terminal (3-6 minutes) until you can enter commands

The only thing that won't work from a remote setup is, of course, flashing. You'll have to do that locally with `wchisp`. Thankfully, `wchisp` provides prebuilt binaries so getting that set up is very easy.

1. In your codespace, `python fak.py compile`
1. Download the `.ihx` file(s) in `build/` to your local machine
1. In your local machine, `wchisp flash central.ihx`
1. And then if you have a split, `wchisp flash peripheral.ihx`

## Examples

Check out the examples and see how keyboards and keymaps are defined in FAK, and how powerful and crazy it can get.

1. [Beginner - 3x3 macropad](ncl/examples/3x3_macropad)
1. [Intermediate - cheapis](ncl/examples/cheapis)
1. [Advanced - zilpzalp](ncl/examples/zilpzalp)

# Projects

Let me know if you're using FAK on a project and I'd be happy to add it here!

- [MIAO by kilipan](https://github.com/kilipan/miao). Drop-in replacement CH552T MCU for the Seeed Studio XIAO series. Keeb Supply sells a production version of the Miao, which you can [purchase here](https://keeb.supply/products/miao). (Not affiliated/sponsored)
- [CH552-44, CH552-48, CH552-48-LPR by rgoulter](https://github.com/rgoulter/keyboard-labs#ch552-44-low-budget-hand-solderable-pcb-in-bm40jj40-form-factor). Ortholinear keebs built on WeAct board and PCBA'd standalone CH552T.
- [CH552-36 by rgoulter](https://github.com/rgoulter/keyboard-labs#ch552-36-low-budget-36-key-split-keyboard-with-smt-components). Split keyboard built on a sub-100x100 mm2 PCB with SMT components.
- [Ch55p34 by doesntfazer](https://github.com/doesntfazer/Ch55p34-keyboard). 34-key column-staggered unibody keyboard with standalone CH552T.

# Features

## Layers

Yep. Layers. Up to 32.

## Composable keycodes

Keycodes are in 32 bits. 16 for the hold portion. 16 for the tap portion. If both portions exist, you get a hold-tap.

```
# A
tap.reg.kc.A

# Ctrl-A
tap.reg.kc.A & tap.reg.mod.lctl

# Ctrl-A when tapped, Layer 1 (MO(1) in QMK) when held
# Both portions exist. This is a hold-tap.
tap.reg.kc.A & tap.reg.mod.lctl & hold.reg.layer 1

# Ctrl-Shift-A when tapped, Layer 1 with Alt and Shift when held
tap.reg.kc.A & tap.reg.mod.lctl & tap.reg.mod.lsft & hold.reg.layer 1 & hold.reg.mod.lalt & hold.reg.mod.lsft

# Layer 1 when pressed/held
# Only hold portion exists. This is not a hold-tap.
hold.reg.layer 1
```

## Partial and full transparency

Either one or both of the tap and hold portions can be transparent. Full transparency is equivalent to `KC_TRNS` in QMK.

```
let kc = tap.reg.kc in
let mod = hold.reg.mod in
{
  layers = [
    [ # Layer 0
      kc.A & mod.lctl,       kc.B & mod.lsft,    kc.C & mod.lalt,         hold.reg.layer 1
    ],
    [ # Layer 1
      tap.trans & mod.lsft,  kc.J & hold.trans,  tap.trans & hold.trans,  tap.trans & hold.trans
    ]
  ]
}
```

## Complex hold-tap behaviors

Inspired by and building on top of ZMK.

- You can choose a flavor *per key*. Tap-preferred on key 3, hold-preferred on key 42, and balanced on key 69? No problem!
- Quick tap. Allows hold-taps to remain resolved as a tap if tapped then held quickly.
- Quick tap interrupt. Allows a quick tap to re-resolve to a hold if interrupted by another key press.
- Global quick tap. Allows hold-taps to resolve as a tap during continuous typing.
- Global quick tap ignore consecutive. Ignores global quick tap if the same key is pressed at least twice in a row.
- Eager decision. Allows hold-taps to pre-resolve as a hold or a tap until the actual decision is made. If the actual decision doesn't match the eager decision, the eager is reversed then the actual is applied. Otherwise, if the decisions match, nothing else happens because it's as if the hold-tap predicted the future.

```
let my_crazy_behavior = {
  timeout_ms = 200,          # Known as tapping term in QMK and ZMK. 200 by default.
  timeout_decision = 'hold,  # Can be set to 'hold (default) or 'tap
  eager_decision = 'tap,     # Can be set to 'hold, 'tap, or 'none (default)
  key_interrupts = [         # Size of this must match key count. This assumes we have 5 keys.
    { decision = 'hold, trigger_on = 'press },      # Similar to ZMK's hold-preferred / QMK's HOLD_ON_OTHER_KEY_PRESS
    { decision = 'tap, trigger_on = 'press },
    { decision = 'hold, trigger_on = 'release },    # Similar to ZMK's balanced / QMK's PERMISSIVE_HOLD
    { decision = 'tap, trigger_on = 'release },
    { decision = 'none },                           # Similar to ZMK's tap-preferred
  ],
  quick_tap_ms = 150,              # 0 (disabled) by default
  quick_tap_interrupt_ms = 500,    # 0 (disabled) by default
  global_quick_tap_ms = 120,       # 0 (disabled) by default
  global_quick_tap_ignore_consecutive = true,     # false by default
} in

# Behaviors are bound to hold-taps like so
let my_crazy_key =
  tap.reg.kc.A
  & hold.reg.layer 1
  & hold.reg.behavior my_crazy_behavior
in
```

Things like retro-tap and ZMK's `tap-unless-interrupted` are not defined as-is. They don't need to be because you can make behaviors that emulate them. The exercise of doing so is left to the reader.

## Tap dance

Similar to ZMK, the bindings can be hold-taps! This means in the following example, if you do a tap-tap-hold, you'll momentarily access layer 2.

```
# 200 is the tapping_term_ms
td.make 200 [
  tap.reg.kc.F,
  tap.reg.kc.A & hold.reg.layer 1,
  tap.reg.kc.K & hold.reg.layer 2,
]
```

## Split support

Central and peripheral sides are *fully independently* defined, so no considerations need to be made about symmetry, pin placement, and whatnot. They can be two entirely different keyboards connected together for all FAK cares.

```
# This is an example of how a 10-key "split macropad" would be defined in keyboard.ncl

let { DirectPinKey, PeripheralSideKey, .. } = import "fak/keyboard.ncl" in
let { CH552T, .. } = import "fak/mcus.ncl" in

let D = DirectPinKey in
let S = PeripheralSideKey in

let side_periph = {
  mcu = CH552T,
  split.channel = CH552T.features.uart_30_31,
  keys = [
    D 13, D 14, D 15,
    D 32, D 33, D 12,
  ]
} in

# The central side has two fields that aren't in the peripheral:
# `split.peripheral` and `usb_dev`
{
  mcu = CH552T,
  split.channel = CH552T.features.uart_12_13,
  split.peripheral = side_periph,
  usb_dev = {
    # Nickel doesn't support hex literals yet
    vendor_id = 2023,
    product_id = 69,
    product_ver = 420,
  },
  keys = [
    D 14, D 15,   S 0, S 1, S 2,
    D 30, D 11,   S 3, S 4, S 5,
  ]
}
```

Limitations:
- Only UART0 is supported. UART1 is not yet supported.
- Central and peripheral sides are fixed. That is, you can't plug it in on the peripheral side. Well, you can, but of course it won't work as a USB keyboard.

[^2]: Almost any pin. USB data pins obviously won't work here.

## Combos

Combos are implemented as *virtual keys*. They're like regular keys but they are activated by pressing multiple physical keys at the same time. And since they're like regular keys, they're just like any other key in your keymap with full support for all the features. They can even have different keycodes across layers.

```
let kc = tap.reg.kc in
let mod = hold.reg.mod in
let my_tap_dance = td.make 200 [kc.SPC, kc.ENT, hold.reg.layer 1] in
{
  virtual_keys = [
    # The first argument is the timeout_ms (up to 255)
    # The second argument is the key indices/positions (min 2, max 9 keys)
    combo.make 50 [2, 3],

    # By default, the combo is released once *any one* of the key indices is released
    # Merge with `combo.slow_release` if you want the combo to release once *all* of the key indices are released instead
    combo.make 30 [0, 2, 5] & combo.slow_release,

    # During fast typing, you might want to temporarily ignore combos to prevent triggering them
    # To do this, merge with `combo.require_prior_idle_ms [ms]`
    # For example, the following combo is ignored if the last keypress happened no more than 180ms ago.
    combo.make 60 [0, 1] & combo.require_prior_idle_ms 180

    # You can't use virtual key indices. Just physical keys.
  ],
  # Assuming a 6-key macropad + 3 virtual keys, our layers need to have 9 keycodes.
  layers = [
    [
      kc.A, kc.B, kc.C,
      kc.D, kc.E, kc.F,
      # After physical keycodes, our virtual keycodes begin:
      kc.N1 & mod.lctl,
      kc.N2 & mod.lalt,
      kc.N3,
    ],
    [
      kc.G, kc.H, kc.I,
      kc.J, kc.K, kc.L,
      # Like physical keys, they can be different across layers and use transparency
      my_tap_dance,
      tap.trans & mod.lgui,
      kc.N4,
    ],
  ],
}
```

Limitations:
- Fully overlapping combos (e.g., `[2, 3]` and `[2, 3, 4]`) are not supported yet. Partially overlapping combos (e.g., `[2, 3]` and `[3, 4, 5]`) are supported though, as shown in the example above.

## Sticky mods

Tired of holding shift for just one key? What about tapping shift then *only* the next key press gets shifted? That's what sticky mods are all about and more.

```
let sticky_shift = tap.sticky.mod.lsft in

# It doesn't have to be just shift. You can mix and match mods as shown below.
# When you press two sticky mods in a row, they get stacked or combined, waiting for the next key press.

let sticky_gui_alt = tap.sticky.mod.lgui & tap.sticky.mod.lalt in
let sticky_ctrl_shift = tap.sticky.mod.lctl & tap.sticky.mod.lsft in
```

Another use case for sticky mods that I find very useful and personally use is this:

```
# Normal shift on hold, sticky shift on tap!
let best_shift_ever =
  tap.sticky.mod.lsft
  & hold.reg.mod.lsft
  & hold.reg.behavior { ... }
```

## Mouse keys

Yep. Mouse keys. Constant speed.

```
# Mouse buttons
tap.custom.mouse.BTN1   # Left mouse button
tap.custom.mouse.BTN2   # Right mouse button
tap.custom.mouse.BTN3   # Middle mouse button
# This goes up to BTN8, if you have a use case for that

# Mouse movement
tap.custom.mouse.UP
tap.custom.mouse.DOWN
tap.custom.mouse.LEFT
tap.custom.mouse.RGHT

# Mouse wheel
tap.custom.mouse.WH_U   # Scroll up
tap.custom.mouse.WH_D   # Scroll down
```

You can customize mouse settings in your keymap defintion.

```
{
  mouse = {
    move_speed = 4,           # Higher = faster move
    scroll_interval_ms = 20,  # Lower = faster scroll
  },
  layers = ...
}
```

## Caps word

Yep. Caps word.

- When active, tapping any key except the alphabetical keys, numbers, backspace, delete deactivates caps word state.
- When active, tapping alphabetical keys sends the shifted alphabetical keys; tapping `-` sends `_`.
- Caps word state only remains active if a key was tapped in the previous 5 seconds.

```
tap.custom.fak.CWTG     # Caps word toggle
tap.custom.fak.CWON     # Caps word on
tap.custom.fak.CWOFF    # Caps word off
```

## Macros

Yep. Macros.

```
# Macro for word selection
# Ctrl+Right, Ctrl+Left, Ctrl+Shift+Right

let kc = tap.reg.kc in
let tm = tap.reg.mod in
let md = hold.reg.mod in

let word_select = macro.make [
  macro.press md.lctl,
  macro.tap kc.RGHT,
  macro.tap kc.LEFT,
  macro.tap (kc.RGHT & tm.lsft),
  macro.release md.lctl,
] in
```

Here are the following possible steps or instructions that can go into `macro.make [...]`.

```
macro.press [keycode]
macro.release [keycode]
macro.tap [keycode]
macro.wait [duration in ms, up to 65535]
macro.pause_for_release
```

- `tap` does a `press` then a `release` condensed into one step.
- `pause_for_release` waits for the macro key to be released then runs the steps after it. There can be at most one of this in a macro. Two or more will lead to heat death of the universe.

Parameterizing macros is immediately possible thanks to Nickel, so there is no need to learn any other constructs. The following is an example that emulates[^3] `SEND_STRING` from QMK. Unlike QMK, this is not a C macro. It's simply a Nickel function that takes in a string and returns a macro.

```
let macro_send_string = fun str =>
  let steps =
    std.string.uppercase str
    |> std.string.characters
    |> std.array.map (fun char => macro.tap tap.reg.kc."%{char}")
  in
  macro.make steps
in

let my_macro_1 = macro_send_string "fak yeah" in
let my_macro_2 = macro_send_string "@gmail.com" in
```

As of writing, there are no checks enforced in Nickel to check if all your `press`es are eventually `release`d. That is, it's possible to leave your `press`es pressed even after the macro is fully done. Take note of this, especially if you have weird behavior after activating a macro. This is all because I honestly don't know if checks should even be enforced or if there are actual use cases for leaving keys pressed after a macro.

[^3]: To keep the example short, it does not *fully* emulate `SEND_STRING`. Most notably, it lacks support for shifted characters/keys like `ABC` and `!^$`.

## Conditional layers

Inspired by ZMK, this is a generalization of what's usually known as "tri-layers" where activating at least two specified layers (commonly known as "lower" and "raise") activate another layer ("adjust").

```
{
  conditional_layers = {
    # The key ("3") specifies the layer that will be activated when the
    # specified layers ([1, 2]) are all activated
    "3" = [1, 2],

    # Nickel does not support integers as keys, as far as I know
    # So that's why we have to use a string for now

    # There can be more than two activating layers. Go crazy.
    "9" = [3, 5, 7, 8],
  },
  layers = ...
}
```

Now, conditional layers (such as 3 and 9 in the example above) are fully controlled by the firmware and FAK doesn't want your grubby little fingers on them. Checks are enforced in Nickel, so you cannot activate conditional layers yourself any other way, like `hold.reg.layer X` where X is a conditional layer.

## Repeat key

`tap.custom.fak.REP` repeats the last reported non-modifier keycode including modifiers when it was pressed. Basically, tapping a modifier like Ctrl alone will not be repeated by the repeat key, but Ctrl-A will be, since it has a non-modifier keycode (A).

## Foolproof config

If you do something illegal like `hold.reg.layer 2` but you don't even have a layer 2, you'll get an error. It won't let you compile. Same thing if you try to mix incompatible building blocks like `tap.reg.kc.A & tap.trans & tap.custom.fak.BOOT`. Basically, assuming there's nothing wrong with your config's syntax, if you get an error from Nickel, then it's likely you did something that doesn't make sense or you've hit a hard limit (like defining layer 33).
