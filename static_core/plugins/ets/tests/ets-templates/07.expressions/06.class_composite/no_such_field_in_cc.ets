/*---
desc: Compilation error when using an inaccessible field
tags: [negative, compile-only]
---*/

class Friend {
  name: String
  private age: int
}

let person: Friend = { surname: "Anderson" }
