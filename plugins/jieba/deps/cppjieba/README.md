# CppJieba

[![CMake](https://github.com/yanyiwu/cppjieba/actions/workflows/cmake.yml/badge.svg)](https://github.com/yanyiwu/cppjieba/actions/workflows/cmake.yml)
[![Author](https://img.shields.io/badge/author-@yanyiwu-blue.svg?style=flat)](http://yanyiwu.com/) 
[![Platform](https://img.shields.io/badge/platform-Linux,macOS,Windows-green.svg?style=flat)](https://github.com/yanyiwu/cppjieba)
[![Performance](https://img.shields.io/badge/performance-excellent-brightgreen.svg?style=flat)](http://yanyiwu.com/work/2015/06/14/jieba-series-performance-test.html) 
[![Tag](https://img.shields.io/github/v/tag/yanyiwu/cppjieba.svg)](https://github.com/yanyiwu/cppjieba/releases)

## 简介

CppJieba是"结巴(Jieba)"中文分词的C++版本

### 主要特点

- 🚀 高性能：经过线上环境验证的稳定性和性能表现
- 📦 易集成：源代码以头文件形式提供 (`include/cppjieba/*.hpp`)，包含即可使用
- 🔍 多种分词模式：支持精确模式、全模式、搜索引擎模式等
- 📚 自定义词典：支持用户自定义词典，支持多词典路径（使用'|'或';'分隔）
- 💻 跨平台：支持 Linux、macOS、Windows 操作系统
- 🌈 UTF-8编码：原生支持 UTF-8 编码的中文处理

## 快速开始

### 环境要求

- C++ 编译器：
  - g++ (推荐 4.1 以上版本)
  - 或 clang++
- cmake (推荐 2.6 以上版本)

### 安装步骤

```sh
git clone https://github.com/yanyiwu/cppjieba.git
cd cppjieba
git submodule init
git submodule update
mkdir build
cd build
cmake ..
make

make test
```

## 使用示例

```
./demo
```

结果示例：

```
[demo] Cut With HMM
他/来到/了/网易/杭研/大厦
[demo] Cut Without HMM
他/来到/了/网易/杭/研/大厦
我来到北京清华大学
[demo] CutAll
我/来到/北京/清华/清华大学/华大/大学
小明硕士毕业于中国科学院计算所，后在日本京都大学深造
[demo] CutForSearch
小明/硕士/毕业/于/中国/科学/学院/科学院/中国科学院/计算/计算所/，/后/在/日本/京都/大学/日本京都大学/深造
[demo] Insert User Word
男默/女泪
男默女泪
[demo] CutForSearch Word With Offset
[{"word": "小明", "offset": 0}, {"word": "硕士", "offset": 6}, {"word": "毕业", "offset": 12}, {"word": "于", "offset": 18}, {"word": "中国", "offset": 21}, {"word": "科学", "offset": 27}, {"word": "学院", "offset": 30}, {"word": "科学院", "offset": 27}, {"word": "中国科学院", "offset": 21}, {"word": "计算", "offset": 36}, {"word": "计算所", "offset": 36}, {"word": "，", "offset": 45}, {"word": "后", "offset": 48}, {"word": "在", "offset": 51}, {"word": "日本", "offset": 54}, {"word": "京都", "offset": 60}, {"word": "大学", "offset": 66}, {"word": "日本京都大学", "offset": 54}, {"word": "深造", "offset": 72}]
[demo] Tagging
我是拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上CEO，走上人生巅峰。
[我:r, 是:v, 拖拉机:n, 学院:n, 手扶拖拉机:n, 专业:n, 的:uj, 。:x, 不用:v, 多久:m, ，:x, 我:r, 就:d, 会:v, 升职:v, 加薪:nr, ，:x, 当上:t, CEO:eng, ，:x, 走上:v, 人生:n, 巅峰:n, 。:x]
[demo] Keyword Extraction
我是拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上CEO，走上人生巅峰。
[{"word": "CEO", "offset": [93], "weight": 11.7392}, {"word": "升职", "offset": [72], "weight": 10.8562}, {"word": "加薪", "offset": [78], "weight": 10.6426}, {"word": "手扶拖拉机", "offset": [21], "weight": 10.0089}, {"word": "巅峰", "offset": [111], "weight": 9.49396}]
```

For more details, please see [demo](https://github.com/yanyiwu/cppjieba-demo).

### 分词结果示例

**MPSegment**

Output:
```
我来到北京清华大学
我/来到/北京/清华大学

他来到了网易杭研大厦
他/来到/了/网易/杭/研/大厦

小明硕士毕业于中国科学院计算所，后在日本京都大学深造
小/明/硕士/毕业/于/中国科学院/计算所/，/后/在/日本京都大学/深造

```

**HMMSegment**

```
我来到北京清华大学
我来/到/北京/清华大学

他来到了网易杭研大厦
他来/到/了/网易/杭/研大厦

小明硕士毕业于中国科学院计算所，后在日本京都大学深造
小明/硕士/毕业于/中国/科学院/计算所/，/后/在/日/本/京/都/大/学/深/造

```

**MixSegment**

```
我来到北京清华大学
我/来到/北京/清华大学

他来到了网易杭研大厦
他/来到/了/网易/杭研/大厦

小明硕士毕业于中国科学院计算所，后在日本京都大学深造
小明/硕士/毕业/于/中国科学院/计算所/，/后/在/日本京都大学/深造

```

**FullSegment**

```
我来到北京清华大学
我/来到/北京/清华/清华大学/华大/大学

他来到了网易杭研大厦
他/来到/了/网易/杭/研/大厦

小明硕士毕业于中国科学院计算所，后在日本京都大学深造
小/明/硕士/毕业/于/中国/中国科学院/科学/科学院/学院/计算/计算所/，/后/在/日本/日本京都大学/京都/京都大学/大学/深造

```

**QuerySegment**

```
我来到北京清华大学
我/来到/北京/清华/清华大学/华大/大学

他来到了网易杭研大厦
他/来到/了/网易/杭研/大厦

小明硕士毕业于中国科学院计算所，后在日本京都大学深造
小明/硕士/毕业/于/中国/中国科学院/科学/科学院/学院/计算所/，/后/在/中国/中国科学院/科学/科学院/学院/日本/日本京都大学/京都/京都大学/大学/深造

```

以上依次是MP,HMM,Mix三种方法的效果。  

可以看出效果最好的是Mix，也就是融合MP和HMM的切词算法。即可以准确切出词典已有的词，又可以切出像"杭研"这样的未登录词。

Full方法切出所有字典里的词语。

Query方法先使用Mix方法切词，对于切出来的较长的词再使用Full方法。

### 自定义用户词典

自定义词典示例请看`dict/user.dict.utf8`。

没有使用自定义用户词典时的结果:

```
令狐冲/是/云/计算/行业/的/专家
```

使用自定义用户词典时的结果:

```
令狐冲/是/云计算/行业/的/专家
```

### 关键词抽取

```
我是拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上CEO，走上人生巅峰。
["CEO:11.7392", "升职:10.8562", "加薪:10.6426", "手扶拖拉机:10.0089", "巅峰:9.49396"]
```

For more details, please see [demo](https://github.com/yanyiwu/cppjieba-demo).

### 词性标注

```
我是蓝翔技工拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上总经理，出任CEO，迎娶白富美，走上人生巅峰。
["我:r", "是:v", "拖拉机:n", "学院:n", "手扶拖拉机:n", "专业:n", "的:uj", "。:x", "不用:v", "多久:m", "，:x", "我:r", "就:d", "会:v", "升职:v", "加薪:nr", "，:x", "当上:t", "CEO:eng", "，:x", "走上:v", "人生:n", "巅峰:n", "。:x"]
```

For more details, please see [demo](https://github.com/yanyiwu/cppjieba-demo).

支持自定义词性。
比如在(`dict/user.dict.utf8`)增加一行

```
蓝翔 nz
```

结果如下：

```
["我:r", "是:v", "蓝翔:nz", "技工:n", "拖拉机:n", "学院:n", "手扶拖拉机:n", "专业:n", "的:uj", "。:x", "不用:v", "多久:m", "，:x", "我:r", "就:d", "会:v", "升职:v", "加薪:nr", "，:x", "当:t", "上:f", "总经理:n", "，:x", "出任:v", "CEO:eng", "，:x", "迎娶:v", "白富美:x", "，:x", "走上:v", "人生:n", "巅峰:n", "。:x"]
```

## 其它词典资料分享

+ [dict.367W.utf8] iLife(562193561 at qq.com)

## 生态系统

CppJieba 已经被广泛应用于各种编程语言的分词实现中：

- [GoJieba](https://github.com/yanyiwu/gojieba) - Go 语言版本
- [NodeJieba](https://github.com/yanyiwu/nodejieba) - Node.js 版本
- [CJieba](https://github.com/yanyiwu/cjieba) - C 语言版本
- [jiebaR](https://github.com/qinwf/jiebaR) - R 语言版本
- [exjieba](https://github.com/falood/exjieba) - Erlang 版本
- [jieba_rb](https://github.com/altkatz/jieba_rb) - Ruby 版本
- [iosjieba](https://github.com/yanyiwu/iosjieba) - iOS 版本
- [phpjieba](https://github.com/jonnywang/phpjieba) - PHP 版本
- [perl5-jieba](https://metacpan.org/pod/distribution/Lingua-ZH-Jieba/lib/Lingua/ZH/Jieba.pod) - Perl 版本

### 应用项目

- [simhash](https://github.com/yanyiwu/simhash) - 中文文档相似度计算
- [pg_jieba](https://github.com/jaiminpan/pg_jieba) - PostgreSQL 分词插件
- [gitbook-plugin-search-pro](https://plugins.gitbook.com/plugin/search-pro) - Gitbook 中文搜索插件
- [ngx_http_cppjieba_module](https://github.com/yanyiwu/ngx_http_cppjieba_module) - Nginx 分词插件

## 贡献指南

我们欢迎各种形式的贡献，包括但不限于：

- 提交问题和建议
- 改进文档
- 提交代码修复
- 添加新功能


如果您觉得 CppJieba 对您有帮助，欢迎 star ⭐️ 支持项目！


