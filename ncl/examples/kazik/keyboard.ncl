let { ColToRowKey, RowToColKey, .. } = import "../../fak/keyboard.ncl" in
let { CH552T, .. } = import "../../fak/mcus.ncl" in

{
  mcu = CH552T,
  matrix = {
    cols = [15, 16, 31, 34, 35],
    rows = [32, 11, 12, 13],
  },
  usb_dev = {
    vendor_id = 1,
    product_id = 2,
    product_ver = 3,
  },
  keys =
    let C = ColToRowKey in
    let R = RowToColKey in
    [
      R 0 0, C 0 0, R 1 0, C 1 0, R 2 0, C 2 0, R 3 0, C 3 0, R 4 0, C 4 0,
      R 0 1, C 0 1, R 1 1, C 1 1, R 2 1, C 2 1, R 3 1, C 3 1, R 4 1, C 4 1,
      R 0 2, C 0 2, R 1 2, C 1 2, R 2 2, C 2 2, R 3 2, C 3 2, R 4 2, C 4 2,
      R 0 3, C 0 3,        C 1 3, R 2 3, C 2 3, R 3 3,
    ]
}
