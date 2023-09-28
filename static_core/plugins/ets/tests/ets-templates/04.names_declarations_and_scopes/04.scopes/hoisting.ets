/*---
desc: Hoisting not available
tags: [negative, compile-only]
---*/

function main(): void {
  let x = y;
  let y = 1;
}
