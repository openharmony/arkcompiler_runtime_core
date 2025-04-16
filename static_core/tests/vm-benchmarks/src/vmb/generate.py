#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
import json
from typing import List, Iterable, Set, Optional, Dict, Any
from pathlib import Path
from shutil import rmtree
from string import Template
from dataclasses import asdict
from collections import namedtuple
from vmb.helpers import get_plugin, read_list_file, \
    log_time, create_file, die, force_link
from vmb.unit import BenchUnit, BENCH_PREFIX
from vmb.cli import Args
from vmb.lang import LangBase
from vmb.doclet import DocletParser, TemplateVars
from vmb.gensettings import GenSettings
from vmb.shell import ShellUnix

SrcPath = namedtuple("SrcPath", "full rel")
log = logging.getLogger('vmb')


class BenchGenerator:
    def __init__(self, args: Args) -> None:
        self.args = args  # need to keep full cmdline for measure overrides
        self.paths: List[Path] = args.paths
        self.out_dir: Path = Path(args.outdir).joinpath('benches').resolve()
        self.override_src_ext: Set[str] = args.src_langs
        self.extra_plug_dir = args.extra_plugins
        self.template_dirs: List[Path] = [
            p.joinpath('templates')
            # extra first (if exists)
            for p in (self.extra_plug_dir, Path(__file__).parent) if p]
        self.abort = args.abort_on_fail
        if self.out_dir.is_dir():
            rmtree(str(self.out_dir))

    @staticmethod
    def search_test_files_in_dir(d: Path,
                                 root: Path,
                                 ext: Iterable[str] = (),
                                 allowed_dir_name: Optional[str] = None) -> List[SrcPath]:
        if allowed_dir_name:
            log.trace('Search test files, allowed dir name: %s', allowed_dir_name)
        files = []
        for p in d.glob('**/*'):
            if allowed_dir_name and not p.parent.name == allowed_dir_name:
                continue
            if p.parent.parent.name == 'common':
                continue
            if p.suffix and p.suffix in ext:
                log.trace('Src: %s', str(p))
                full = p.resolve()
                files.append(
                    SrcPath(full, full.parent.relative_to(root.resolve())))
        return files

    @staticmethod
    def process_test_list(lst: Path, ext: Iterable[str] = (),
                          allowed_dir_name: Optional[str] = None) -> List[SrcPath]:
        cwd = Path.cwd().resolve()
        paths = [cwd.joinpath(p) for p in read_list_file(lst)]
        files = []
        for p in paths:
            x = BenchGenerator.search_test_files_in_dir(p, p, ext, allowed_dir_name)
            files += x
        return files

    @staticmethod
    def search_test_files(paths: List[Path],
                          ext: Iterable[str] = (),
                          allowed_dir_name: Optional[str] = None) -> List[SrcPath]:
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
                    files += BenchGenerator.process_test_list(root, ext, allowed_dir_name)
                    continue
                if root.suffix not in ext:
                    continue
                files.append(SrcPath(root, Path('.')))
            # in case of dir search by file extension
            elif root.is_dir():
                files += BenchGenerator.search_test_files_in_dir(d, root, ext, allowed_dir_name)
            else:
                log.warning('Src: %s not found!', root)
        return files

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
                force_link(bench_dir.joinpath(libpath.name), libpath)
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
        common = common if common.is_dir() else \
            full.parent.parent.parent.joinpath('common', lang_name)
        if common.is_dir():
            src = ''
            log.trace('Common dir: %s', common)
            for p in common.glob(f'*.{lang_name}'):
                log.trace('Common file: %s', p)
                with open(p, 'r', encoding="utf-8") as f:
                    src += f.read()
            return src
        return ''

    @staticmethod
    def check_resources(full: Path, lang_name: str, dest: Path) -> bool:
        """Check 'resources' at ../ets/*.ets and link to destdir."""
        if full.parent.name != lang_name:
            return False
        resources = full.parent.parent.joinpath('resources')
        if resources.is_dir():
            log.trace('Resources: %s', resources)
            force_link(dest.joinpath('resources'), resources)
            return True
        return False

    @staticmethod
    def check_native(full: Path, dest: Path, values: Dict[str, Any]) -> bool:
        """Check 'native' near the source and link to destdir."""
        native = full.parent.joinpath('native')
        if not native.is_dir():
            return False
        log.debug('Native: %s', native)
        dest_dir = dest.joinpath('native')
        dest_dir.mkdir(parents=True, exist_ok=True)
        for f in native.glob('*'):
            if f.is_file():
                dest_file = dest_dir.joinpath(f.name)
                with open(f, 'r', encoding='utf-8') as t:
                    native_tpl = t.read()
                tpl = Template(native_tpl)
                with create_file(dest_file) as d:
                    d.write(tpl.substitute(values))
        return True

    @staticmethod
    def write_config(bench_dir: Path, values: TemplateVars):
        with create_file(bench_dir.joinpath('config.json')) as f:
            f.write(json.dumps(values.config))

    @staticmethod
    def process_generator(src_full: Path, bench_dir: Path,
                          values: TemplateVars, ext: str):
        script = src_full.parent.joinpath(values.generator)
        cmd = f'{script} {bench_dir} bench_{values.bench_name}{ext}'
        log.trace('Test generator: %s', script)
        ShellUnix().run(cmd)

    @staticmethod
    def emit_bench_variant(values: TemplateVars,
                           template: Template,
                           lang_impl: LangBase,
                           src: SrcPath,
                           outdir: Path,
                           outext: str) -> BenchUnit:
        log.trace('Bench Variant: %s @ %s',
                  values.bench_name, values.fixture)
        # create bench unit dir
        bench_dir = outdir.joinpath(src.rel, f'bu_{values.bench_name}')
        bench_dir.mkdir(parents=True, exist_ok=True)
        values.bench_path = str(bench_dir)
        # process template values
        tags = set(values.tags)
        values.tags = ';'.join([str(t) for t in tags])
        bugs = set(values.bugs)
        values.bugs = ';'.join([str(t) for t in bugs])
        values.method_call = lang_impl.get_method_call(
            values.method_name, values.method_rettype)
        values.imports = BenchGenerator.process_imports(
            lang_impl, values.imports, bench_dir, src)
        values.common = BenchGenerator.check_common_files(
            src.full, lang_impl.short_name)
        tpl_values = asdict(values)
        # create links to extra dirs if any
        custom_values = {
            'resources': BenchGenerator.check_resources(
                src.full, lang_impl.short_name, bench_dir),
            'native': BenchGenerator.check_native(
                src.full, bench_dir, tpl_values)
        }
        tpl_values.update(
            lang_impl.get_custom_fields(tpl_values, custom_values))
        # fill template with values
        bench = template.substitute(tpl_values)
        bench_file = bench_dir.joinpath(f'bench_{values.bench_name}{outext}')
        log.trace('Bench: %s', bench_file)
        with create_file(bench_file) as f:
            f.write(bench)
        if values.generator or values.disable_inlining or values.aot_opts:
            BenchGenerator.write_config(bench_dir, values)
        if values.generator:
            BenchGenerator.process_generator(
                src.full, bench_dir, values, outext)
        return BenchUnit(bench_dir, src=src.full, tags=tags, bugs=bugs)

    def get_lang(self, lang: str) -> LangBase:
        lang_plugin = get_plugin('langs', lang, extra=self.extra_plug_dir)
        lang_impl: LangBase = lang_plugin.Lang()
        log.info('Using lang: %s', lang_impl.name)
        return lang_impl

    def get_template(self, name: str) -> Template:
        for d in self.template_dirs:
            template_path = d.joinpath(name)
            if not template_path.exists():
                continue
            log.debug('Using template: %s', template_path)
            with open(template_path, 'r', encoding="utf-8") as f:
                tpl = Template(f.read())
            return tpl
        raise RuntimeError(f'Template {name} not found!')

    def process_source_file(self, src: Path, lang: LangBase) -> Iterable[TemplateVars]:
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
            die(self.abort, 'Aborting on first error...')
            return []
        return TemplateVars.params_from_parsed(
            full_src, parser.state, args=self.args)

    def add_bu(self, bus: List[BenchUnit], template: Template,
               lang_impl: LangBase, src: SrcPath, variant: TemplateVars,
               settings: Optional[GenSettings], out_ext: str) -> BenchUnit:
        try:
            bu = BenchGenerator.emit_bench_variant(
                variant, template, lang_impl, src, self.out_dir, out_ext)
            if settings and settings.link_to_src:
                link = bu.path.joinpath(
                    f'{BENCH_PREFIX}{bu.name}{Path(src.full).suffix}')
                force_link(link, src.full)
            bus.append(bu)
        # pylint: disable-next=broad-exception-caught
        except Exception as e:
            log.error(e)
            die(self.abort, 'Aborting on first fail...')
        return bu

    def generate(self, lang: str,
                 settings: Optional[GenSettings] = None) -> List[BenchUnit]:
        """Generate benchmark sources for requested language."""
        bus: List[BenchUnit] = []
        lang_impl = self.get_lang(lang)
        src_ext = lang_impl.src
        out_ext = lang_impl.ext
        template_name = f'Template{lang_impl.ext}'
        if settings:  # override if set in platform
            src_ext = settings.src
            out_ext = settings.out
            template_name = settings.template
        if self.override_src_ext:  # override if set in cmdline
            src_ext = self.override_src_ext
        template = self.get_template(template_name)
        for src in BenchGenerator.search_test_files(self.paths, ext=src_ext):
            for variant in self.process_source_file(src.full, lang_impl):
                self.add_bu(bus, template, lang_impl, src,
                            variant, settings, out_ext)
        return bus


