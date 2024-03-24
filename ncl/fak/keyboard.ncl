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
  dma_must_even_address | Bool | default = false,
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

let SplitChannel = fun mcu label value =>
  if std.record.has_field "type" value && value.type == 'uart then
    std.contract.apply (ElementOf mcu.features) label value
  else if std.record.has_field "soft_serial_pin" value then
    std.contract.apply (ElementOf mcu.gpios) label value.soft_serial_pin
  else
    std.contract.blame_with_message "Invalid split channel. Only UART or soft serial pin are supported."
in

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

let EncoderDef = fun mcu => {
  type | [| 'physical, 'peripheral |],
  data | match {
    'physical => {
      pin_a | ElementOf mcu.gpios,
      pin_b | ElementOf mcu.gpios,
      resolution | BoundedInt 1 128,
    },
    'peripheral => Uint8,
  } type
} in

let KeyboardPeripheralSide = {
  mcu | Mcu,
  matrix | Matrix mcu | default = {},
  keys | Set (PhysicalKey mcu matrix),
  encoders | Set (EncoderDef mcu) | default = [],
  split.channel | SplitChannel mcu,
} in

let KeyboardCentralSide = {
  mcu | Mcu,
  matrix | Matrix mcu | default = {},
  keys | Set (PhysicalKey mcu matrix),
  encoders | Set (EncoderDef mcu) | default = [],
  usb_dev | UsbDev,
  debounce_ms | Uint8 | default = DEFAULT_DEBOUNCE_MS,
  split | {
    channel | SplitChannel mcu,
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
  PhysicalEncoder = fun a b res => {
    type = 'physical,
    data = {
      pin_a = a,
      pin_b = b,
      resolution = res,
    }
  },
  PeripheralSideEncoder = fun periph_encoder_idx => {
    type = 'peripheral,
    data = periph_encoder_idx,
  },
  SoftSerialPin = fun pin => {
    soft_serial_pin = pin,
  },
  Keyboard = KeyboardCentralSide,
}
