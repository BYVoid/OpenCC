# PyOpenCC-pure

A pure python based on Open Chinese Convert. You can use it without opencc binary. The result could differ a bit, the performance is also not expect to be the same.

## how to use

`pip install pyopencc_pure-0.0.1-py3-none-any.whl`

### command-line

The config file is need, with json format. The input text is converted and result is print out.
`python -m  pyopencc_pure --config s2t.json  简体中文`

If the input and output file are provided, the converted result is not print out.

`python -m  pyopencc_pure --config s2tw.json --fine zhCN.txt --out zhTW.txt`

### use package

```
>>> import pyopencc_pure`
>>> config = pyopencc_pure.load_config('s2t.json')
>>> pyopencc_pure.convert(config, '简体中文')
'簡體中文'
```
## Build

requirements: `pip install wheel`

`python setup.py sdist bdist_wheel`
