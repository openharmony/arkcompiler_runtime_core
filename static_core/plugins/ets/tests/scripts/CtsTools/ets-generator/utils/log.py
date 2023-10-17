import sys
import time
import traceback


def __print_formatted(severity, msg):
    timestamp = int(round(time.time() * 1000))
    print("{} {} [Host] - {}".format(timestamp, severity, msg), file=sys.stderr)


def info(msg, extra_line=False):
    if extra_line:
        print("\n", file=sys.stderr)
    __print_formatted("INFO", msg)


def exception(msg):
    (_, value, tb) = sys.exc_info()
    __print_formatted("ERROR", f"{str(value)} \n message = {msg}")
    traceback.print_tb(tb, file=sys.stderr)
