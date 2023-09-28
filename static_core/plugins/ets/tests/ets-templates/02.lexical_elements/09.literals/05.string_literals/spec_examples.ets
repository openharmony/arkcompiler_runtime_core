/*---
desc: String literals consist of zero or more characters enclosed between single or double quotes.
params: examples from the specification
---*/

let b = 'Hello, world!'
let c = "Hello, world!"
let d = "\\"
let e = ""
let f = "don't do it"
let g = 'don\'t do it'
let h = 'don\u0027t do it'

function main(): void {
    assert b instanceof string
    assert c instanceof string
    assert d instanceof string
    assert e instanceof string
    assert f instanceof string
    assert g instanceof string
    assert h instanceof string
}
