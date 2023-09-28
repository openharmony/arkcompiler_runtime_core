/*---
desc: Using a class composite inside a class method
---*/

class Person {
  weight: int;
  height: int;

  static getPerson(): Person {
    let person: Person = {weight: 10,  height: 5};
    return person;
  }
}
  
function main(): void {
  let person = Person.getPerson();
  let index = person.weight / person.height;
  assert index == 2;
}
  