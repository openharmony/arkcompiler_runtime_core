/*---
desc: Simple form fo interface property
---*/

interface I {
    text: string;
}

class B implements I {
}

function main(): void {
    let instance: I = new B();

    instance.text = "text";
    assert instance.text == "text";
}