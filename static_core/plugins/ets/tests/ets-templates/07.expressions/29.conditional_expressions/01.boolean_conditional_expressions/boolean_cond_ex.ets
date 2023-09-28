/*---
desc: Conditional expression boolean
tags: [compile-only, negative]
---*/

function main(): void {
  let bool1: boolean = true
  let notBool: String = "Hello"
  let bool3: boolean = bool1 ? notBool : bool1
  assert bool3 == false
}