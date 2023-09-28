/*---
desc: For loop with new variable
---*/

function main(): void {
  let sum = 0;
  for (let i: int = 1; i < 4; i++) {
    sum += i;
  }
  assert sum == 6;
}