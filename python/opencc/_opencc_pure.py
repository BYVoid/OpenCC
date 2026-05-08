"""
Pure Python implementation of the OpenCC Chinese conversion algorithm.

Algorithm overview (mirrors the C++ reference implementation):

  Dictionary (.txt)
    Each line: ``key<TAB>value1 value2 ...``
    The first listed candidate is used as the conversion target.

  Trie
    Character-by-character prefix tree.  Built per dictionary file.
    ``prefix_match(text, pos)`` returns ``(matched_length, value)``
    for the *longest* match starting at ``text[pos]``.

  DictGroup  (_GroupMatcher)
    An ordered list of tries.  At each position the tries are tried in
    declaration order; the **first** trie that has any match (not the
    globally longest) wins.  This gives phrase dictionaries priority
    over character dictionaries, mirroring the C++ PrefixMatch logic.

  Conversion
    Scans the text left-to-right.  At each position the active
    group/dict matcher is consulted; the matched substring is replaced
    with the dictionary value.  On no match, exactly one character is
    passed through unchanged.

  MaxMatch Segmentation
    Same scan as Conversion, but instead of replacing tokens the
    matched keys are emitted as segment boundaries.  Unmatched
    characters accumulate in a buffer that is flushed as a single
    segment when the next match is found or the string ends.

  Conversion Chain
    Multiple conversion steps applied sequentially to each segment
    produced by the segmenter.  Each step uses an independent
    group/dict matcher.
"""

import json
import os
import importlib.util
from pathlib import Path

__all__ = ['OpenCC']

_this_dir = os.path.dirname(os.path.abspath(__file__))

# Bundled data directories (populated when the package is installed with
# ``pip install .`` and the data files are included as package_data).
_pkg_config_dir = os.path.join(_this_dir, 'config')
_pkg_dict_dir = os.path.join(_this_dir, 'dictionary')
_pkg_jieba_dict_dir = os.path.join(_this_dir, 'jieba_dict')


_MAX_PARENT_TRAVERSAL_DEPTH = 8


def _runfile_dir(relative_path: str) -> str:
    runfiles_root = os.environ.get('RUNFILES_DIR')
    workspace = os.environ.get('TEST_WORKSPACE', '_main')
    if not runfiles_root:
        return ''

    for candidate in (
            Path(runfiles_root) / workspace / relative_path,
            Path(runfiles_root) / relative_path):
        if candidate.is_dir():
            return str(candidate)
    return ''


def _find_repo_root() -> str:
    """
    Walk up the directory tree to find the OpenCC repo root.

    Detects the repo by the presence of ``data/config/`` and
    ``data/dictionary/`` subdirectories.  Returns an empty string when the
    repo root cannot be determined (e.g. in a standalone installed package
    without bundled data files).
    """
    candidate = _this_dir
    for _ in range(_MAX_PARENT_TRAVERSAL_DEPTH):
        if (os.path.isdir(os.path.join(candidate, 'data', 'config')) and
                os.path.isdir(os.path.join(candidate, 'data', 'dictionary'))):
            return candidate
        parent = os.path.dirname(candidate)
        if parent == candidate:
            break
        candidate = parent
    return ''


_repo_root = _find_repo_root()
_repo_config_dir = os.path.join(_repo_root, 'data', 'config') if _repo_root else ''
_repo_dict_dir = os.path.join(_repo_root, 'data', 'dictionary') if _repo_root else ''
_repo_jieba_config_dir = os.path.join(_repo_root, 'plugins', 'jieba', 'data', 'config') if _repo_root else ''
_repo_jieba_dict_dir = os.path.join(_repo_root, 'plugins', 'jieba', 'deps', 'cppjieba', 'dict') if _repo_root else ''
_runfiles_config_dir = _runfile_dir('data/config')
_runfiles_dict_dir = _runfile_dir('data/dictionary')
_runfiles_jieba_config_dir = _runfile_dir('plugins/jieba/data/config')
_runfiles_jieba_dict_dir = _runfile_dir('plugins/jieba/deps/cppjieba/dict')


