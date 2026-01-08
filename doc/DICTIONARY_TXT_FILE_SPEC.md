## 词典注释语法与排序规则（拟议）

1. 注释行（comment line）、词典记录行（dictionary entry line）、空行（empty line）的定义

  - **注释行**：行首以 `#` 开始的整行
      - 注释行在词典解析阶段被忽略，不参与任何 key → value 映射生成
      - 注释行在文本结构层面被保留，用于排序与格式化
      - （不支持 # 之前有空格和不可见字符的语法）
  - **词典记录行**：以 tab 分开的 key / value pair (其中 value 可以有多个值对应，以空格分开) 
  - **空行**：不包含任何可见字符（visible characters）
  - 凡不属于「注释行」、「词典记录行」、「空行」三种定义之一的行，为不合法，解析器应视其为未定义行为（Undefined Behavior），或直接报错中止。
  - 格式不支持行内注释，即「key \t value # 注释」这样的内容，这样的注释会被认为是 value 的一部分

2. 注释块（comment block）

  - 「连续的注释行」（即中间不含非注释行）构成一个注释块
  - 注释块之间若存在一个或多个空行，则视为不同注释块

3. 文件级注释块（header / footer block）

    3.1 文件开头注释块（header block）

      - 从文件起始位置开始扫描，位于文件开头且在「第一个词典记录行前的最后一个空行」之前的所有注释块，均视为文件级注释
      - 文件开头注释块的特性：
          - 不会 attach 到任何词条
          - 在排序或格式化后始终保留在文件开头
          - 允许由多个注释块与空行组成（例如：多段 license、不同来源的说明文本）
    
    3.2 文件结尾注释块（footer block）
    
      - 位于最后一条词典记录之后的注释块
      - 对于文件结尾注释块：
          - 不 attach 到任何词条
          - 在排序或格式化后始终保留在文件结尾

4. 词条注释块

  - 注释块仅在满足以下全部条件时，会 attach 到其后的词条记录行：
      - 注释块后紧跟一条词典记录行
      - 注释块与该记录之间没有空行
      - 注释块不属于 header block
  - 当注释块 attach 到词条行时：
      - 注释块与该词典记录构成一个不可分割的排序单元
      - 排序、重排时始终随该记录一起移动

5. 游离注释块（floating block）

  - 不满足 attach 条件，且不属于 header / footer 的注释块，视为游离注释块
  - 游离注释块在排序与格式化阶段，向下确定其版面锚点（anchor），该锚点定义为其后出现的第一条词典记录行
  
  （该规则用于安全支持「通过注释禁用词条」的常见用法。锚点仅用于确定输出位置，不构成词条语义上的 attachment。）

6. 排序规则

  - 排序的最小单位为词典记录 + 其上的词条注释块
  - header block 固定在文件开头
  - footer block 固定在文件结尾
  - 仅对词典记录单元的 key （即 tab separated 第一列）进行排序
      - 排序必须为稳定排序（Stable Sort），即当 key 相同时，应保持其在原始文档中出现的相对顺序
  - 游离注释块在排序完成后，插入到其锚点（anchor）位置，即位于其后第一条词典记录所在记录单元之前；若不存在后续词典记录，则插入到所有记录单元之后、footer block 之前；多个游离注释块锚点相同时，保持其原始相对顺序

7. 空行的正规化 (Newline Normalization)

  - 在解析阶段，连续的多个空行应视为单个逻辑分隔符。
  - 在格式化输出阶段，应强制运行「最多保留一个连续空行」的规则。
  - 关键约束：
      - 任何 floating block 的前后必须至少输出一个空行，以防止其在下次解析时被误判为 attached block。
      - 任何 attached block 与其后的词典记录行之间，严禁输出空行。

## 以 context-free grammar 表示

一个合法的字典 txt 文件应符合如下 `DICTIONARY_FILE` 定义：

```
EMPTY_LINE      ::= NEWLINE

KEY             ::= <any_char_except_tab_newline>+
VALUE           ::= <any_char_except_space_tab_newline>+
VALUE_LIST      ::= VALUE (SPACE VALUE)*
ENTRY_LINE      ::= KEY TAB VALUE_LIST NEWLINE

COMMENT_LINE    ::= "#" <any_char_except_newline>* NEWLINE
COMMENT_BLOCK   ::= COMMENT_LINE+

LINE_UNIT       ::= COMMENT_BLOCK
                  | ENTRY_LINE
                  | EMPTY_LINE

DICTIONARY_FILE ::= LINE_UNIT*
```
