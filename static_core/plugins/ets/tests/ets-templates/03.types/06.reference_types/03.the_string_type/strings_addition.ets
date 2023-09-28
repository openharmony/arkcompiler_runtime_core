/*---
desc: adding two strings
---*/

function main(): void {
  let a: string = "abc";
  let b: string = "def";
  let c: string = a + b;

  assert c == "abcdef";
}
