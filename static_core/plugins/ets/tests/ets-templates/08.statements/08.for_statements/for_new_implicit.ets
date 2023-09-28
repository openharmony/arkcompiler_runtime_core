/*---
desc: For loop with new variable (implicit type)
---*/

function main(): void {
  let sum = 0;
  for (let i = 1; i < 4; i++) {
    sum += i;
  }
  assert sum == 6;
}