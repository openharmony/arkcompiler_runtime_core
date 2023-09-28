/*---
desc: The comma at the end of the composite class is ignored
---*/

class Person {
  name: String
  age: int
}
  
function main(): void {
  let p: Person = { name: "Alice", age: 25, }
}
