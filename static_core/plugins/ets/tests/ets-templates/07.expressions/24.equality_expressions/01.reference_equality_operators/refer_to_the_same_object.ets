/*---
desc: The result of === at runtime is true if both operand values refer to the same object
---*/

class A {}

function main(): void {
  let a: A = new A();
  let b: A = a;
  assert (a !== a) == false;
  assert (a !== b) == false;
  assert (a === a) == true;
  assert (a === b) == true;
}			