# Trie

class _TrieNode:
    """Single node in the character trie."""

    __slots__ = ('children', 'value')

    def __init__(self):
        self.children = {}  # char -> _TrieNode
        self.value = None   # str or None; non-None marks a terminal node


class _Trie:
    """
    Character-by-character trie for longest-prefix matching.

    Mirrors the inner ``Table`` class in ``PrefixMatch.cpp``.
    """

    __slots__ = ('root',)

    def __init__(self):
        self.root = _TrieNode()

    def add(self, key: str, value: str) -> None:
        """Insert ``key -> value`` into the trie."""
        node = self.root
        for ch in key:
            child = node.children.get(ch)
            if child is None:
                child = _TrieNode()
                node.children[ch] = child
            node = child
        node.value = value

    def prefix_match(self, text: str, pos: int):
        """
        Find the longest entry whose key is a prefix of ``text[pos:]``.

        Returns ``(matched_length, value)``; ``matched_length`` is 0 when
        no entry matches.
        """
        node = self.root
        matched_len = 0
        matched_value = None
        i = pos
        n = len(text)
        while i < n:
            child = node.children.get(text[i])
            if child is None:
                break
            node = child
            i += 1
            if node.value is not None:
                matched_len = i - pos
                matched_value = node.value
        return matched_len, matched_value


# DictGroup

class _GroupMatcher:
    """
    Ordered collection of tries.

    Mirrors the multi-table behaviour of ``PrefixMatch::MatchPrefix`` in the
    C++ implementation: tries are checked in declaration order; the **first**
    trie that has any prefix match returns its longest match for that trie.
    This gives phrase dictionaries priority over character dictionaries.
    """

    __slots__ = ('tries',)

    def __init__(self, tries):
        self.tries = list(tries)

    def match(self, text: str, pos: int):
        """
        Return ``(matched_length, value)`` using priority-ordered trie lookup.
        Returns ``(0, None)`` when no trie has a match.
        """
        for trie in self.tries:
            length, value = trie.prefix_match(text, pos)
            if length > 0:
                return length, value
        return 0, None


# Jieba segmentation

_jieba_tokenizer_cache: dict = {}


class _JiebaSegmenter:
    """
    Optional Jieba segmenter backed by the ``jieba`` Python package.

    The dependency is intentionally lazy: importing ``opencc`` and using the
    normal mmseg configs never imports or requires ``jieba``.
    """

    __slots__ = ('tokenizer',)

    def __init__(self, resources: dict, config_dir: str = ''):
        try:
            import jieba
        except ImportError as exc:
            raise ImportError(
                "Config uses Jieba segmentation. Install the optional "
                "dependency with `pip install jieba` to use *_jieba configs."
            ) from exc

        dict_path = _find_jieba_resource(
            resources.get('dict_path', ''),
            config_dir,
            'jieba.dict.utf8',
        )
        user_dict_path = _find_jieba_resource(
            resources.get('user_dict_path', ''),
            config_dir,
            'user.dict.utf8',
            required=False,
        )

        cache_key = (dict_path, user_dict_path)
        tokenizer = _jieba_tokenizer_cache.get(cache_key)
        if tokenizer is None:
            tokenizer = jieba.Tokenizer(dictionary=dict_path)
            tokenizer.initialize()
            if user_dict_path:
                tokenizer.load_userdict(user_dict_path)
            _jieba_tokenizer_cache[cache_key] = tokenizer
        self.tokenizer = tokenizer

    def segment(self, text: str) -> list:
        return list(self.tokenizer.cut(text, HMM=True))


def _jieba_available() -> bool:
    return importlib.util.find_spec('jieba') is not None


