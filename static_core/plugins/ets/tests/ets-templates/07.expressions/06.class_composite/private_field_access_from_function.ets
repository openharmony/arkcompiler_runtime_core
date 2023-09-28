/*---
desc: Checking access of a private field inside a composite class outside the class
tags: [negative, compile-only]
---*/

class Person {
  private name: String;
  age: int;
}
  
function main(): void {
  let person: Person = {name:"Bob", age: 18};
}
  