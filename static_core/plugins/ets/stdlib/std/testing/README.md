# ArkTest framework
ArkTest is unit testing framework for ArkTS code.  
_NOTE(ipetrov): add more documentation_

### Example of usage
```ts
// Create a testsuite instance
let myTestsuite = new ArkTestsuite("myTestsuite");
// Add a test to the testsuite
myTessuite.addTest("TestWithEqualityAndNonEquality", () => {
    let one = 1;
    assertEQ(one, 1);
    assertNE(one, 2);
    assertLT(one, 3);
    assertLE(one, 1, "1 should be <= 1");
    assertLE(one, 4);
});
// Add one more test to the testsuite
myTessuite.addTest("TestWithExceptions", () => {
    expectError(() => { throw new Error() }, new Error());
    let expectedException = new Exception("Expected message");
    expectException(() => { throw new Exception("Expected message") }, expectedException);
    // Expect any exception
    expectException(() => { throw new Exception("Expected message") });
    expectNoThrow(() => {});
});
// Run all tests for the testsuite
myTestsuite.run();
```