def _find_jieba_resource(raw_path: str,
                         config_dir: str,
                         fallback_name: str,
                         required: bool = True) -> str:
    """
    Resolve Jieba resources for the pure Python optional backend.

    Plugin configs point at ``jieba_merged.ocd2`` for the C++ plugin.  Python
    ``jieba`` consumes cppjieba's text dictionaries directly, so that request
    is mapped to ``jieba.dict.utf8`` plus ``user.dict.utf8``.
    """
    candidates = []

    if raw_path:
        if raw_path.endswith('jieba_merged.ocd2'):
            candidates.append(raw_path[:-len('jieba_merged.ocd2')] + fallback_name)
            candidates.append(fallback_name)
        else:
            candidates.append(raw_path)
            candidates.append(os.path.join(os.path.dirname(raw_path), fallback_name))
            candidates.append(fallback_name)
    else:
        candidates.append(fallback_name)

    search_dirs = (
        '',
        config_dir,
        os.path.dirname(config_dir) if config_dir else '',
        _pkg_jieba_dict_dir,
        _runfiles_jieba_dict_dir,
        _repo_jieba_dict_dir,
    )

    seen = set()
    for candidate in candidates:
        for search_dir in search_dirs:
            if not candidate:
                continue
            path = candidate if os.path.isabs(candidate) or not search_dir else os.path.join(search_dir, candidate)
            norm = os.path.abspath(path)
            if norm in seen:
                continue
            seen.add(norm)
            if os.path.isfile(path):
                return path

    if required:
        raise FileNotFoundError(f'Jieba resource not found: {fallback_name!r}')
    return ''


# Dictionary loading

_trie_cache: dict = {}  # (absolute_path, reversed: bool) -> _Trie


def _find_dict_txt(filename: str, config_dir: str = ''):
    """
    Resolve a dictionary filename (typically ``Foo.ocd2``) to a
    ``(txt_path, needs_reverse)`` tuple.

    Resolution:
    1. Strip the extension (``Foo.ocd2`` → ``Foo``).
    2. Look for ``<stem>.txt`` next to a custom config, in the bundled pkg
       dir, then in Bazel runfiles and the repo dir.
    3. If the stem ends with ``Rev`` (e.g. ``HKVariantsRev``), look for
       the forward dict (``HKVariants.txt``) and mark it for reversal.
       These reversed dicts are generated on-the-fly from the forward file,
       matching the behaviour of the ``reverse_items`` build script.

    Raises ``FileNotFoundError`` if no suitable txt file is found.
    """
    stem = filename
    for ext in ('.ocd2', '.ocd', '.txt'):
        if stem.endswith(ext):
            stem = stem[: -len(ext)]
            break

    if os.path.isabs(filename):
        path = stem + '.txt'
        if os.path.isfile(path):
            return path, False

    search_dirs = (
        config_dir,
        _pkg_dict_dir,
        _runfiles_dict_dir,
        _repo_dict_dir,
    )

    for search_dir in search_dirs:
        if not search_dir:
            continue
        path = os.path.join(search_dir, stem + '.txt')
        if os.path.isfile(path):
            return path, False

    # Try reversed dict: strip the trailing "Rev" and reverse the forward file.
    if stem.endswith('Rev'):
        forward_stem = stem[:-3]
        for search_dir in search_dirs:
            if not search_dir:
                continue
            path = os.path.join(search_dir, forward_stem + '.txt')
            if os.path.isfile(path):
                return path, True

    raise FileNotFoundError(f'Dictionary file not found for: {filename!r}')


def _parse_dict_lines(path: str):
    """
    Yield ``(key, candidates)`` tuples for every data line in a txt dict file.

    Comment lines (starting with ``#``) and blank lines are skipped.
    ``candidates`` is a list of one or more whitespace-separated values.
    """
    with open(path, encoding='utf-8-sig') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or '\t' not in line:
                continue
            key, rest = line.split('\t', 1)
            candidates = rest.split()
            if candidates:
                yield key, candidates


