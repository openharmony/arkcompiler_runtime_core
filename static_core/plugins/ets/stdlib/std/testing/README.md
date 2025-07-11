# ArkTest framework
ArkTest is unit testing framework for ArkTS code.  
_NOTE(ipetrov): add more documentation_

### Example of usage
```ts
// Create a testsuite instance
let myTestsuite = new arktest.ArkTestsuite("myTestsuite");
// Add a test to the testsuite
myTessuite.addTest("TestWithEqualityAndNonEquality", () => {
    let one = 1;
    arktest.assertEQ(one, 1);
    arktest.assertNE(one, 2);
    arktest.assertLT(one, 3);
    arktest.assertLE(one, 1, "1 should be <= 1");
    arktest.assertLE(one, 4);
});
// Add one more test to the testsuite
myTessuite.addTest("TestWithExceptions", () => {
    arktest.expectError(() => { throw new Error() }, new Error());
    let expectedException = new Exception("Expected message");
    arktest.expectException(() => { throw new Exception("Expected message") }, expectedException);
    // Expect any exception
    arktest.expectException(() => { throw new Exception("Expected message") });
    arktest.expectNoThrow(() => {});
});
// Run all tests for the testsuite
myTestsuite.run();
```