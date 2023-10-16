class C0 {
   x0: boolean
}

function test_1() : C0 {
   return ([{x0: true}])[0]
}

function test_2() : C0 {
   return (([[{x0: true}]])[0])[0];
}

function test_3() {
    let x0: C0 = ([{x0: true}])[0];
    if (false) {
        return x0;
    }

    // Expecting type to be inferred from the other return expression
    return ([{x0: true}])[0];
}

function test_parameter(x : C0) {
}

function main() : void {
   let x0: C0 = ([{x0: true}])[0];
   assert (x0.x0 == true) : "Incorrect class composite property #1!";

   let x1: C0 = (([[{x0: true}]])[0])[0];
   assert (x1.x0 == true) : "Incorrect class composite property #2!";

   let x2: C0 = ((([[[{x0: true}]]])[0])[0])[0];
   assert (x2.x0 == true) : "Incorrect class composite property #3!";

   let f1: C0 = test_1();
   assert (f1.x0 == true) : "Incorrect class composite property #4!";

   let f2: C0 = test_2();
   assert (f2.x0 == true) : "Incorrect class composite property #5!";

   let f3: C0 = test_3();
   assert (f3.x0 == true) : "Incorrect class composite property #6!";

   test_parameter(([{x0: true}])[0]);
   test_parameter((([[{x0: true}]])[0])[0]);
}
