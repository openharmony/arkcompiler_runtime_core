/*---
desc: >-
    Multi-line string literals that can include embedded expressions
    are called Template literals.
    A template literal with embedded expression is a template string.
---*/

function main() {
  let t1 = `test1`
  let t2 = `test2 ${t1}`
  let t3 = `test3 ${t2}`

  assert t3 == "test3 test2 test1"
}
