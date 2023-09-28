/*---
desc: Function call expression
---*/

function callee(a: int, b: boolean): int {
  if (b) {
    return a;
  } else {
    return -a;
  }
}

function main(): void {
  assert(callee(1,  true) ==  1);
  assert(callee(1, false) == -1);
}
