function main():int {
    let failures = 0;

    failures += test(checkBadRadix(0.0 as Double, -1), "0 with Negative Radix");

    failures += test(checkBadRadix(0.0 as Double, 1), "0 Radix 1");
    failures += test(checkBadRadix(0.0 as Double, 37), "0 Radix 37");

    failures += test(checkBadRadix(1.0 as Double, 1), "1 Radix 1");
    failures += test(checkBadRadix(1.0 as Double, 37), "1 Radix 37");

    failures += test(checkBadRadix(Double.MAX_VALUE as Double, 1), "MAX_VALUE Radix 1");
    failures += test(checkBadRadix(Double.MAX_VALUE as Double, 37), "MAX_VALUE Radix 37");


    failures += test(checkBadRadix(Double.MIN_VALUE as Double, 1), "MIN_VALUE Radix 1");
    failures += test(checkBadRadix(Double.MIN_VALUE as Double, 37), "MIN_VALUE Radix 37");

    failures += test(checkBadRadix(Double.POSITIVE_INFINITY as Double, 1), "POSITIVE_INFINITY Radix 1");
    failures += test(checkBadRadix(Double.POSITIVE_INFINITY as Double, 37), "POSITIVE_INFINITY Radix 37");

    failures += test(checkBadRadix(Double.NEGATIVE_INFINITY as Double, 1), "NEGATIVE_INFINITY Radix 1");
    failures += test(checkBadRadix(Double.NEGATIVE_INFINITY as Double, 37), "NEGATIVE_INFINITY Radix 37");

    failures += test(checkBadRadix(Double.NaN as Double, 1), "NaN Radix 1");
    failures += test(checkBadRadix(Double.NaN as Double, 37), "NaN Radix 37");

    return test(failures, "All tests run");
}

function test(result: int, message: String ): int {
    if(result == 0) {
      console.println("PASSED: " + message)
      return 0;
    }
    console.println("FAILED: " + message);
    return 1;
}

function checkBadRadix(val: Double, radix: int):int {

    let sv: String;
    try {
        sv = val.toString(radix);
    } catch(aoe: ArgumentOutOfRangeException) {
        return 0;
    } catch(ue: Exception) {
        console.println("Unknown exeption catched" + ue)
    }

    if(sv == null) {
        console.println("Returned null");
    }
    console.print("String: ");
    console.print(sv);
    console.println();
    return 1;
}
