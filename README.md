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

Requirements: `jq`, `meson`, `nickel 1.2.1`, `sdcc`, `ninja`, and your CH55x flasher tool of choice
If you use Nix, you can simply use the provided flakes.

1. Clone this repo
1. Edit `ncl/keyboard.ncl` and `ncl/keymap.ncl`
1. `meson setup build && cd build`
1. `ninja`
1. Flash `build/main.bin`

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
[
  [ # Layer 0
    kc.A & mod.lctl,       kc.B & mod.lsft,    kc.C & mod.lalt,         hold.reg.layer 1
  ],
  [ # Layer 1
    tap.trans & mod.lsft,  kc.J & hold.trans,  tap.trans & hold.trans,  tap.trans & hold.trans
  ]
]
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

## Foolproof config

If you do something illegal like `hold.reg.layer 2` but you don't even have a layer 2, you'll get an error. It won't let you compile. Same thing if you try to mix incompatible building blocks like `tap.reg.kc.A & tap.trans & tap.custom.BOOT`. Basically, assuming there's nothing wrong with your config's syntax, if you get an error from Nickel, then it's likely you did something that doesn't make sense or you've hit a hard limit (like defining layer 33).

This project is still at its very early stages though, so some error cases won't be caught yet. Not yet 100% foolproof. Also, the error messages might not look very helpful in the meantime.
