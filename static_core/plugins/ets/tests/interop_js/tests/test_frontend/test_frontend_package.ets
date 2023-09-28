package Test;

import {
    Car, foo, A
} from "/plugins/ets/tests/interop_js/tests/test_frontend/test_frontend_package.js";


function test_newcall(): boolean {
    let color = "Cyan";
    let car = new Car(color);
    return color == car.color as String;
}

function test_dyncall(): int {
    let a = new A();
    // this call is needed to fill dynamic call cache with wrong signature
    let fake_res = foo(314);
    return a.getB().prop_var.foo(5) as int;
}

function test_dyncall_by_value(): int {
    let a = new A();
    // this call is needed to fill dynamic call cache with wrong signature
    let fake_res = a.getB().prop_var.foo(5) as int;
    return foo(1) as int;
}
