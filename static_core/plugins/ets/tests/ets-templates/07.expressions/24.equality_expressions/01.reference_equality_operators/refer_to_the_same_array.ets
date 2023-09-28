/*---
desc: The result of === at runtime is true if both operand values refer to the same array
---*/

function main(): void {
  let a: int[] = [1,2,3];
  let b: int[] = a;
  assert (a !== a) == false;
  assert (a !== b) == false;
  assert (a === a) == true;
  assert (a === b) == true;
}			