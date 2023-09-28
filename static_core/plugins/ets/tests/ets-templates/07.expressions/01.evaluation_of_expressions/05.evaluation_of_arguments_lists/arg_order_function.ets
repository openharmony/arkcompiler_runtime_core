/*---
desc: Argument list evaluation order (functions)
---*/

let a: boolean = false;
let b: boolean = false;
let c: boolean = false;

function ta(): int {
  assert !a;
  assert !b;
  assert !c;
  a = true;
  return 0;
}

function tb(): int {
  assert a;
  assert !b;
  assert !c;
  b = true;
  return 1;
}

function tc(): int {
  assert a;
  assert b;
  assert !c;
  c = true;
  return 2;
}

function td(): int {
  assert a;
  assert b;
  assert c;
  return 3;
}


function test(a: int, b: int, c: int, d: int): void {
  assert a == 0 && b == 1 && c == 2 && d == 3;
}

function main(): void {
  a = b = c = false;  // reset due to jit mode warmup

  test(ta(), tb(), tc(), td());
}
