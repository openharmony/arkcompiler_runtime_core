/*---
desc: Switch statement, duplicate value
tags: [negative, compile-only]
---*/

function f(i: int): void {
  switch (i) {
    case 0: assert i == 0; break;
    case 0: assert i == 1; break;
    case 0: assert i == 2; break;
    default: assert false;
  }
}

function main(): void {
  f(0);
  f(1);
  f(2);
}
