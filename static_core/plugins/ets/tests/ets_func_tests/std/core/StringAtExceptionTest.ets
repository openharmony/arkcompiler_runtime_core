function main(): int {
    let failures = 0;

    let src: String = "All mimsy were the borgoves";
    try {
        src.at(100);
    } catch (e) {
        let b: boolean = e instanceof ArgumentOutOfRangeException;
        if(!b) {
            console.println("Unexpected exception" + e);
            return 1;
        }
    }
    src = "Twas brillig, and the slithy toves";
    try {
        src.at(-100);
    } catch (e) {
        let b: boolean = e instanceof ArgumentOutOfRangeException;
        if(!b) {
            console.println("Unexpected exception" + e);
            return 1;
        }
    }

    return 0;
}

