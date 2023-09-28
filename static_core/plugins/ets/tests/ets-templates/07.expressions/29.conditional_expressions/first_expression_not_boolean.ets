/*---
desc: A compile-time error occurs unless the first expression is of the type boolean or Boolean.
tags: [compile-only, negative]
---*/

function cond(): void {}

function main(): void {
  let a = cond() ? 0 : 1;
}