# F.A. Keyboard Firmware

A keyboard firmware for the CH55x series.

## What the FAK?

FAK stands for F.A. Keyboard. F.A. are the initials of a person who silently crashed into my party and did not even make their presence known to me or my family. **You know who you are.**

## Why FAK?

I live in a country where Pi Picos and Pro Micros are not considered cheap. When I began the search for making keyboards as cheap as possible, I discovered CH55x chips which are *much* cheaper. RP2040 needs an external QSPI flash. ATmega32U4 needs an external clock. As for the CH55x however, all you need to get it running is the chip itself and two capacitors. You can get a complete MCU for less than a dollar.

Besides that, I want to be able to make keyboard configurations **declaratively**. ZMK already does this to an extent, but FAK is way more flexible as it uses a purely[^1] functional programming language designed for configs called [Nickel](https://nickel-lang.org/). This means you can do crazy things like layout parameterization, keycodes as variables, reusing and parameterizing parts of your keymap, and much more. In fact, Nickel is responsible for turning your config into C code!

[^1]: Almost pure. Side-effects are allowed but very constrained.

## Compatible hardware

Currently, it is only tested on the CH552T, but it should also work with CH552E/G. Other chips like the CH558 and CH559 will be supported soon to take advantage of their larger flash and memory.

# Getting started

Requirements:
- [`sdcc`](https://sourceforge.net/projects/sdcc/files)
- [`nickel` 1.2.1](https://github.com/tweag/nickel/releases/tag/1.2.1)
- [`python` 3.11+](https://www.python.org/downloads)
- [`meson`](https://mesonbuild.com/)
- [`ninja`](https://github.com/ninja-build/ninja/releases/tag/v1.11.1)
- [`wchisp`](https://github.com/ch32-rs/wchisp/releases/tag/nightly)

If you use Nix, you can simply use the provided flakes.

1. Clone this repo
1. Edit `ncl/keyboard.ncl` and `ncl/keymap.ncl`
1. `python fak.py flash`
1. `python fak.py flash_p` for the peripheral side if you have a split

**Note:** It is **recommended** you use `fak.py` instead of entering `meson` commands directly. The build system is streamlined such that Nickel automatically gives you the correct build options. However, Meson can't configure itself for this to work, and we wouldn't want to enter those build options manually everytime they change, so `fak.py` exists as a helper script that derives build options from Nickel then applies them to Meson.

**TL;DR:** Don't use Meson directly. `fak.py` is your friend. Makes life easy.

## Examples

Check out the examples and see how keyboards and keymaps are defined in FAK, and how powerful and crazy it can get.

1. [Beginner - 3x3 macropad](ncl/examples/3x3_macropad)
1. [Intermediate - cheapis](ncl/examples/cheapis)
1. [Advanced - zilpzalp](ncl/examples/zilpzalp)

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

Current limitations:
- Only UART0 is supported. UART1 is not yet supported.
- No bitbang implementation yet, which would allow for half-duplex single-wire comms on any[^2] pin at the cost of firmware size. Hardware UART is full-duplex two-wire and only available on certain pins.
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

Current limitations:
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

## Foolproof config

If you do something illegal like `hold.reg.layer 2` but you don't even have a layer 2, you'll get an error. It won't let you compile. Same thing if you try to mix incompatible building blocks like `tap.reg.kc.A & tap.trans & tap.custom.fak.BOOT`. Basically, assuming there's nothing wrong with your config's syntax, if you get an error from Nickel, then it's likely you did something that doesn't make sense or you've hit a hard limit (like defining layer 33).

This project is still at its very early stages though, so some error cases won't be caught yet. Not yet 100% foolproof. Also, the error messages don't look very helpful in the meantime.
