/*---
desc: Initializing a class using a class composite
tags: [compile-only]
---*/

class Person {
  name: String;
  age: int;
}
  
function main(): void {
  let p: Person = { name: "Alice", age: 25 };
}
