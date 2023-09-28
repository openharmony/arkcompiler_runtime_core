/*---
desc: >-
    Multi-line string literals that can include embedded expressions
    are called Template literals.
    A template literal with embedded expression is a template string.
tags: [compile-time, negative]
---*/

function main() {
  let a = "A"
  // syntax error
  let t1 = `template ${a example`
}