@log_time
def generate_main_regular(args: Args,
                          settings: Optional[GenSettings] = None) -> List[BenchUnit]:
    """Command: Generate benches from doclets."""
    log.info("Starting GEN phase...")
    log.trace("GEN phase args:  %s", args)
    generator = BenchGenerator(args)
    bus: List[BenchUnit] = []
    for lang in args.langs:
        bus += generator.generate(lang, settings=settings)
    log.passed('Generated %d bench units', len(bus))
    return bus


def generate_mode(mode: str, lang: str,
                  generator: BenchGenerator,
                  settings: Optional[GenSettings] = None, arkjs_suffix: str = '') -> List[BenchUnit]:
    """Generate benchmark sources for requested language."""
    log.trace("Interop GEN for mode: %s", mode)
    bus: List[BenchUnit] = []
    lang_impl = generator.get_lang(lang)
    src_ext = lang_impl.src
    out_ext = lang_impl.ext
    template_name = f'Template{lang_impl.ext}'
    if settings:  # override if set in platform
        src_ext = settings.src
        out_ext = settings.out
        template_name = settings.template
    template = generator.get_template(template_name)
    for src in BenchGenerator.search_test_files(generator.paths, ext=src_ext, allowed_dir_name=mode):
        for variant in generator.process_source_file(src.full, lang_impl):
            bu = generator.add_bu(bus, template, lang_impl, src,
                                  variant, settings, out_ext)
            if mode == 'bu_a2j' and lang == 'ets':
                create_interop_runner(generator, variant, bu, arkjs_suffix=arkjs_suffix)
    return tags_workaround(bus, mode)


