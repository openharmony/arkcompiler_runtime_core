/*---
desc: >-
    Array access expression. A numeric types conversion (§6.2.1) is performed on index
    expression to make sure the resultant type is int. A compile-time error occurs otherwise.
---*/

let arr: int[] = [1, 2, 3];

function main(): void {
  let x: Byte = new Byte(0 as byte)
  let y: Short = new Short(1 as short)
  let z: Int = new Int(2 as int)

  assert arr[x] == 1 && arr[y] == 2 && arr[z] == 3
}
