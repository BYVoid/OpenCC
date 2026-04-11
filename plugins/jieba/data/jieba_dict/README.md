# Jieba 分词词典

此目录包含 Jieba 中文分词所需的词典文件，来源于 [libcppjieba](https://github.com/yanyiwu/libcppjieba)。

## 文件说明

- **jieba.dict.utf8** (4.9 MB) - 主词典文件，包含词语及其词频
- **hmm_model.utf8** (508 KB) - 隐马尔可夫模型（HMM）文件，用于识别未登录词
- **user.dict.utf8** (33 B) - 用户自定义词典（可选）

## 许可证

这些词典文件继承自 jieba 项目，遵循 MIT 许可证。

## 使用方式

在 OpenCC 配置文件中指定这些词典的路径。IDF 和停用词数据
会从 `deps/libcppjieba/dict/` 自动解析，无需复制到此目录：

```json
{
  "segmentation": {
    "type": "jieba",
    "dict_path": "jieba_dict/jieba.dict.utf8",
    "model_path": "jieba_dict/hmm_model.utf8",
    "user_dict_path": "jieba_dict/user.dict.utf8"
  }
}
```

## 自定义用户词典

您可以编辑 `user.dict.utf8` 添加自定义词语，格式为：

```
词语 词频 词性
```

例如：
```
云计算 5 n
机器学习 8 n
```

每行一个词语，词频和词性可选。
