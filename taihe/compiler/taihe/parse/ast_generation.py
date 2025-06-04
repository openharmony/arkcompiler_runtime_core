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
from typing import Any

from antlr4 import CommonTokenStream, FileStream, InputStream, TerminalNode, Token
from antlr4.error.ErrorListener import ErrorListener

from taihe.parse.antlr.TaiheAST import TaiheAST
from taihe.parse.antlr.TaiheLexer import TaiheLexer
from taihe.parse.antlr.TaiheParser import TaiheParser
from taihe.utils.diagnostics import AbstractDiagnosticsManager
from taihe.utils.exceptions import IDLSyntaxError
from taihe.utils.sources import SourceBase, SourceBuffer, SourceFile, SourceLocation


def get_pos(ctx: Token):
    text = ctx.text.splitlines()
    row = ctx.line
    col = ctx.column
    row_offset = len(text) - 1
    col_offset = len(text[-1])
    beg = (
        row,
        col + 1,
    )
    end = (
        row + row_offset,
        col + col_offset if row_offset == 0 else col_offset,
    )
    return beg, end


def add_pos(ctx, pos_dict: dict):
    if isinstance(ctx, Token):
        beg, end = get_pos(ctx)
        pos_dict[ctx] = beg, end
    elif isinstance(ctx, TerminalNode):
        add_pos(ctx.symbol, pos_dict)
        pos_dict[ctx] = pos_dict[ctx.symbol]
    else:
        for child in ctx.children:
            add_pos(child, pos_dict)
        beg, _ = pos_dict[ctx.children[0]]
        _, end = pos_dict[ctx.children[-1]]
        pos_dict[ctx] = beg, end


class TaiheErrorListener(ErrorListener):
    def __init__(
        self,
        source: SourceBase,
        diag: AbstractDiagnosticsManager,
        pos_dict: dict,
    ) -> None:
        super().__init__()
        self.diag = diag
        self.source = source
        self.has_error = False
        self.pos_dict = pos_dict

    def syntaxError(self, recognizer, offendingSymbol, line, column, msg, e):
        add_pos(offendingSymbol, self.pos_dict)
        (start_row, start_col), (stop_row, stop_col) = get_pos(offendingSymbol)
        self.diag.emit(
            IDLSyntaxError(
                offendingSymbol.text,
                loc=SourceLocation(
                    self.source, start_row, start_col, stop_row, stop_col
                ),
            )
        )
        self.has_error = True


def issubkind(real_kind, node_kind):
    ctx_kind = node_kind + "Context"
    ctx_type = getattr(TaiheParser, ctx_kind)
    sub_kind = real_kind + "Context"
    sub_type = getattr(TaiheParser, sub_kind)
    subclasses = ctx_type.__subclasses__()
    if subclasses:
        return sub_type in subclasses
    else:
        return real_kind == node_kind


def visit(node_kind: str, ctx, pos_dict: dict) -> Any:
    if node_kind.endswith("Lst"):
        node = []
        for sub in ctx:
            with suppress(Exception):
                node.append(visit(node_kind[:-3], sub, pos_dict))
        return node
    if node_kind.endswith("Opt"):
        node = None
        if ctx is not None:
            with suppress(Exception):
                node = visit(node_kind[:-3], ctx, pos_dict)
        return node
    beg, end = pos_dict[ctx]
    if node_kind == "TOKEN":
        return TaiheAST.TOKEN(_beg=beg, _end=end, text=ctx.text)
    kwargs = {"_beg": beg, "_end": end}
    for attr_full_name, attr_ctx in ctx.__dict__.items():
        if attr_full_name[0].isupper():
            attr_kind_name, attr_name = attr_full_name.split("_", 1)
            kwargs[attr_name] = visit(attr_kind_name, attr_ctx, pos_dict)
    real_kind = ctx.__class__.__name__[:-7]
    if not issubkind(real_kind, node_kind):
        raise ValueError(f"{real_kind} is not a subkind of {node_kind}")
    return getattr(TaiheAST, real_kind)(**kwargs)


def generate_ast(source: SourceBase, diag: AbstractDiagnosticsManager) -> TaiheAST.Spec:
    if isinstance(source, SourceBuffer):
        input_stream = InputStream(source.buf)
    elif isinstance(source, SourceFile):
        input_stream = FileStream(source.source_identifier, encoding="utf-8")
    else:
        raise NotImplementedError

    lexer = TaiheLexer(input_stream)
    token_stream = CommonTokenStream(lexer)

    pos_dict = {}

    error_listener = TaiheErrorListener(source, diag, pos_dict)

    parser = TaiheParser(token_stream)
    parser.removeErrorListeners()
    parser.addErrorListener(error_listener)
    tree = parser.spec()

    add_pos(tree, pos_dict)

    return visit("Spec", tree, pos_dict)
