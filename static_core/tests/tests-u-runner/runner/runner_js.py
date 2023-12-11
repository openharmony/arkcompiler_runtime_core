import logging
from os import path

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.options.config import Config
from runner.runner_file_based import RunnerFileBased, PandaBinaries
from runner.utils import get_platform_binary_name

_LOGGER = logging.getLogger("runner.runner_js")


class PandaJSBinaries(PandaBinaries):
    @property
    def ark_quick(self) -> str:
        if self.conf_kind == ConfigurationKind.QUICK:
            return self.check_path(self.config.general.build, 'bin', get_platform_binary_name('arkquick'))
        return super().ark_quick


class RunnerJS(RunnerFileBased):
    def __init__(self, config: Config, name: str) -> None:
        RunnerFileBased.__init__(self, config, name, PandaJSBinaries)
        ecmastdlib_abc: str = f"{self.build_dir}/pandastdlib/arkstdlib.abc"
        if not path.exists(ecmastdlib_abc):
            ecmastdlib_abc = f"{self.build_dir}/gen/plugins/ecmascript/ecmastdlib.abc"  # stdlib path for GN build

        self.quick_args = []
        if self.conf_kind == ConfigurationKind.QUICK:
            ecmastdlib_abc = self.generate_quick_stdlib(ecmastdlib_abc)

        self.runtime_args.extend([
            f'--boot-panda-files={ecmastdlib_abc}',
            '--load-runtimes=ecmascript',
        ])

        if self.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            self.aot_args.extend([
                f'--boot-panda-files={ecmastdlib_abc}',
                '--load-runtimes=ecmascript',
            ])
