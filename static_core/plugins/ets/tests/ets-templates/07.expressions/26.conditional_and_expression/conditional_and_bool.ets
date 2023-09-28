/*---
desc: Conditional and operator
---*/

function check(): boolean {
  assert(false);
  return true;
}

function main(): void {
  let x: boolean = false;
  assert(!(x && check()));
}