/*---
desc: Second and third operands are of type Array
---*/


function main(): void {
  let y: int[] = [0, 1, 2];
  let z: int[] = [0, 2, 4];

  let x: int[] = true ? y : z;
  assert x == y;
}