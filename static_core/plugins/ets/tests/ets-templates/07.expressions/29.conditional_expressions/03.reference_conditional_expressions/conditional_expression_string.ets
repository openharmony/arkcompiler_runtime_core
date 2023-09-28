/*---
desc: Second and third operands are of type String
---*/


function main(): void {
  let x: String = true ? "Hello" : "World";
  assert x == "Hello";
}