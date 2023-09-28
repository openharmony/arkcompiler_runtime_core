/*---
desc: Checking access of a protected field inside a composite class outside the class
tags: [negative, compile-only]
---*/

class Person {
  protected name: String;
  age: int;
}
  
function main(): void {
  let person: Person = {name:"Bob", age: 18}
}
  