/*---
desc: Compilation error with incorrect value type
tags: [negative, compile-only]
---*/

class Person {
  name: String;
  age: int;
}

function main(): void{ 
  let person: Person = { name: "Bob", age: "30" };
}