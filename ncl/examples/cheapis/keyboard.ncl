let { DirectPinKey, ColToRowKey, PeripheralSideKey, .. } = import "../../fak/keyboard.ncl" in
let { CH552T, .. } = import "../../fak/mcus.ncl" in

let side_central = {
  mcu = CH552T,
  matrix = {
    cols = [14, 15, 16, 17, 10],
    rows = [35, 34, 33, 32]
  },
  split.channel = CH552T.features.uart_12_13,
} in

let side_periph = {
  mcu = CH552T,
  matrix = {
    cols = [10, 17, 16, 15, 14],
    rows = [35, 34, 33, 32]
  },
  split.channel = CH552T.features.uart_30_31,
  keys = 
    let M = ColToRowKey in
    [
      M 0 0, M 1 0, M 2 0, M 3 0, M 4 0,
      M 0 1, M 1 1, M 2 1, M 3 1, M 4 1,
      M 0 2, M 1 2, M 2 2, M 3 2, M 4 2,
      M 0 3, M 1 3,
    ]
} in

side_central & {
  usb_dev = {
    vendor_id = 65535,
    product_id = 0,
    product_ver = 42069,
    # FIXME: There is a known issue with strings making USB connection slow
    # manufacturer = "dotleon",
    # product = "cheapis",
  },
  split.peripheral = side_periph,
  keys =
    let D = DirectPinKey in
    let M = ColToRowKey in
    let S = PeripheralSideKey in
    [
      M 0 0, M 1 0, M 2 0, M 3 0, M 4 0,      S 0,  S 1,  S 2,  S 3,  S 4,
      M 0 1, M 1 1, M 2 1, M 3 1, M 4 1,      S 5,  S 6,  S 7,  S 8,  S 9,
      M 0 2, M 1 2, M 2 2, M 3 2, M 4 2,      S 10, S 11, S 12, S 13, S 14,
                           M 3 3, M 4 3,      S 15, S 16,
    ]
}
