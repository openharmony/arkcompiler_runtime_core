/*---
desc: Conditional and operator
---*/

function check(): Boolean {
  assert(false);
  return true;
}

function main(): void {
  let x: Boolean = true;
  assert(!(x || check()));
}