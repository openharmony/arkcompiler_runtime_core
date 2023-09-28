/*---
desc: Template literals are of type string, which is a predefined reference type
---*/

function main() {
  // newlines only
  let t1 = `


`
  assert t1 instanceof string && t1 == "\n\n\n"

  // empty string
  let t2 = ``
  assert t2 instanceof string && t2 == ""

  // escape back-tick
  let t3 = `\``
  assert t3 instanceof string && t3 == "`"

  // escape dollar sign
  let t4 = `\${2*2}`
  assert t4 instanceof string && t4 == "${2*2}"

  // escape opening curly brace
  let t5 = `$\{2*2}`
  assert t5 instanceof string && t5 == "${2*2}"

  // escape back-slash
  let t6 = `test \\node`
  assert t6 instanceof string && t6 == "test \\node"
}
