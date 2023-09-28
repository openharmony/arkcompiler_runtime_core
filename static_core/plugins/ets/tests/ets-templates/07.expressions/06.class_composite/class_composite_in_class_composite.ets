/*---
desc: Checking access of a protected field inside a composite class outside the class
---*/

class Person {
  name: String;
  child: Child;

}

class Child {
  age: int;
}


function main(): void{ 
  let person: Person = {name: "Bob", child: {age: 15}};

  assert person.child.age == 15;
}