def create_interop_runner(generator: BenchGenerator, variant: TemplateVars, bu: BenchUnit, arkjs_suffix: str = ''):
    runner_template = generator.get_template('TemplateInteropA2J' + arkjs_suffix + '.tpl_js')
    runner_js = runner_template.substitute({'METHOD': variant.method_name,
                                            'STATE': variant.state_name,
                                            'NAME': variant.bench_name})
    runner_file = bu.path.joinpath('InteropRunner.js')
    log.trace("Generating %s", str(runner_file))
    with create_file(runner_file) as f:
        f.write(runner_js)


def tags_workaround(bus: List[BenchUnit], mode: str) -> List[BenchUnit]:
    if not bus:
        return bus
    for bu in bus:
        if hasattr(bu, 'tags') and bu.tags:
            continue  # Note need to find out why this is sometimes not the case
        tags: Set[str] = set()
        tags.add(mode)
        bu.tags = tags
    return bus


@log_time
def generate_main_interop(generator: BenchGenerator, arkjs_suffix: str = '') -> List[BenchUnit]:
    """Command: Generate benches from doclets."""
    log.info("Starting interop GEN phase...",)
    bus: List[BenchUnit] = []
    if not generator:
        log.warning("Generator for interop is not defined, stop")
        return bus  # empty
    # Note a2a and a2j templates are like in arkts_host
    bus += generate_mode('bu_a2a', 'ets', generator, settings=GenSettings(src={'.ets'},
                         template='Template.ets',
                         out='.ets',
                         link_to_src=False))
    bus += generate_mode('bu_a2j', 'ets', generator, settings=GenSettings(src={'.ets'},
                         template='Template.ets',
                         out='.ets',
                         link_to_src=False), arkjs_suffix=arkjs_suffix)
    bus += generate_mode('bu_j2a', 'js', generator, settings=GenSettings(src={'.js'},
                         template='TemplateInteropJ2A' + arkjs_suffix + '.tpl_js',
                         out='.js',
                         link_to_src=False))
    bus += generate_mode('bu_j2j', 'js', generator, settings=GenSettings(src={'.js'},
                         template='Template.tpl_js',
                         out='.js',
                         link_to_src=False))
    log.passed('Generated %d bench units', len(bus))
    return sorted(bus, key=lambda x: x.path)


def generate_main(args: Args,
                  settings: Optional[GenSettings] = None) -> List[BenchUnit]:
    if args.langs and 'interop' in args.langs:
        return generate_main_interop(BenchGenerator(args))
    if args.langs and 'interop_arkjs' in args.langs:
        return generate_main_interop(BenchGenerator(args), arkjs_suffix='_arkjs')
    return generate_main_regular(args, settings)


if __name__ == '__main__':
    generate_main(Args())
