import logging
import subprocess
from os import path, listdir, getenv, chdir
from typing import Dict

from runner.logger import Log

_LOGGER = logging.getLogger("runner.plugins_registry")


class Singleton(type):
    _instances: Dict = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]


class PluginsRegistry(metaclass=Singleton):
    ENV_PLUGIN_PATH = "PLUGIN_PATH"
    BUILTIN_PLUGINS = "plugins"
    BUILTIN_ROOT = "runner"

    def __init__(self):
        self.registry = {}
        self.side_plugins = []
        self.load_from_env()
        self.load_builtin_plugins()

    @staticmethod
    def filter_builtins(item_list):
        return [item for item in item_list if not item.startswith("__")]

    @staticmethod
    def my_dir(obj):
        return PluginsRegistry.filter_builtins(dir(obj))

    def load_plugin(self, plugin_name: str, plugin_path: str):
        runner_class = [
            cls
            for cls in PluginsRegistry.filter_builtins(listdir(plugin_path))
            if cls.startswith("runner_")
        ]
        runner_class_name = runner_class.pop() if len(runner_class) > 0 else None
        if runner_class_name is not None:
            last_dot = runner_class_name.rfind(".")
            runner_class_name = runner_class_name[:last_dot]
            class_module_name = f"{PluginsRegistry.BUILTIN_ROOT}" \
                                f".{PluginsRegistry.BUILTIN_PLUGINS}" \
                                f".{plugin_name}.{runner_class_name}"
            class_module_root = __import__(class_module_name)
            class_module_plugin = getattr(getattr(class_module_root, PluginsRegistry.BUILTIN_PLUGINS), plugin_name)
            class_module_runner = getattr(class_module_plugin, runner_class_name)
            classes = PluginsRegistry.my_dir(class_module_runner)
            classes = [cls for cls in classes if cls.startswith("Runner") and cls.lower().endswith(plugin_name.lower())]
            class_name = classes.pop() if len(classes) > 0 else None
            if class_name is not None:
                class_obj = getattr(class_module_runner, class_name)
                self.add(plugin_name, class_obj)

    def load_builtin_plugins(self):
        starting_path = path.join(path.dirname(__file__), PluginsRegistry.BUILTIN_PLUGINS)
        plugin_names = PluginsRegistry.filter_builtins(listdir(starting_path))

        for plugin_name in plugin_names:
            plugin_path = path.join(starting_path, plugin_name)
            if path.isdir(plugin_path):
                self.load_plugin(plugin_name, plugin_path)
            else:
                Log.all(_LOGGER, f"Found extra file '{plugin_path}' at plugins folder")

    def load_from_env(self):
        builtin_plugins_path = path.join(path.dirname(__file__), PluginsRegistry.BUILTIN_PLUGINS)
        side_plugins = getenv(PluginsRegistry.ENV_PLUGIN_PATH, "").split(path.pathsep)
        if len(side_plugins) > 0:
            chdir(builtin_plugins_path)
            for side_plugin in side_plugins:
                if path.exists(side_plugin):
                    cmd = ["ln", "-s", side_plugin]
                    subprocess.run(cmd, check=True)
                    self.side_plugins.append(path.join(
                        path.dirname(__file__),
                        PluginsRegistry.BUILTIN_PLUGINS,
                        path.basename(side_plugin)
                    ))

    def add(self, runner_name: str, runner):
        if runner_name not in self.registry:
            self.registry[runner_name] = runner
            Log.all(_LOGGER, f"Registered plugin '{runner_name}' with class '{runner.__name__}'")
        else:
            Log.exception_and_raise(_LOGGER, f"Plugin '{runner_name}' already registered")

    def get_runner(self, name: str):
        return self.registry.get(name, None)

    def is_registered(self, name: str) -> bool:
        return name in self.registry

    def cleanup(self):
        if len(self.side_plugins) > 0:
            cmd = ["rm"]
            for side_plugin in self.side_plugins:
                cmd.append(side_plugin)
            subprocess.run(cmd, check=True)
