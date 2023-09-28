/*---
desc: Initializing a class with inherited fields
---*/

class Parent {
  name: String
  age: int
}
  
class Child extends Parent {
  weight: int
}
  
function main(): void {
  let child: Child = { name: "Ben", age: 3, weight: 20 };

  assert child.weight == 20;
} 
  