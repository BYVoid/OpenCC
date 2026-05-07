from importlib import metadata

from opencc._opencc_pure import OpenCC
from opencc._opencc_pure import list_configs

__all__ = ['CONFIGS', 'OpenCC', '__version__']

try:
    __version__ = metadata.version('OpenCC')
except metadata.PackageNotFoundError:
    __version__ = 'dev'

CONFIGS = list_configs()
