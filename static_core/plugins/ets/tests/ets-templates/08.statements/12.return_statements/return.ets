/*---
desc: Return statement
tags: [negative, compile-only]
---*/

function test(): void {
  return;

  assert false;  // CTE: unreachable statement
}

function main(): void {
  test();
}