def _load_trie(path: str, reverse: bool) -> _Trie:
    """
    Load a ``key<TAB>value1 value2 ...`` txt file into a :class:`_Trie`.

    When ``reverse`` is ``True`` the mapping is inverted: each candidate
    value becomes a key whose conversion target is the original key.  When
    multiple original keys share the same candidate value the first
    encountered (in file order) wins, matching the behaviour of the C++
    dict-reversal script.
    """
    cache_key = (path, reverse)
    cached = _trie_cache.get(cache_key)
    if cached is not None:
        return cached

    trie = _Trie()

    if not reverse:
        for key, candidates in _parse_dict_lines(path):
            trie.add(key, candidates[0])
    else:
        # Reverse: each candidate value -> original key (first writer wins).
        reversed_mapping: dict = {}
        for key, candidates in _parse_dict_lines(path):
            for candidate in candidates:
                if candidate not in reversed_mapping:
                    reversed_mapping[candidate] = key
        for new_key, new_value in reversed_mapping.items():
            trie.add(new_key, new_value)

    _trie_cache[cache_key] = trie
    return trie


def _get_trie(filename: str, config_dir: str = '') -> _Trie:
    """Resolve ``filename`` to a txt path and return the loaded trie."""
    path, needs_reverse = _find_dict_txt(filename, config_dir)
    return _load_trie(path, needs_reverse)


# Config parsing

def _parse_dict_node(d: dict, config_dir: str = '') -> _GroupMatcher:
    """
    Recursively parse a JSON dict-config node and return a :class:`_GroupMatcher`.

    Mirrors ``ConfigInternal::ParseDict`` in ``Config.cpp``.  Groups are
    flattened into an ordered list of leaf tries, preserving the priority
    order used by ``PrefixMatch::AddDict``.
    """
    dtype = d['type']

    if dtype == 'group':
        tries = []
        for sub in d['dicts']:
            sub_matcher = _parse_dict_node(sub, config_dir)
            # Flatten: extend with the sub-group's ordered tries list.
            tries.extend(sub_matcher.tries)
        return _GroupMatcher(tries)

    if dtype in ('ocd2', 'ocd', 'txt', 'text'):
        trie = _get_trie(d['file'], config_dir)
        return _GroupMatcher([trie])

    raise ValueError(f'Unknown dict type: {dtype!r}')


# Core conversion routines

def _convert_phrase(text: str, matcher: _GroupMatcher) -> str:
    """
    Convert *text* using left-to-right longest-prefix matching.

    At each position the group matcher is consulted (first-trie-wins
    semantics).  On a match the matched substring is replaced by the
    dictionary value.  On no match exactly one character is passed through
    unchanged, mirroring ``Conversion::AppendConverted`` in the C++ code.
    """
    parts = []
    i = 0
    n = len(text)
    while i < n:
        length, value = matcher.match(text, i)
        if length > 0:
            parts.append(value)
            i += length
        else:
            parts.append(text[i])
            i += 1
    return ''.join(parts)


def _segment(text: str, matcher: _GroupMatcher) -> list:
    """
    Max-match segmentation of *text*.

    Scans left-to-right using the segmentation dict.  On a match the
    pending character buffer is flushed as one segment and the matched key
    becomes the next segment.  Unmatched characters accumulate until the
    next match or end of string.

    Mirrors ``MaxMatchSegmentation::Segment`` in the C++ code.
    """
    segments = []
    buf = []
    i = 0
    n = len(text)
    while i < n:
        length, _ = matcher.match(text, i)
        if length > 0:
            if buf:
                segments.append(''.join(buf))
                buf = []
            segments.append(text[i: i + length])
            i += length
        else:
            buf.append(text[i])
            i += 1
    if buf:
        segments.append(''.join(buf))
    return segments


# Config location helpers

def _find_config(config_name: str) -> str:
    """
    Locate a config JSON file by name (with or without ``.json`` suffix).

    Searches the bundled package config directory first, then the source-tree
    ``data/config`` directory as a fallback.
    """
    filename = config_name if config_name.endswith('.json') else config_name + '.json'
    for search_dir in (
            _pkg_config_dir,
            _runfiles_config_dir,
            _repo_config_dir,
            _runfiles_jieba_config_dir,
            _repo_jieba_config_dir):
        if not search_dir:
            continue
        path = os.path.join(search_dir, filename)
        if os.path.isfile(path):
            return path
    raise FileNotFoundError(f'Config not found: {config_name!r}')


