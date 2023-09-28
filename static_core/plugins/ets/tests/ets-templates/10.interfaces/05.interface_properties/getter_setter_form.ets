/*---
desc: Implementing getter from interface
---*/

interface I {
    get text()
    set text(s: string)
}

class B implements I {
    private text : string = "";

    get text(): string {
        return this.text;
    }

    set text(s: string): void {
        this.text = s;
    }
}

function main(): void {
    let instance: I = new B();

    instance.text = "text";
    assert instance.text == "text";
}