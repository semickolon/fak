let { ColToRowKey, PeripheralSideKey, PhysicalEncoder, PeripheralSideEncoder, SoftSerialPin, .. } = import "../../fak/keyboard.ncl" in
let { CH559L, .. } = import "../../fak/mcus.ncl" in

let kbd_matrix = {
  cols = [11, 10, 17, 16, 15, 14],
  rows = [24, 25, 22, 21],
} in

let side_central = {
  mcu = CH559L,
  matrix = kbd_matrix,
  split.channel = SoftSerialPin 30,
} in

let side_periph = {
  mcu = CH559L,
  matrix = kbd_matrix,
  split.channel = SoftSerialPin 30,
  encoders =
    let E = PhysicalEncoder in
    [
      E 12 13 4,
    ],
  keys = 
    let M = ColToRowKey in
    [
             M 1 0, M 2 0, M 3 0, M 4 0, M 5 0,
      M 0 1, M 1 1, M 2 1, M 3 1, M 4 1, M 5 1,
      M 0 2, M 1 2, M 2 2, M 3 2, M 4 2, M 5 2, M 5 3,
                    M 1 3, M 2 3, M 3 3, M 4 3,
    ]
} in

side_central & {
  usb_dev = {
    vendor_id = 14908, # 0x3A3C
    product_id = 1,
    product_ver = 130,
  },
  split.peripheral = side_periph,
  encoders =
    let E = PhysicalEncoder in
    let S = PeripheralSideEncoder in
    [
      E 13 12 4,
      S 0,
    ],
  keys =
    let M = ColToRowKey in
    let S = PeripheralSideKey in
    [ # Polydactyl
             M 1 0, M 2 0, M 3 0, M 4 0, M 5 0,                S 4,  S 3,  S 2,  S 1,  S 0,
      M 0 1, M 1 1, M 2 1, M 3 1, M 4 1, M 5 1,                S 10, S 9,  S 8,  S 7,  S 6,  S 5,
      M 0 2, M 1 2, M 2 2, M 3 2, M 4 2, M 5 2, M 5 3,   S 17, S 16, S 15, S 14, S 13, S 12, S 11,
                    M 1 3, M 2 3, M 3 3, M 4 3,                S 21, S 20, S 19, S 18,
                  
    ],
}
