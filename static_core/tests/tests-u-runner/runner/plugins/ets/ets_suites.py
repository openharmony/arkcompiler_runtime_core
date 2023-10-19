from enum import Enum


class EtsSuites(Enum):
    CTS = "ets-cts"
    FUNC = "ets-func-tests"
    RUNTIME = "ets-runtime"
    GCSTRESS = "ets-gc-stress"
