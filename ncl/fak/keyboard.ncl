let { MAX_USB_STRING_LENGTH, DEFAULT_DEBOUNCE_MS, .. } = import "constants.ncl" in
let { Uint8, Uint16, BoundedInt, Set, ElementOf, .. } = import "util_types.ncl" in

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

let McuFeature = fun gpios => {
  type | [| 'uart |],
  keys | Set String,
  pins | Set (ElementOf gpios),
} in

let Mcu = {
  gpios | Set GpioPin,
  features | { _ : McuFeature gpios } | default = {},
} in

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

let Matrix = fun mcu => {
  cols | Array (ElementOf mcu.gpios) | default = [],
  rows | Array (ElementOf mcu.gpios) | default = [],
} in

let PhysicalKey = fun mcu matrix => {
  type | [| 'direct, 'matrix, 'peripheral |],
  data | match {
    'direct => ElementOf mcu.gpios,
    'matrix => {
      col | MatrixCol matrix,
      row | MatrixRow matrix,
    },
    # Index validity check is performed in main.ncl
    'peripheral => Uint8,
  } type
} in

let KeyboardPeripheralSide = {
  mcu | Mcu,
  matrix | Matrix mcu | default = {},
  keys | Set (PhysicalKey mcu matrix),
  split.channel | ElementOf mcu.features,
} in

let KeyboardCentralSide = {
  mcu | Mcu,
  matrix | Matrix mcu | default = {},
  keys | Set (PhysicalKey mcu matrix),
  usb_dev | UsbDev,
  debounce_ms | Uint8 | default = DEFAULT_DEBOUNCE_MS,
  split | {
    channel | ElementOf mcu.features,
    peripheral | KeyboardPeripheralSide,
  } | optional,
} in

{
  DirectPinKey = fun pin => {
    type = 'direct,
    data = pin
  },
  MatrixKey = fun col_pin row_pin => {
    type = 'matrix,
    data.col = col_pin,
    data.row = row_pin,
  },
  PeripheralSideKey = fun periph_key_idx => {
    type = 'peripheral,
    data = periph_key_idx,
  },
  Keyboard = KeyboardCentralSide,
}
