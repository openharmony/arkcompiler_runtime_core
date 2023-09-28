/*---
desc: >-
    Multi-line string literals that can include embedded expressions
    are called Template literals.
    A template literal with embedded expression is a template string.
tags: [compile-time, negative]
---*/

function main() {
  // expression refers to its template literal
  let t1 = `template ${t1}
      literal
      example
      `
}
