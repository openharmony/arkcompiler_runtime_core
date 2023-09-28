/*---
desc: Conditional expression order
---*/

function f1(): int {
  assert false;
  return 1;
}
function f2(): int {
  assert false;
  return 2;
}
function f3(): int {
  assert false;
  return 3;
}

function main(): void {
  let cond = false;
  let a = cond ? f1() : cond ? f2() : cond ? f3() : 0;
  assert a == 0;
}