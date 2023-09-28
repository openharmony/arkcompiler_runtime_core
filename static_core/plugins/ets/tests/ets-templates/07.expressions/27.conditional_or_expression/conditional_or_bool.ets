/*---
desc: Conditional or operator
---*/

function check(): boolean {
  assert false;
  return true;
}

function main(): void {
  let x: boolean = true;
  assert x || check();
}