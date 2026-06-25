import os
from typing import Optional

try:
    import opencc_clib
except ImportError:
    from opencc.clib import opencc_clib

__all__ = ['CONFIGS', 'OpenCC', '__version__']

__version__ = opencc_clib.__version__
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
        value += f':{path}'
    os.environ[name] = value


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
