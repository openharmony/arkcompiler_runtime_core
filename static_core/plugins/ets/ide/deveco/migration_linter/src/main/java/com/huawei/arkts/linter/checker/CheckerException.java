package com.huawei.arkts.linter.checker;

public class CheckerException extends Exception {
    public CheckerException(String message) {
        super(message);
    }

    public CheckerException(Throwable cause) {
        super(cause);
    }
}
