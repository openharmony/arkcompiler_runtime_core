import {aaa} from './base.js'

function A() {
    a = 2;
    b = 2;
    c = 2;
}

function B() {
    let a = 0;
    let b = 0;
}

a = 0;
b = 0;

function C() {
    let m = a + b;
}

function D() {
    let d = a + b;
    let e = 3;
    let m = d + e;
    return m;
}

class E {
    constructor(name) {
        this.name = name
    }
}
