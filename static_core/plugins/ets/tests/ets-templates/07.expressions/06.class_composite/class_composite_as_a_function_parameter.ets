/*---
desc: Using a class composite as a function parameter
---*/

class Person {
  age: int;
}

let checkVar: int;

function foo(person: Person): void {
  checkVar = person.age;
}
  
function main(): void {
  checkVar = 0;
  assert checkVar == 0;
  for (let i = 0; i < 20; i++) {
    foo({ age: i });
    assert checkVar == i;
  }
}
  