/*---
desc: Readonly simple form fo interface property
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

  assert instance.text == "text";
}