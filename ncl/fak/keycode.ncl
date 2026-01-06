let util = import "util_functions.ncl" in
let hid_codes = import "hid_codes.ncl" in

let mod_ = fun K => let K = fun mod => K { mods."%{mod}" = true } in {
  lctl = K "left_ctrl",
  lsft = K "left_shift",
  lalt = K "left_alt",
  lgui = K "left_gui",
  rctl = K "right_ctrl",
  rsft = K "right_shift",
  ralt = K "right_alt",
  rgui = K "right_gui",
} in

{
  tap = let 
    K = fun t d => { type = 'hold_tap, data.tap = { type = t, data = d } }
  in {
    none = K 'none {},
    reg = {
      kc = let _K = fun x => K 'regular { hid_code = x } in
        hid_codes
        |> util.array.enumerate
        |> std.array.flat_map (fun { index, value } =>
            value
            |> std.array.map (fun v => { "%{v}" = _K index })
          )
        |> std.array.reduce_left (&),
      mod = mod_ (K 'regular),
      ks = {
        CIRC = kc.N6,
        DLR = kc.N4,
        TILD = kc.GRV,
        PIPE = kc.BSLS,
        PERC = kc.N5,
        AMPR = kc.N7,
        ASTR = kc.N8,
        HASH = kc.N3,
        AT = kc.N2,
        DQUO = kc.QUOT,
        LCBR = kc.LBRC,
        RCBR = kc.RBRC,
        LPRN = kc.N9,
        RPRN = kc.N0,
        LABK = kc.COMM,
        RABK = kc.DOT,
        QUES = kc.SLSH,
        UNDS = kc.MINS,
        PLUS = kc.EQL,
        COLN = kc.SCLN,
        EXLM = kc.N1,
      }
      |> std.record.map_values ((&) mod.lsft),
    },
    sticky = {
      mod = mod_ (K 'sticky),
      layer = fun i => K 'sticky { layer = i },
    },
    layer = let K = fun i op => K 'layer { layer = i, operation = op } in {
      DF = fun i => K i 'df,
      TG = fun i => K i 'tog,
      TO = fun i => K i 'to,
    },
    custom = let K = fun t c => K 'custom { type = t, data.code = c } in {
      fak = let K = fun code => K 'fak code in {
        REST = K 0,
        BOOT = K 1,
        CWON = K 2,
        CWOF = K 3,
        CWTG = K 4,
        REP  = K 5,
      },
      media = let K = fun code => K 'consumer code in {
        PLAY = K 205,
        MUTE = K 226,
        VOLU = K 233,
        VOLD = K 234,
        NEXT = K 181,
        PREV = K 182,
        FASTFWD = K 179,
        REWIND = K 180,
      },
      mouse = let K = fun code => K 'mouse code in {
        BTN1 = K 0,
        BTN2 = K 1,
        BTN3 = K 2,
        BTN4 = K 3,
        BTN5 = K 4,
        BTN6 = K 5,
        BTN7 = K 6,
        BTN8 = K 7,
        RGHT = K 8,
        LEFT = K 9,
        DOWN = K 10,
        UP =   K 11,
        WH_U = K 12,
        WH_D = K 13,
      },
    },
    tlex = K 'transparent_layer_exit {},
    trans = K 'transparent {},
  },
  
  hold = let
    K = fun t d => { type = 'hold_tap, data.hold = { type = t, data = d } }
  in {
    none = K 'none {},
    reg = {
      behavior = fun b => K 'regular { behavior = b },
      layer = fun i => K 'regular { layer = i },
      mod = mod_ (K 'regular),
    },
    nop = fun b => K 'nop { behavior = b },
    tlex = fun b => K 'transparent_layer_exit { behavior = b },
    trans = K 'transparent {}
  },
  
  td = {
    make = fun t b => {
      type = 'tap_dance,
      data = {
        tapping_term_ms = t,
        bindings = b
      }
    }
  },

  combo = {
    make = fun t k => {
      type = 'combo,
      data = {
        timeout_ms = t,
        key_indices = k,
      },
    },
    slow_release = { data.slow_release = true },
    require_prior_idle_ms = fun ms => { data.require_prior_idle_ms = ms },
  },

  encoder = 
    let make = fun i dir => {
      type = 'encoder,
      data = { index = i, direction = dir },
    } in
  {
    cw = fun i => make i 'cw,
    ccw = fun i => make i 'ccw,
  },

  macro = {
    make = fun steps => {
      type = 'hold_tap,
      data.tap = {
        type = 'custom,
        data = {
          type = 'macro,
          data.steps = steps,
        },
      },
    },
    press = fun kc => { inst = 'press, arg.keycode = kc },
    release = fun kc => { inst = 'release, arg.keycode = kc },
    tap = fun kc => { inst = 'tap, arg.keycode = kc },
    wait = fun ms => { inst = 'wait, arg.duration_ms = ms },
    pause_for_release = { inst = 'pause_for_release, arg = {} },
  },
  
  MO = fun i => hold.reg.layer i,
  LT = fun b i tap => hold.reg.behavior b & hold.reg.layer i & tap,
  LM = fun b i mods tap => LT b i tap,
  DF = fun i => tap.layer.DF i,
  TG = fun i => tap.layer.TG i,
  TO = fun i => tap.layer.TO i,
  CTL_T = fun b tap => hold.reg.behavior b & hold.reg.mod.lctl & tap,
}
