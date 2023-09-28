/*---
desc: The numerical comparison operators with type boolean.
---*/

function main(): void {
  let a: boolean = true;
  let b: boolean = false;

  assert !b == true;
  assert !a == false;
}