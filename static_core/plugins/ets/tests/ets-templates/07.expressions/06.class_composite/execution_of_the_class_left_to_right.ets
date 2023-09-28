/*---
desc: Name-value pairs of the class composite are then executed from left to right in the textual order they occur in the source code
---*/

let res: int;

class Person {
  name: String;
  age: int;
}
  
function getName(): String {
  res = 1;
  return "Bob";
}

function getAge(): int {
  assert res == 1;
  res = 2;
  return 18;
}

function main(): void {
  res = 0;
  let person: Person = {name: getName(), age: getAge()};

  assert res == 2;
}