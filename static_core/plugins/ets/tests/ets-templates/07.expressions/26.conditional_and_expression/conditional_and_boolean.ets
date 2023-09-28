/*---
desc: Conditional and operator
---*/

function check(): Boolean {
  assert(false);
  return true;
}

function main(): void {
  let x: Boolean = false;
  assert(!(x && check()));
}