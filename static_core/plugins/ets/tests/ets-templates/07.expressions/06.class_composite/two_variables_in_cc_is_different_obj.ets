/*---
desc: Checking that two variables with the same expressions of the compound class are different objects
tags: [compile-only]
---*/

class Person {
  height: Int;
  age: Int;
}
  
function main(): void {
  let person: Person = {height:new Int(20), age: new Int(20)}
  assert person.height !== person.age
}
  