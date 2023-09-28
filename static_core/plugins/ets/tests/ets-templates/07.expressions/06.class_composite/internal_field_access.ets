/*---
desc: Checking a internal field in class composite
---*/

class Person {
  internal name: String;
  age: int;
}

function main(): void {
  let obj: Person = { name: "Ben", age: 3 };

  assert obj.name == "Ben";
}