/*---
desc: Conditional expression order
---*/

function main(): void {
  let cond = false;
  let a = cond?1:cond?2:cond?3:0;
  let b = cond?1:(cond?2:(cond?3:0));
  assert a == b;
}