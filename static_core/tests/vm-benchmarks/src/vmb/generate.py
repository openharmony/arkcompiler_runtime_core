#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
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

import logging
from typing import List, Iterable, Set
from pathlib import Path
from shutil import rmtree
from string import Template
from dataclasses import asdict
from collections import namedtuple
from vmb.helpers import get_plugin, read_list_file, log_time, create_file, die
from vmb.unit import BenchUnit
from vmb.cli import Args
from vmb.lang import LangBase
from vmb.doclet import DocletParser, TemplateVars

SrcPath = namedtuple("SrcPath", "full rel")
log = logging.getLogger('vmb')


class BenchGenerator:
    def __init__(self, args: Args) -> None:
        self.args = args  # need to keeep full cmdline for measure overrides
        self.data_dir: Path = Path(__file__).parent.joinpath('templates')
        self.paths: List[Path] = args.paths
        self.out_dir: Path = Path(args.outdir).joinpath('benches').resolve()
        self.override_src_ext: Set[str] = args.src_langs
        self.extra_plug_dir = args.extra_plugins
        if self.out_dir.is_dir():
            rmtree(str(self.out_dir))

    @staticmethod
    def search_test_files_in_dir(d: Path,
                                 root: Path,
                                 ext: Iterable[str] = ()) -> List[SrcPath]:
        files = []
        for p in d.glob('**/*'):
            if p.parent.parent.name == 'common':
                continue
            if p.suffix and p.suffix in ext:
                log.debug('Src: %s', str(p))
                full = p.resolve()
                files.append(
                    SrcPath(full, full.parent.relative_to(root)))
        return files

    @staticmethod
    def process_test_list(lst: Path, ext: Iterable[str] = ()) -> List[SrcPath]:
        cwd = Path.cwd().resolve()
        paths = [cwd.joinpath(p) for p in read_list_file(lst)]
        files = []
        for p in paths:
            x = BenchGenerator.search_test_files_in_dir(p, p, ext)
            files += x
        return files

    @staticmethod
    def search_test_files(paths: List[Path],
                          ext: Iterable[str] = ()) -> List[SrcPath]:
        """Collect all src files to gen process.

        Returns flat list of (Full, Relative) paths
        """
        log.debug('Searching sources: **/*%r', ext)
        files = []
        for d in paths:
            root = d.resolve()
            # if file name provided add it if suffix matches
            if root.is_file():
                if '.lst' == root.suffix:
                    log.debug('Processing list file: %s', root)
                    files += BenchGenerator.process_test_list(root, ext)
                    continue
                if root.suffix not in ext:
                    continue
                files.append(SrcPath(root, Path('.')))
            # in case of dir search by file extention
            elif root.is_dir():
                files += BenchGenerator.search_test_files_in_dir(d, root, ext)
            else:
                log.warning('Src: %s not found!', root)
        return files

    def process_source_file(self, src: Path, lang: LangBase
                            ) -> Iterable[TemplateVars]:
        with open(src, 'r', encoding="utf-8") as f:
            full_src = f.read()
        if '@Benchmark' not in full_src:
            return []
        try:
            parser = DocletParser.create(full_src, lang).parse()
            if not parser.state:
                return []
        except ValueError as e:
            log.error('%s in %s', e, str(src))
            return []
        return TemplateVars.params_from_parsed(
            full_src, parser.state, args=self.args)

    @staticmethod
    def process_imports(lang_impl: LangBase, imports: str,
                        bench_dir: Path, src: SrcPath) -> str:
        """Process @Import and return `import` statement(s)."""
        import_lines = ''
        for import_doclet in imports:
            m = lang_impl.parse_import(import_doclet)
            if not m:
                log.warning('Bad import: %s', import_doclet)
                continue
            libpath = src.full.parent.joinpath(m[0])
            if not libpath.is_file():
                log.warning('Lib does not exist: %s', libpath)
            else:
                link = bench_dir.joinpath(libpath.name)
                if link.exists():
                    link.unlink()
                link.symlink_to(libpath)
                import_lines += m[1]
        return import_lines

    @staticmethod
    def check_common_files(full: Path, lang_name: str) -> str:
        """Check if there is 'common' code at ../common/ets/*.ets.

        This feature is actually meaningless now
        and added only for the compatibility with existing tests
        """
        if full.parent.name != lang_name:
            return ''
        common = full.parent.parent.joinpath('common', lang_name)
        if common.is_dir():
            src = ''
            for p in common.glob(f'*.{lang_name}'):
                log.trace('Common file: %s', p)
                with open(p, 'r', encoding="utf-8") as f:
                    src += f.read()
            return src
        return ''

    @staticmethod
    def emit_bench_variant(v: TemplateVars,
                           tpl: Template,
                           lang_impl: LangBase,
                           out: Path,
                           src: SrcPath) -> BenchUnit:
        log.debug('Bench Variant: %s @ %s',
                  v.BENCH_NAME, v.FIXTURE)
        tags = set(v.TAGS)
        v.TAGS = ';'.join([str(t) for t in tags])
        bugs = set(v.BUGS)
        v.BUGS = ';'.join([str(t) for t in bugs])
        # create bench unit dir
        bench_dir = out.joinpath(src.rel, f'bu_{v.BENCH_NAME}')
        bench_dir.mkdir(parents=True, exist_ok=True)
        v.METHOD_CALL = lang_impl.get_method_call(
            v.METHOD_NAME, v.METHOD_RETTYPE)
        v.IMPORTS = BenchGenerator.process_imports(
            lang_impl, v.IMPORTS, bench_dir, src)
        v.COMMON = BenchGenerator.check_common_files(
            src.full, lang_impl.short_name)
        bench = tpl.substitute(asdict(v))
        bench_file = bench_dir.joinpath(f'bench_{v.BENCH_NAME}{lang_impl.ext}')
        log.info('Bench: %s', bench_file)
        with create_file(bench_file) as f:
            f.write(bench)
        return BenchUnit(bench_dir, tags=tags, bugs=bugs)

    def generate_lang(self, lang: str, quick_abort: bool = False
                      ) -> List[BenchUnit]:
        """Generate benchmark sources for requested language."""
        bus = []
        lang_plugin = get_plugin('langs', lang, extra=self.extra_plug_dir)
        lang_impl: LangBase = lang_plugin.Lang()
        log.info('Using lang: %s', lang_impl.name)
        template = self.data_dir.joinpath(f'Template{lang_impl.ext}')
        with open(template, 'r', encoding="utf-8") as f:
            tpl = Template(f.read())
        ext = self.override_src_ext if self.override_src_ext else lang_impl.src
        for src in BenchGenerator.search_test_files(self.paths, ext=ext):
            for v in self.process_source_file(src.full, lang_impl):
                try:
                    bu = BenchGenerator.emit_bench_variant(
                        v, tpl, lang_impl, self.out_dir, src)
                    bus.append(bu)
                # pylint: disable-next=broad-exception-caught
                except Exception as e:
                    log.error(e)
                    die(quick_abort, 'Aborting on first fail...')
        return bus


@log_time
def generate_main(args: Args) -> List[BenchUnit]:
    """Command: Generate benches from doclets."""
    log.info("Starting GEN phase...")
    generator = BenchGenerator(args)
    bus: List[BenchUnit] = []
    for lang in args.langs:
        bus += generator.generate_lang(lang, quick_abort=args.abort_on_fail)
    log.passed('Generated %d bench units', len(bus))
    return bus


if __name__ == '__main__':
    generate_main(Args())
