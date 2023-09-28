function main():int {
    let failures = 0;
    failures += test(checkGoodRadix(Double.POSITIVE_INFINITY, 2, "Infinity"), "Check Positive Infinity with radix 2");
    failures += test(checkGoodRadix(Double.NEGATIVE_INFINITY, 2, "-Infinity"), "Check Negatibe Infinity with radix 2");
    failures += test(checkGoodRadix(Double.NaN, 2, "NaN"), "Check Nan with radix 2");

    failures += test(checkGoodRadix(Double.POSITIVE_INFINITY, 8, "Infinity"), "Check Positive Infinity with radix 8");
    failures += test(checkGoodRadix(Double.NEGATIVE_INFINITY, 8, "-Infinity"), "Check Negatibe Infinity with radix 8");
    failures += test(checkGoodRadix(Double.NaN, 8, "NaN"), "Check Nan with radix 8");

    failures += test(checkGoodRadix(Double.POSITIVE_INFINITY, 10, "Infinity"), "Check Positive Infinity with radix 10");
    failures += test(checkGoodRadix(Double.NEGATIVE_INFINITY, 10, "-Infinity"), "Check Negatibe Infinity with radix 10");
    failures += test(checkGoodRadix(Double.NaN, 10, "NaN"), "Check Nan with radix 10");

    failures += test(checkGoodRadix(Double.POSITIVE_INFINITY, 16, "Infinity"), "Check Positive Infinity with radix 16");
    failures += test(checkGoodRadix(Double.NEGATIVE_INFINITY, 16, "-Infinity"), "Check Negatibe Infinity with radix 16");
    failures += test(checkGoodRadix(Double.NaN, 16, "NaN"), "Check Nan with radix 10");

    return test(failures, "All tests run");

}

function test(result: int, message: String ): int {
    if(result == 0) {
      return 0;      
    }
    console.println("FAILED: " + message);
    return 1;
}

function checkGoodRadix(val: Double, radix: int, expected: String):int {
    
    let sv: String;
    try {
        sv = val.toString(radix);
    } catch(aoe: ArgumentOutOfRangeException) {
        return 1;
    }
    
    if(sv == expected) return 0;

    return 1;
}
