# coding=utf-8
#
# Copyright (c) 2025 Huawei Device Co., Ltd.
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

from contextlib import suppress
from typing import Any, cast

from antlr4.CommonTokenStream import CommonTokenStream
from antlr4.error.ErrorListener import ErrorListener
from antlr4.FileStream import FileStream
from antlr4.InputStream import InputStream
from antlr4.ParserRuleContext import ParserRuleContext
from antlr4.Token import Token
from antlr4.tree.Tree import ErrorNodeImpl, TerminalNodeImpl, Tree

from taihe.parse.antlr.TaiheAST import TaiheAST
from taihe.parse.antlr.TaiheLexer import TaiheLexer
from taihe.parse.antlr.TaiheParser import TaiheParser
from taihe.utils.diagnostics import DiagnosticsManager
from taihe.utils.exceptions import IDLSyntaxError
from taihe.utils.sources import (
    SourceBase,
    SourceBuffer,
    SourceFile,
    SourceLocation,
    TextPosition,
    TextRange,
)


class SourceCodeLocator:
    def __init__(self, source: SourceBase) -> None:
        self.source = source
        self._cache: dict[Tree | Token, TextRange | None] = {}

    def get_range(self, ctx: Tree | Token) -> TextRange | None:
        pos = self._cache.get(ctx, KeyError())
        if not isinstance(pos, KeyError):
            return pos
        elif isinstance(ctx, Token):
            text = cast(str, ctx.text).splitlines()  # type: ignore
            row = cast(int, ctx.line)
            col = cast(int, ctx.column)
            row_offset = len(text) - 1
            col_offset = len(text[-1])
            beg = TextPosition(
                row,
                col + 1,
            )
            end = TextPosition(
                row + row_offset,
                col + col_offset if row_offset == 0 else col_offset,
            )
            pos = TextRange(beg, end)
        elif isinstance(ctx, TerminalNodeImpl | ErrorNodeImpl):
            pos = self.get_range(ctx.symbol)
        elif isinstance(ctx, ParserRuleContext):
            text_ranges: list[TextRange] = []
            children = cast(list[Tree | Token], ctx.children) or []
            for child in children:
                self.get_range(child)
                if (pair := self._cache.get(child)) is not None:
                    text_ranges.append(pair)
            if not text_ranges:
                pos = None
            else:
                pos = TextRange(text_ranges[0].start, text_ranges[-1].stop)
        else:
            raise TypeError(f"Unsupported type {type(ctx)} for ASTRecorder.get_pos()")
        return self._cache.setdefault(ctx, pos)

    def get_loc(self, ctx: Tree | Token) -> SourceLocation:
        return SourceLocation(self.source, self.get_range(ctx))


class TaiheErrorListener(ErrorListener):
    def __init__(
        self,
        recorder: SourceCodeLocator,
        diag: DiagnosticsManager,
    ) -> None:
        super().__init__()
        self.recorder = recorder
        self.diag = diag

    def syntaxError(
        self,
        recognizer: Any,
        offendingSymbol: Token,
        line: int,
        column: int,
        msg: str,
        e: Any,
    ):
        self.diag.emit(
            IDLSyntaxError(
                cast(str, offendingSymbol.text),  # type: ignore
                loc=self.recorder.get_loc(offendingSymbol),
            )
        )


def is_qualified(real_kind: str, node_kind: str):
    ctx_kind = node_kind + "Context"
    ctx_type = getattr(TaiheParser, ctx_kind)
    sub_kind = real_kind + "Context"
    sub_type = getattr(TaiheParser, sub_kind)
    subclasses = ctx_type.__subclasses__()
    if subclasses:
        return sub_type in subclasses
    else:
        return real_kind == node_kind


class TaiheASTConverter:
    def __init__(self, recorder: SourceCodeLocator):
        self.recorder = recorder

    def visit(self, node_kind: str, ctx: Any) -> Any:
        if node_kind.endswith("Lst"):
            lst: list[Any] = []
            for sub in ctx:
                with suppress(Exception):
                    lst.append(self.visit(node_kind[:-3], sub))
            return lst
        if node_kind.endswith("Opt"):
            opt_node: Any | None = None
            if ctx is not None:
                with suppress(Exception):
                    opt_node = self.visit(node_kind[:-3], ctx)
            return opt_node
        loc = self.recorder.get_loc(ctx)
        if node_kind == "TOKEN":
            return TaiheAST.TOKEN(loc=loc, text=ctx.text)
        kwargs = {"loc": loc}
        for attr_full_name, attr_ctx in ctx.__dict__.items():
            if attr_full_name[0].isupper():
                attr_kind_name, attr_name = attr_full_name.split("_", 1)
                kwargs[attr_name] = self.visit(attr_kind_name, attr_ctx)
        real_kind = ctx.__class__.__name__[:-7]  # Remove the trailing "Context"
        pass
        return getattr(TaiheAST, real_kind)(**kwargs)


def generate_ast(source: SourceBase, diag: DiagnosticsManager) -> TaiheAST.Spec:
    if isinstance(source, SourceBuffer):
        input_stream = InputStream(source.buf)
    elif isinstance(source, SourceFile):
        input_stream = FileStream(source.source_identifier, encoding="utf-8")
    else:
        raise NotImplementedError

    lexer = TaiheLexer(input_stream)
    token_stream = CommonTokenStream(lexer)

    recorder = SourceCodeLocator(source)
    error_listener = TaiheErrorListener(recorder, diag)
    converter = TaiheASTConverter(recorder)

    parser = TaiheParser(token_stream)
    parser.removeErrorListeners()
    parser.addErrorListener(error_listener)  # type: ignore
    tree = parser.spec()
    return converter.visit("Spec", tree)