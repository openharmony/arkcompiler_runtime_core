/*---
desc: >-
    The value of a type T expression is always suitable for
    assignment to a type T variable; it means that unless heap
    pollution occurs
---*/

class Box<T> {
    private v: T;

    public set(v: T): void {
      this.v = v;
    }

    public get(): T {
      return this.v;
    }
}

function main(): int {
    let intBox: Box<Int> = new Box<Int>();
    intBox.set(42);

    let obj: Object = intBox;
    let stringBox: Box<String> = obj as Box<String>;

    try {
        let s: String = stringBox.get();
    } catch (e: ClassCastException) {
        return 0;
    }
    return 1;
}
