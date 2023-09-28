/*---
desc: Nested if statement
---*/

function main(): void {
  if (false) {
    if (true) {
      assert false;
    }
  } else assert true;
}