/*---
desc: >-
    Array access expression. This resultant variable is a component of the array but
    is never considered const even if a const variable is denoted in the array reference
    expression.
---*/

type NullableInt = Int|null
const arr: NullableInt[] = new NullableInt[3];

function main(): void {
  arr[0] = new Int(1)
  arr[1] = 2
  arr[2] = null

  assert arr[0] instanceof Int;
  assert arr[0] instanceof Object;
  assert arr[0] == 1

  assert arr[1] instanceof Int;
  assert arr[1] instanceof Object;
  assert arr[1] == 2

  assert arr[2] == null
}
