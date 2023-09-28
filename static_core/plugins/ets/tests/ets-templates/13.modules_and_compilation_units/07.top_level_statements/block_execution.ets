/*---
desc: A module may contain any number of such blocks that merge logically into a single block in the order
---*/

function pass(): void {
  assert true;
}

{
  pass();
}

function main(): void {
  assert false;
}