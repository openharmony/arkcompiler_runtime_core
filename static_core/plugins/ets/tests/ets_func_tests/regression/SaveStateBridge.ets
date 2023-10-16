class C0 {
    x0: boolean
}

class C1 {
    x1: C0
}

function main() {
    let x1: C1 = {x1: {x0: false}}
    assert(!x1.x1.x0)   
    while(x1.x1.x0) {}
}
