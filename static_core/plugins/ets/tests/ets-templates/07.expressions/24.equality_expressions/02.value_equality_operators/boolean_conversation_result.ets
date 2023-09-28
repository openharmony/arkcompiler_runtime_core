/*---
desc: A reference equality expression is always of the boolean type.
---*/

function main(): void {
  let a: int = 5;
  let b: int = 5;
  let res = (a == b);
  assert res == true;
}			