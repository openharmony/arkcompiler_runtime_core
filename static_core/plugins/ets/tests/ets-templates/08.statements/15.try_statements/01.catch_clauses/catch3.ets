/*---
desc: The exception extends to the surrounding scope if no catch clause can handle the exception.
---*/

class Ex extends Exception {}
class F extends Exception {}

function main(): int {
    try {
        try {
            try{
                try {
                    try {
                        try {
                            try {
                                try {
                                    try {
                                        try {
                                            try {
                                                throw new F();
                                            } catch (e: Ex) {
                                                return 1;
                                            }
                                        } catch (e: Ex) {
                                            return 1;
                                        }
                                    } catch (e: Ex) {
                                        return 1;
                                    }
                                } catch (e: Ex) {
                                    return 1;
                                }
                            } catch (e: Ex) {
                                return 1;
                            }
                        } catch (e: Ex) {
                            return 1;
                        }
                    } catch (e: Ex) {
                        return 1;
                    }
                } catch (e: Ex) {
                    return 1;
                }
            } catch (e: Ex) {
                return 1;
            }
        } catch (e: Ex) {
            return 1;
        }
    } catch (e: Ex) {
        return 1;
    } catch (f: F) {
        return 0;
    }
    return 1;
}
