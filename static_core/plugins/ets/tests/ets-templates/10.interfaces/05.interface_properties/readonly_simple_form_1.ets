/*---
desc: Readonly simple form fo interface property
tags: [negative, compile-only]
---*/

interface I {
  readonly text: string;
}

class B implements I {
  public constructor() {
    text = "text";
  }

}

function main(): void {
  let instance: I = new B();

  instance.text = "text";
}