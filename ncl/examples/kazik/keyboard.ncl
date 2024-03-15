let { ColToRowKey, RowToColKey, .. } = import "../../fak/keyboard.ncl" in
let { CH552T, .. } = import "../../fak/mcus.ncl" in

{
  mcu = CH552T,
  matrix = {
    cols = [17],
    rows = [10]
  },
  usb_dev = {
    vendor_id = 43962, # 0xABBA
    product_id = 1,
    product_ver = 256, # 0x0100
  },
  keys =
    let C = ColToRowKey in
    let R = RowToColKey in
    [
      C 0 0, R 0 0
    ]
}
