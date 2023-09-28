/*---
desc: Array access expression
---*/

let arr: int[] = [1, 2, 3];

function main(): void {
  let x = arr[0];
  let y = arr[x];
  let z = arr[x+1];
  assert(x == 1 && y == 2 && z == 3);
}
