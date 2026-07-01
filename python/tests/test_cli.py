import os
import pathlib
import pytest
import subprocess
import sys
import zipfile

import opencc.cli

_TEXT_ZIP_CONFIG = """{
  "name": "Python CLI Resource Zip Test",
  "segmentation": {
    "type": "mmseg",
    "dict": {
      "type": "text",
      "file": "STPhrases.txt"
    }
  },
  "conversion_chain": [
    {
      "dict": {
        "type": "group",
        "dicts": [
          {
            "type": "text",
            "file": "STPhrases.txt"
          },
          {
            "type": "text",
            "file": "STCharacters.txt"
          }
        ]
      }
    },
    {
      "dict": {
        "type": "text",
        "file": "TWPhrases.txt"
      }
    }
  ]
}
"""


def _write_resource_zip(path: pathlib.Path) -> None:
    with zipfile.ZipFile(path, 'w', compression=zipfile.ZIP_STORED) as archive:
        archive.writestr('s2twp.json', _TEXT_ZIP_CONFIG)
        archive.writestr('STPhrases.txt', "打印机\t印表機\n")
        archive.writestr('STCharacters.txt', "鼠\t鼠\n标\t標\n")
        archive.writestr('TWPhrases.txt', "鼠標\t滑鼠\n")


def _write_custom_config(config_path: pathlib.Path, dict_path: pathlib.Path) -> None:
    config_path.write_text(
        '{\n'
        '  "name": "Python CLI Custom Config Test",\n'
        '  "segmentation": {\n'
        '    "type": "mmseg",\n'
        '    "dict": {\n'
        '      "type": "text",\n'
        f'      "file": "{dict_path.name}"\n'
        '    }\n'
        '  },\n'
        '  "conversion_chain": [\n'
        '    {\n'
        '      "dict": {\n'
        '        "type": "text",\n'
        f'        "file": "{dict_path.name}"\n'
        '      }\n'
        '    }\n'
        '  ]\n'
        '}\n',
        encoding='utf-8',
    )
    dict_path.write_text('鼠标\t滑鼠\n', encoding='utf-8')


def _run_cli(monkeypatch, *args):
    monkeypatch.setattr(sys, 'argv', ['opencc', *args])
    opencc.cli.main()


def _subprocess_env():
    env = os.environ.copy()
    pythonpath_entries = [p for p in sys.path if p]
    if env.get('PYTHONPATH'):
        pythonpath_entries.append(env['PYTHONPATH'])
    env['PYTHONPATH'] = os.pathsep.join(pythonpath_entries)
    return env


def test_cli_skips_tofu_risk_dictionaries_by_default(tmp_path, monkeypatch):
    input_path = tmp_path / 'input.txt'
    output_path = tmp_path / 'output.txt'
    input_path.write_text('㑮', encoding='utf-8')

    _run_cli(
        monkeypatch,
        '-c', 't2s.json',
        '-i', str(input_path),
        '-o', str(output_path),
    )

    assert output_path.read_text(encoding='utf-8') == '㑮'


def test_cli_include_tofu_risk_dictionaries_flag(tmp_path, monkeypatch):
    input_path = tmp_path / 'input.txt'
    output_path = tmp_path / 'output.txt'
    input_path.write_text('㑮', encoding='utf-8')

    _run_cli(
        monkeypatch,
        '-c', 't2s.json',
        '-i', str(input_path),
        '-o', str(output_path),
        '--include-tofu-risk-dictionaries',
    )

    assert output_path.read_text(encoding='utf-8') == '𫝈'


def test_cli_rejects_same_input_and_output_file(tmp_path, monkeypatch, capsys):
    input_path = tmp_path / 'same.txt'
    input_path.write_text('汉字', encoding='utf-8')

    with pytest.raises(SystemExit) as exc_info:
        _run_cli(
            monkeypatch,
            '-c', 's2t.json',
            '-i', str(input_path),
            '-o', str(pathlib.Path(input_path)),
        )

    captured = capsys.readouterr()
    assert exc_info.value.code == 1
    assert 'input and output refer to the same file' in captured.err


def test_cli_resource_zip_support(tmp_path, monkeypatch):
    input_path = tmp_path / 'input.txt'
    output_path = tmp_path / 'output.txt'
    resource_zip = tmp_path / 'opencc-resources.zip'
    input_path.write_text('打印机和鼠标', encoding='utf-8')
    _write_resource_zip(resource_zip)

    _run_cli(
        monkeypatch,
        '-c', 's2twp.json',
        '-i', str(input_path),
        '-o', str(output_path),
        '--resource-zip', str(resource_zip),
    )

    assert output_path.read_text(encoding='utf-8') == '印表機和滑鼠'


def test_cli_subprocess_integration(tmp_path):
    input_path = tmp_path / 'input.txt'
    output_path = tmp_path / 'output.txt'
    resource_zip = tmp_path / 'opencc-resources.zip'
    input_path.write_text('打印机和鼠标', encoding='utf-8')
    _write_resource_zip(resource_zip)

    result = subprocess.run(
        [
            sys.executable,
            '-m',
            'opencc.cli',
            '-c',
            's2twp.json',
            '-i',
            str(input_path),
            '-o',
            str(output_path),
            '--resource-zip',
            str(resource_zip),
        ],
        capture_output=True,
        text=True,
        env=_subprocess_env(),
        check=False,
    )

    assert result.returncode == 0, result.stderr
    assert output_path.read_text(encoding='utf-8') == '印表機和滑鼠'


def test_cli_builtin_config_stem_supports_omitting_json(tmp_path, monkeypatch):
    input_path = tmp_path / 'input.txt'
    output_path = tmp_path / 'output.txt'
    input_path.write_text('汉字', encoding='utf-8')

    _run_cli(
        monkeypatch,
        '-c', 's2t',
        '-i', str(input_path),
        '-o', str(output_path),
    )

    assert output_path.read_text(encoding='utf-8') == '漢字'


def test_cli_custom_config_path_without_json_extension(tmp_path, monkeypatch):
    input_path = tmp_path / 'input.txt'
    output_path = tmp_path / 'output.txt'
    config_path = tmp_path / 'custom-config'
    dict_path = tmp_path / 'custom-dict.txt'
    input_path.write_text('鼠标', encoding='utf-8')
    _write_custom_config(config_path, dict_path)

    _run_cli(
        monkeypatch,
        '-c', str(config_path),
        '-i', str(input_path),
        '-o', str(output_path),
    )

    assert output_path.read_text(encoding='utf-8') == '滑鼠'
