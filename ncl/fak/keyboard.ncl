let { MAX_USB_STRING_LENGTH, DEFAULT_DEBOUNCE_MS, .. } = import "constants.ncl" in
let { Uint8, Uint16, BoundedInt, Set, ElementOf, .. } = import "util_types.ncl" in

let GpioPin = std.contract.from_predicate (fun value =>
  std.is_number value
  && std.number.is_integer value
  && value >= 00
  && value <= 47
  && value % 10 < 8) in

let UsbString = std.contract.from_predicate (fun value =>
  std.is_string value
  && std.string.length value <= MAX_USB_STRING_LENGTH
) in

let McuFeature = fun gpios => {
  type | [| 'uart |],
  keys | Set String,
  pins | Set (ElementOf gpios),
} in

let Mcu = {
  family | [| 'CH552, 'CH559 |],
  gpios | Set GpioPin,
  features | { _ : McuFeature gpios } | default = {},
} in

let MatrixCol = fun matrix => BoundedInt 0 (std.array.length matrix.cols) in
let MatrixRow = fun matrix => BoundedInt 0 (std.array.length matrix.rows) in

let UsbDev = {
  vendor_id | Uint16,
  product_id | Uint16,
  product_ver | Uint16,
  manufacturer | UsbString | default = "",
  product | UsbString | default = "",
  serial_number | UsbString | default = "",
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
      direction | [| 'col_to_row, 'row_to_col |],
    },
    # Index validity check is performed in main.ncl
    'peripheral => Uint8,
  } type
} in

let KeyboardPeripheralSide = {
  mcu | Mcu,
  matrix | Matrix mcu | default = {},
  keys | Set (PhysicalKey mcu matrix),
  # TODO: Check that this is a UART feature
  split.channel | ElementOf mcu.features,
} in

let KeyboardCentralSide = {
  mcu | Mcu,
  matrix | Matrix mcu | default = {},
  keys | Set (PhysicalKey mcu matrix),
  usb_dev | UsbDev,
  debounce_ms | Uint8 | default = DEFAULT_DEBOUNCE_MS,
  split | {
    # TODO: Check that this is a UART feature
    channel | ElementOf mcu.features,
    peripheral | KeyboardPeripheralSide,
  } | optional,
} in

let MatrixKey = fun col_pin row_pin direction => {
  type = 'matrix,
  data.col = col_pin,
  data.row = row_pin,
  data.direction = direction,
} in

{
  DirectPinKey = fun pin => {
    type = 'direct,
    data = pin
  },
  ColToRowKey = fun c r => MatrixKey c r 'col_to_row,
  RowToColKey = fun c r => MatrixKey c r 'row_to_col,
  PeripheralSideKey = fun periph_key_idx => {
    type = 'peripheral,
    data = periph_key_idx,
  },
  Keyboard = KeyboardCentralSide,
}
