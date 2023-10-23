/*---
desc: 03.08.11 Union types - literal types
name: 03.types/08.reference_types/03.08.11.union_types/union02
tags: []
---*/

type T = "hello" | "goodbye"
function say(greet: T, name: string): string {
    return greet + " " + name
}

function tToString01(greet: T): string {
    // implicit cast
    return greet
}

function tToString02(greet: T): string {
    // explicit cast
    return greet as string
}

function tToString03(greet: T): string {
    let name: string = greet
    return name + "+"
}

function main(): void {
    let t1: T = "hello"
    let t2: T = "goodbye"
    t1 = t2
    assert t1 == "goodbye"
    assert say(t1, "blue sky") == "goodbye blue sky"
    assert tToString01(t1) == "goodbye"
    assert tToString02(t1) == "goodbye"
    assert tToString03(t1) == "goodbye+"
}
