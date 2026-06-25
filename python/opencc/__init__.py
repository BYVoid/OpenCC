import importlib.util
import os
from typing import Optional

__all__ = ['CONFIGS', 'OpenCC', '__version__']

_this_dir = os.path.dirname(os.path.abspath(__file__))
_opencc_share_dir = os.path.join(_this_dir, 'clib', 'share', 'opencc')
_opencc_rootdir = os.path.abspath(os.path.join(_this_dir, '..', '..'))
_opencc_configdir = os.path.join(_opencc_rootdir, 'data', 'config')

if os.path.isdir(_opencc_share_dir):
    CONFIGS = [f for f in os.listdir(_opencc_share_dir) if f.endswith('.json')]
elif os.path.isdir(_opencc_configdir):
    CONFIGS = [f for f in os.listdir(_opencc_configdir) if f.endswith('.json')]
else:
    CONFIGS = []
_CONFIG_STEMS = {config[:-5] for config in CONFIGS}


def _append_path_to_env(name: str, path: str) -> None:
    value = os.environ.get(name, '')
    if path in value:  # Path already exists
        return
    if value == '':
        value = path
    else:
        value += f'{os.pathsep}{path}'
    os.environ[name] = value


def _load_opencc_clib_from_runfiles():
    candidate_dirs = [
        os.path.join(_opencc_rootdir, 'src', 'pyd'),
        os.path.join(_opencc_rootdir, 'src'),
    ]
    candidate_names = [
        'opencc_clib.pyd',
        'opencc_clib.so',
    ]
    for directory in candidate_dirs:
        if not os.path.isdir(directory):
            continue
        for entry in os.listdir(directory):
            if entry == 'opencc_clib.pyd' or entry.startswith('opencc_clib.') or entry.startswith('opencc_clib.cpython-'):
                candidate_names.append(entry)
    candidate_paths = []
    for directory in candidate_dirs:
        for name in candidate_names:
            candidate_paths.append(os.path.join(directory, name))
    for candidate in candidate_paths:
        if not os.path.isfile(candidate):
            continue
        spec = importlib.util.spec_from_file_location('opencc_clib', candidate)
        if spec is None or spec.loader is None:
            continue
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        return module
    raise ImportError('opencc_clib not found in Bazel runfiles')


try:
    import opencc_clib
except ImportError:
    try:
        from opencc.clib import opencc_clib
    except ImportError:
        opencc_clib = _load_opencc_clib_from_runfiles()


__version__ = opencc_clib.__version__


def _normalize_config_name(config: str) -> str:
    if config.endswith('.json'):
        return config
    if config in _CONFIG_STEMS:
        return config + '.json'
    return config


class OpenCC(opencc_clib._OpenCC):

    def __init__(self,
                 config: str = 't2s',
                 include_tofu_risk_dictionaries: bool = True,
                 resource_zip: Optional[str] = None) -> None:
        config = _normalize_config_name(config)
        if resource_zip is None and not os.path.isfile(config):
            config_under_share_dir = os.path.join(_opencc_share_dir, config)
            if os.path.isfile(config_under_share_dir):
                config = config_under_share_dir
        super().__init__(config, include_tofu_risk_dictionaries, resource_zip)
        self.config = config

    def convert(self, text: str):
        byte_text = text.encode('utf-8')
        return super().convert(byte_text, len(byte_text))
