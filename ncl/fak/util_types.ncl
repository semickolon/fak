let { array, .. } = import "util_functions.ncl" in

{
  Uint = (fun bits =>
    std.contract.from_predicate (fun value =>
      std.is_number value
      && std.number.is_integer value
      && value >= 0
      && value < (std.number.pow 2 bits))
  ),
  Uint8 = Uint 8,
  Uint16 = Uint 16,
  Uint32 = Uint 32,

  BoundedInt = (fun lower_inclusive upper_exclusive =>
    std.contract.from_predicate (fun n => 
      std.is_number n 
      && std.number.is_integer n 
      && n >= lower_inclusive
      && n < upper_exclusive)
  ),

  Set = fun t =>
    let UniqueItems = std.contract.from_predicate (fun arr =>
      std.is_array arr
      && (array.unique arr) == arr
    ) in
    std.contract.Sequence [ Array t, UniqueItems ],

  ElementOf = fun list => std.contract.from_predicate
    (match {
      '"Array" => fun e => std.array.elem e list,
      'Record  => fun e => std.array.elem e (std.record.values list),
    } (std.typeof list)),
}
