/*---
desc: The result of !== at runtime is false if both operand values are null
---*/

class A {}

function main(): void {
  let b: A|null = null;
  let c: A|null = null;
  assert (b !== c) == false;
  assert (b != c) == false;
}
