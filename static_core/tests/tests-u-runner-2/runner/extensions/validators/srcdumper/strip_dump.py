#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

"""
strip_dump.py - Utility for stripping extraneous content from source-dumper output files.

This module provides functionality to clean up dump files produced by the source dumper
by removing the first line (typically a header or command invocation line) and any
trailing lines that contain a ``"Fatal error"`` marker.  The cleaned content is
either written to a target file or printed to standard output.

Usage as a script::

    python strip_dump.py <path-to-dump-file> [<path-to-output-file>]

If only one argument is provided the stripped output is written to *stdout*.
If a second argument is provided the stripped output is written to that file.

Exit codes:
    0 - success
    1 - file could not be read or written (OSError)
"""
import sys
from pathlib import Path


def strip_dump(dump: Path, target: Path | None = None) -> None:
    """Strip the first line and non-fatal trailing lines from a dump file.

    Reads the dump file at *dump*, removes:

    * The very first line (index 0), which typically contains a shell command or
      invocation header that is not part of the actual dump output.
    * Any consecutive trailing lines that do contain the substring
      ``"Fatal error"``.  Trailing lines are examined from the end of the file
      backwards; once a line containing ``"Fatal error"`` is encountered the
      search stops and the preceding (non-fatal) lines are discarded.

    The resulting lines are joined with newline characters.  If *target* is
    given the result is written to *target*; otherwise it is written to
    *stdout*.

    Args:
        dump: :class:`pathlib.Path` pointing to the dump file to be processed.
        target: Optional :class:`pathlib.Path` for the output file.  When
            ``None`` (default) the stripped content is printed to *stdout*.

    Raises:
        OSError: If the dump file cannot be read or the target file cannot be
            written.

    Example::

        from pathlib import Path
        # Write stripped output to a separate file
        strip_dump(Path("/tmp/my_dump.txt"), Path("/tmp/my_dump_stripped.txt"))

        # Print stripped output to stdout
        strip_dump(Path("/tmp/my_dump.txt"))
    """
    lines = dump.read_text().splitlines()
    lines = lines[1:]
    remove_index = 0
    for i, line in enumerate(lines[::-1]):
        if "Fatal error" not in line:
            remove_index = i
            break
    if remove_index > 0:
        lines = lines[:-1 * remove_index]
    if target is not None:
        target.write_text("\n".join(lines) + '\n', encoding='utf-8')
    else:
        sys.stdout.write("\n".join(lines) + '\n')


def main() -> int:
    """Entry point for command-line usage.

    Parses command-line arguments and delegates to :func:`strip_dump`.

    Positional arguments (via ``sys.argv``):

    1. ``<dump>`` - path to the dump file to be stripped (required).
    2. ``<target>`` - path for the output file (optional).  When omitted the
       stripped content is written to *stdout*.

    Returns:
        int: ``0`` on success, ``1`` if an :exc:`OSError` is raised while
        reading the dump file or writing the output.

    Example::

        python strip_dump.py /path/to/dump.txt /path/to/output.txt
    """
    args = sys.argv[1:]
    dump = Path(args[0])
    target = Path(args[1]) if len(args) > 1 else None
    try:
        strip_dump(dump, target)
        return 0
    except OSError as ex:
        sys.stderr.write(f"Cannot read/write a file {dump} ({ex})\n")
        return 1


if __name__ == '__main__':
    sys.exit(main())
