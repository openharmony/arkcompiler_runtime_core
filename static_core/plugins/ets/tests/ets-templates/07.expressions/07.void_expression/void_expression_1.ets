/*---
desc: Calling void expression inside main function
tags: [compile-only]
---*/

function example(): void {}

function main(): void {
  example();
}