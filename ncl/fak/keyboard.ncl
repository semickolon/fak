let { MAX_USB_STRING_LENGTH, DEFAULT_DEBOUNCE_MS, .. } = import "constants.ncl" in
let { Uint8, Uint16, BoundedInt, Set, .. } = import "util_types.ncl" in

let GpioPin = std.contract.from_predicate (fun value =>
  std.is_number value
  && std.number.is_integer value
  && value >= 10
  && value <= 47
  && value % 10 < 8) in

let UsbString = std.contract.from_predicate (fun value =>
  std.is_string value
  && std.string.length value > 0
  && std.string.length value <= MAX_USB_STRING_LENGTH
) in

let Clock = {
  fsys | [| 'M8, 'M16, 'M24 |],
  external | Bool | default = false
} in

let Mcu = {
  gpios | Set GpioPin,
  clock | Clock
} in

let McuGpioPin = fun mcu =>
  std.contract.from_predicate (fun x => std.array.any ((==) x) mcu.gpios)
in

let MatrixCol = fun matrix => BoundedInt 0 (std.array.length matrix.cols) in
let MatrixRow = fun matrix => BoundedInt 0 (std.array.length matrix.rows) in

let UsbDev = {
  vendor_id | Uint16,
  product_id | Uint16,
  product_ver | Uint16,
  manufacturer | UsbString | optional,
  product | UsbString | optional,
  serial_number | UsbString | optional
} in

let Matrix = (fun mcu => {
  cols | Array (McuGpioPin mcu) | default = [],
  rows | Array (McuGpioPin mcu) | default = [],
}) in

let PhysicalKey = {
  type | [| 'direct, 'matrix |],
  data
} in

{
  DirectPinKey = fun mcu => (
    fun pin => {
      type = 'direct,
      data = pin | McuGpioPin mcu
    }
  ),
  MatrixKey = fun matrix => (
    fun col_pin row_pin => {
      type = 'matrix,
      data = {
        col = col_pin | MatrixCol matrix,
        row = row_pin | MatrixRow matrix
      }
    }
  ),
  Keyboard = {
    mcu | Mcu,
    usb_dev | UsbDev,
    matrix | Matrix mcu | default = {},
    keys | Array PhysicalKey,
    debounce_ms | Uint8 | default = DEFAULT_DEBOUNCE_MS
  },
}
