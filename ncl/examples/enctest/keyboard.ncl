let { ColToRowKey, DirectPinKey, PhysicalEncoder, .. } = import "../../fak/keyboard.ncl" in
let { CH552T, .. } = import "../../fak/mcus.ncl" in

{
  mcu = CH552T,
  usb_dev = {
    vendor_id = 43963, # 0xABBB
    product_id = 1,
    product_ver = 257, # 0x0101
  },
  encoders = let E = PhysicalEncoder in [
    E 12 10 8,
    E 32 11 4,
  ],
  keys =
    let M = ColToRowKey in
    let D = DirectPinKey in
    [
      D 13, D 35, D 34, D 31,
      D 15, D 16, D 17, D 30,
    ]
}
