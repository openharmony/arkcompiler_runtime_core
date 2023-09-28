/*---
desc: Multi-line string literals that can include embedded expressions are called Template literals
---*/

let sentence = `This is an example of multi-line string,
 which should be enclosed in backticks (\`)`

function main() {
    let sentence2 = "This is an example of multi-line string,\n which should be enclosed in backticks (\\`)"
    assert sentence == sentence2
}
