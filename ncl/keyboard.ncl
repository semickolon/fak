let { DirectPinKey, MatrixKey, .. } = import "fak/keyboard.ncl" in

let CH552T = {
  gpios = [
    10, 11, 12, 13, 14, 15, 16, 17,
    30, 31, 32, 33, 34, 35,
  ],
  clock.fsys = 'M24,
} in

{
  mcu = CH552T,
  usb_dev = {
    vendor_id = 65535,
    product_id = 0,
    product_ver = 42069,
  },
  matrix = {
    cols = [14, 15, 16, 17, 10],
    rows = [35, 34, 33, 32]
  },
  keys =
    let D = DirectPinKey mcu in
    let M = MatrixKey matrix in
    [
      M 0 0, M 1 0, M 2 0, M 3 0, M 4 0,
      M 0 1, M 1 1, M 2 1, M 3 1, M 4 1,
      M 0 2, M 1 2, M 2 2, M 3 2, M 4 2,
                           M 3 3, M 4 3,
    ]
}
