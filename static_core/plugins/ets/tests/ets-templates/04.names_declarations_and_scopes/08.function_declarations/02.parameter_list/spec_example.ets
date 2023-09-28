/*---
desc: >-
  A signature contains a parameter list that specifies an identifier
  and a type of each parameter. Every parameter's type must be
  explicitly defined.
---*/

function power(base: number, exponent: number): number {
    return Math.pow(base, exponent)
}

function main() {
    assert 8 == power(2, 3) // both arguments are required in the call
}
