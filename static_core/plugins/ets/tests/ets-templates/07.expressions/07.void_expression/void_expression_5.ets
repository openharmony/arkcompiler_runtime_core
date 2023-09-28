/*---
desc: Void expression as function argument
tags: [compile-only]
---*/

let res: int;

function processValue(value: int): void {
  res = value;
}

function example(): int {
  return 1;
}

function main(): void {
  res = 0;
  processValue(example());

  assert res == 1;
}