/*---
desc: >-
    Multi-line string literals that can include embedded expressions
    are called Template literals.
    A template literal with embedded expression is a template string.
tags: [compile-only]
---*/

function main() {
  let a = "A"
  let b = 1
  let c = true
  let t = `
      a = ${a + c}

      b = ${++b}

      c = ${!!c}

      `
}
