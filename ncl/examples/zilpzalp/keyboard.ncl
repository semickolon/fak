let { ColToRowKey, .. } = import "../../fak/keyboard.ncl" in
let { CH552T, .. } = import "../../fak/mcus.ncl" in

{
  mcu = CH552T,
  matrix = {
    cols = [32, 11, 12, 13],
    rows = [35, 34, 31, 30, 17, 16, 15]
  },
  usb_dev = {
    vendor_id = 43962, # 0xABBA
    product_id = 1,
    product_ver = 256, # 0x0100
  },
  keys =
    let M = ColToRowKey in
    [
             M 0 0, M 1 0, M 2 0, M 3 0,    M 3 6, M 2 6, M 1 6, M 0 6,
      M 0 2, M 0 1, M 1 1, M 2 1, M 3 1,    M 3 5, M 2 5, M 1 5, M 0 5, M 0 4,
             M 1 2, M 2 2, M 3 2,                  M 3 4, M 2 4, M 1 4,
                           M 1 3, M 3 3,    M 2 3, M 0 3,
    ]
}
