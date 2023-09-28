/*---
desc: A String object has an immutable value its value cannot be changed after it is created. Because String object is immutable it can be shared.
---*/

function main(): void {
  let a: string = "abc";
  let b: string = a;

  assert a == b;

  a = a + "d";

  assert a == "abcd";
  assert a != b;
}
