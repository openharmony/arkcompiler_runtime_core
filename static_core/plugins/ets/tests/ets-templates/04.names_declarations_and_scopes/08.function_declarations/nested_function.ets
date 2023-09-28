/*---
desc: Functions must be declared on the top-level.
tags: [compile-only, negative]
---*/

function main(): void {
  function nested(): void {}
}
