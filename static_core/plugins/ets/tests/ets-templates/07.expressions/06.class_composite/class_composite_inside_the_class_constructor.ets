/*---
desc: Using a class composite inside the class constructor
---*/

class Parent {
  name: String;
}

class Person {
  parent: Parent;

  constructor (n: String) {
    this.parent = {name: n};
  }  
}

function main(): void {
  let person: Person = new Person("Bob");

  assert person.parent.name == "Bob";
}