/*---
desc: 03.08.11 Union types
name: 03.types/08.reference_types/03.08.11.union_types/union_param_06
tags: []
---*/
type Margin = number | string

function formatNumber(n: string) {
    assert n == "5 px"
}

function formatMargin(margin: Margin) {
    if (margin instanceof string) {
        formatNumber(margin as string)
    }
}

function main(): void {
    formatMargin("5 px")
    let a: Margin = "5 px"
    formatMargin(a)
}
