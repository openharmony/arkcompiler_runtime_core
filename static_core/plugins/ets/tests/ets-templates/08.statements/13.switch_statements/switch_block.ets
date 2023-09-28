/*---
desc: Switch statement with block
---*/

function f(i: int): void {
  switch (i) {
    case 0: { assert i == 0; break; }
    case 1: { assert i == 1; break; }
    case 2: { assert i == 2; break; }
    default: { assert false; break; }
  }
}

function main(): void {
  f(0);
  f(1);
  f(2);
}