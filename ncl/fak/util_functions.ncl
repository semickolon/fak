let rec _array = {
  unique = fun arr => std.array.fold_left (fun acc e =>
    if std.array.elem e acc then acc else acc @ [e]
  ) [] arr,

  enumerate = fun arr => std.array.fold_left (fun acc e => acc @ [{
    index = std.array.length acc,
    value = e
  }] ) [] arr,

  index_of = fun e arr => 
    let matches = std.array.filter (fun { index, value } => value == e) (enumerate arr) in
    (std.array.first matches).index,
  
  index_of_or_n1 = fun e arr =>
    let matches = std.array.filter (fun { index, value } => value == e) (enumerate arr) in
    if std.array.length matches > 0 then
      (std.array.first matches).index
    else
      -1,
  
  join = fun sep arr => std.array.fold_left (++) "" (std.array.intersperse sep arr),

  chunk | std.number.PosNat -> Array Dyn -> Array (Array Dyn) = fun size arr =>
    if std.array.length arr <= size then
      [ arr ]
    else
      let splits = std.array.split_at size arr in
      std.array.prepend splits.left ( chunk size splits.right ),
  
  at_or = fun i default_value arr =>
    if i < std.array.length arr then
      std.array.at i arr
    else
      default_value,
  
  sort_num | Array Number -> Array Number = fun arr =>
    arr |> std.array.sort (fun x y =>
      if x < y then
        'Lesser
      else if (x == y) then
        'Equal
      else
        'Greater),
  
  min | Array Number -> Number = fun arr =>
    arr |> sort_num |> std.array.first,
  
  max | Array Number -> Number = fun arr =>
    arr |> sort_num |> std.array.last,
  
  first_or = at_or 0,

  last_or = fun default_value arr =>
    if std.array.length arr > 0 then
      std.array.last arr
    else
      default_value,
  
  to_string | Array std.string.Stringable -> String = fun arr =>
    let contents = arr |> std.array.map std.to_string |> join ", " in
    "[%{contents}]",
} in

let Integer = std.number.Integer in
let rec _bit = {
  shift | Integer -> Integer -> Integer = fun n shift => (
    if shift >= 0 then
      n * (std.number.pow 2 shift)
    else
      std.number.truncate (n / (std.number.pow 2 (-shift)))
  ),
} in

let rec _record = {
  only_if | Bool -> Dyn -> Dyn =
    fun cond record => if cond then record else {},
  
  at_or | Dyn -> String -> Dyn -> Dyn =
    fun record field default_value =>
      if std.record.has_field field record then
        record."%{field}"
      else
        default_value,
} in

{
  array = _array,
  bit = _bit,
  record = _record,
}
