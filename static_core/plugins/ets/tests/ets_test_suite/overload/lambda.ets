let void_called: boolean = false;
let object_called = false;
let int_called = false;

function reset(): void {
    void_called = false;
    object_called = false;
    int_called = false;
}

function foo(callback: () => void): void {
    void_called = true;
}

function foo(callback: (x: Object) => void): void {
    object_called = true;
}

function foo(callback: (x: int) => void): void {
    int_called = true;
}

function main(): void {
    reset();
    foo((): void => {
    });
    assert(void_called == true);
    assert(object_called == false);
    assert(int_called == false);

    reset();
    foo((x: Object): void => {
    });
    assert(void_called == false);
    assert(object_called == true);
    assert(int_called == false);

    reset();
    foo((x: int): void => {
    });
    assert(void_called == false);
    assert(object_called == false);
    assert(int_called == true);
}
