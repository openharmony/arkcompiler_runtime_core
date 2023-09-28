/*---
desc: Empty return value inside void expression
tags: [compile-only]
---*/

function example(): void {
  return
}

function main(): void {
  example();
}