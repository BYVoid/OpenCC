# CppJieba字典

文件后缀名代表的是词典的编码方式。
比如filename.utf8 是 utf8编码，filename.gbk 是 gbk编码方式。


## 分词

### jieba.dict.utf8/gbk

作为最大概率法(MPSegment: Max Probability)分词所使用的词典。

格式固定为:

```text
词语 词频 词性
```

- 每行必须正好三列。
- `词频` 读取后会换算成对数权重。
- `词性` 以字符串形式保存在词典节点中，库本身不做枚举校验。

### hmm_model.utf8/gbk

作为隐式马尔科夫模型(HMMSegment: Hidden Markov Model)分词所使用的词典。

__对于MixSegment(混合MPSegment和HMMSegment两者)则同时使用以上两个词典__

### user.dict.utf8

用户词典示例。

支持以下三种行格式:

```text
词语
词语 词性
词语 词频 词性
```

- `1` 列表示只指定词语，使用默认权重，词性为空。
- `2` 列表示 `词语 词性`。
- `3` 列表示 `词语 词频 词性`。
- 可以通过 `|` 或 `;` 传入多个用户词典文件路径。
- 不支持注释语法、额外列或带空格的词语字段。

当前默认权重来自主词典统计值，默认使用中位数权重；也可以在
`cppjieba::DictTrie` 构造时通过 `WordWeightMin`、`WordWeightMedian`、
`WordWeightMax` 调整策略。


## 关键词抽取

### idf.utf8

IDF(Inverse Document Frequency)
在KeywordExtractor中，使用的是经典的TF-IDF算法，所以需要这么一个词典提供IDF信息。

### stop_words.utf8

停用词词典

