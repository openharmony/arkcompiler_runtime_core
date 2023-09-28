/*---
desc: Throw statements cause an exception to be thrown. If no such try statement is found, then the UncatchedExceptionError is thrown.
tags: [negative]
params: UncatchedExceptionError expected
---*/

class Ex extends Exception {}
class Fx extends Exception {}

function getEx(): Exception {
    return new Ex();
}

function main(): int {
    try {
        throw getEx();
    } catch (e: Fx) {
        return 0;
    } catch (e: Error) {
        return 0;
    }
    return 0;
}
