/*---
desc: For loop with existing variable
---*/

function main(): void {
  let i = 0;
  let sum = 0;
  for (i = 1; i < 4; i++) {
    sum += i;
  }
  assert sum == 6;
}