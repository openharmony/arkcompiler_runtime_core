/*---
desc: All string literals, such as ”abc”, are implemented as instances of String class.
---*/

function main(): void {
  let x = "abc";

  assert x instanceof String;
}
