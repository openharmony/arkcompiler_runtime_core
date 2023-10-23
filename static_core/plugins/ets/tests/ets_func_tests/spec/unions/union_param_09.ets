/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_param_06
tags: []
---*/

type Predicate = () => boolean
type Margin = Predicate | string

function callString(s: string): int {
    return s instanceof string ? 0 : -1
}

function callLambda(p: Predicate): int {
    return p() ? 0 : -2
}

function callMargin(a: Margin): int {
    if (a instanceof string){
        return callString(a as string)
    } else  {
        return callLambda(a as Predicate)
    }
}

function main(): int {
    let result: int = callMargin("hello")
    if (result != 0) { return result }

    let a: Margin = "hello"
    result = callMargin(a)
    if (result != 0) { return result }

    let p: Predicate = (): boolean => true
    result = callMargin(p)
    if (result != 0) { return result }

    let b: Margin = p
    result = callMargin(a)

    return result
}
