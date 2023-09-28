/*---
desc: A name must be unique in its block space
tags: [negative, compile-only]
---*/

function main(): void {
  {
    let x = 0;
    let x = 0;
  }
}
