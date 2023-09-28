/*---
desc: Break statement
tags: [negative, compile-only]
---*/

function main(): void {
  let i = 2;

  while (i > 0) {
    break;
    i--;     // compilation fails due to this unreachable statement.

  }

  assert i == 2;
}