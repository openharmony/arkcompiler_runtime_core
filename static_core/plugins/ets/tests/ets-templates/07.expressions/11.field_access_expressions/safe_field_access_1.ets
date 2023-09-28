/*---
desc: Safe access to field
assert: A field access in which objectReference contains ‘?.’ is called safe field access because it handles null safely.
---*/

class A {
  public x: int;
}

function main(): void {
    let instance: A|null = null;
    instance?.x;
}