def _is_mmseg_config(config_path: str) -> bool:
    """Return True if the config uses mmseg segmentation (pure-Python compatible)."""
    try:
        with open(config_path, encoding='utf-8') as f:
            cfg = json.load(f)
        return cfg.get('segmentation', {}).get('type') == 'mmseg'
    except Exception:
        return False


def _is_supported_config(config_path: str) -> bool:
    """Return True if the pure Python backend can use this config now."""
    try:
        with open(config_path, encoding='utf-8') as f:
            cfg = json.load(f)
        seg_type = cfg.get('segmentation', {}).get('type')
        if seg_type == 'mmseg':
            return True
        if seg_type == 'jieba':
            if not _jieba_available():
                return False
            resources = cfg.get('segmentation', {}).get('resources', {})
            config_dir = os.path.dirname(config_path)
            _find_jieba_resource(resources.get('dict_path', ''), config_dir, 'jieba.dict.utf8')
            return True
        return False
    except Exception:
        return False


def list_configs() -> list:
    """
    Return a sorted list of ``.json`` config filenames available to the
    pure Python converter.  Jieba configs are listed only when the optional
    ``jieba`` package and dictionary data are available.
    """
    configs = []
    seen = set()
    for search_dir in (
            _pkg_config_dir,
            _runfiles_config_dir,
            _repo_config_dir,
            _runfiles_jieba_config_dir,
            _repo_jieba_config_dir):
        if not search_dir or not os.path.isdir(search_dir):
            continue
        for fname in os.listdir(search_dir):
            if not fname.endswith('.json') or fname in seen:
                continue
            path = os.path.join(search_dir, fname)
            if _is_supported_config(path):
                configs.append(fname)
                seen.add(fname)
    return sorted(configs)


# Public API

class OpenCC:
    """
    Pure Python OpenCC converter.

    Usage::

        cc = OpenCC('s2t')          # or 's2t.json'
        result = cc.convert('汉字')  # -> '漢字'

    The ``config`` argument accepts a conversion name (e.g. ``'s2t'``) or
    a bare filename (e.g. ``'s2t.json'``).  Custom absolute config paths
    are also accepted.
    """

    def __init__(self, config: str = 't2s') -> None:
        config_name = config[:-5] if config.endswith('.json') else config

        # Allow custom config paths, matching the previous Python wrapper.
        if os.path.isfile(config):
            config_path = config
        else:
            config_path = _find_config(config_name)

        with open(config_path, encoding='utf-8') as f:
            cfg = json.load(f)

        self.config = config if config.endswith('.json') else f'{config}.json'

        config_dir = os.path.dirname(config_path)
        segmentation = cfg.get('segmentation', {})
        seg_type = segmentation.get('type', '')
        if seg_type == 'mmseg':
            self._segmenter = _parse_dict_node(segmentation['dict'], config_dir)
        elif seg_type == 'jieba':
            self._segmenter = _JiebaSegmenter(
                segmentation.get('resources', {}),
                config_dir,
            )
        else:
            raise ValueError(
                f"Config {config_name!r} uses segmentation type {seg_type!r} "
                f"which is not supported by the pure Python backend."
            )

        self._chain: list = [
            _parse_dict_node(step['dict'], config_dir)
            for step in cfg['conversion_chain']
        ]

    def convert(self, text: str) -> str:
        """Convert *text* and return the result."""
        if isinstance(self._segmenter, _JiebaSegmenter):
            segments = self._segmenter.segment(text)
        else:
            segments = _segment(text, self._segmenter)
        result = []
        for seg in segments:
            converted = seg
            for matcher in self._chain:
                converted = _convert_phrase(converted, matcher)
            result.append(converted)
        return ''.join(result)
