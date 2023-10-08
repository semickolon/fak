**Level: Advanced 🥵**

Based on kilipan's zilpzalp example keymaps: https://github.com/kilipan/zilpzalp/tree/main/example_keymaps

- Three highly modularized base layouts: Qwerty, Colemak, and Aptmak. More layouts can be easily added.
- Code is reused as much as possible. Common traits between all layouts are shared.
- Home row mod behaviors are automatically generated based on its side (left/right half) and location on the keyboard. It automatically configures bilateral combinations and allows for combining more than one mod. The way it's configured also allows for flexibility, such as reassigning where the home row mods are (e.g., top/bottom row mods) and the order of mods (e.g., GACS, CSAG) while being completely decoupled to the base layout.
- Combo definition is simplified in `layouts.ncl` and `keymap.ncl` does all the heavy lifting. The auto home row mod behaviors even assign the correct key interrupt for combos (tap on press if all combo keys are on the same side, hold on release if all combo keys are on the other side, no interrupt if combo keys are on both sides).
- This is an example of how you can reuse and share code across layouts, keymaps, and even keyboards. Heck, you can even develop this further to a fully-fledged FAK userspace library. Like [QMK userspaces](https://github.com/qmk/qmk_firmware/blob/master/docs/feature_userspace.md) but on functional programming steroids.
