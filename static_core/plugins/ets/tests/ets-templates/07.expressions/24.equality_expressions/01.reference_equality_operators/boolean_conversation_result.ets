/*---
desc: A reference equality expression is always of the boolean type.
---*/

function main(): void {
  let a: int[] = [1,2,3];
  let b: int[] = a;
  let res = (a === a);
  assert res == true;
}			