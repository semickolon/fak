let { ColToRowKey, .. } = import "../../fak/keyboard.ncl" in
let { CH559L, .. } = import "../../fak/mcus.ncl" in

{
  mcu = CH559L,
  matrix = {
    cols = [11, 10, 17, 16, 15, 14],
    rows = [24, 25, 22, 21],
  },
  usb_dev = {
    vendor_id = 43963, # 0xABBB
    product_id = 1,
    product_ver = 257, # 0x0101
  },
  keys =
    let M = ColToRowKey in
    [ # Polydactyl
             M 1 0, M 2 0, M 3 0, M 4 0, M 5 0,
      M 0 1, M 1 1, M 2 1, M 3 1, M 4 1, M 5 1,
      M 0 2, M 1 2, M 2 2, M 3 2, M 4 2, M 5 2, M 5 3,
                    M 1 3, M 2 3, M 3 3, M 4 3,
    ]
}
