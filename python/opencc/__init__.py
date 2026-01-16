import os

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


def _append_path_to_env(name: str, path: str) -> None:
    value = os.environ.get(name, '')
    if path in value:  # Path already exists
        return
    if value == '':
        value = path
    else:
        value += f':{path}'
    os.environ[name] = value


class OpenCC(opencc_clib._OpenCC):

    def __init__(self, config: str = 't2s') -> None:
        if not config.endswith('.json'):
            config += '.json'
        if not os.path.isfile(config):
            config_under_share_dir = os.path.join(_opencc_share_dir, config)
            if os.path.isfile(config_under_share_dir):
                config = config_under_share_dir
        super().__init__(config)
        self.config = config

    def convert(self, text: str):
        byte_text = text.encode('utf-8')
        return super().convert(byte_text, len(byte_text))
