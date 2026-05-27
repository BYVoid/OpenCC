# CHANGELOG

## v5.5.0

+ feat: add Windows ARM64 build support
+ build: upgrade googletest from 1.11.0 to 1.12.1
+ build: update CMake minimum version requirement to 3.10
+ fix: make namespaces explicit and fix missing includes
+ ci: update stale-issues workflow configuration

## v5.4.0

+ unittest: class Jiaba add default argument input
+ class Jieba: support default dictpath
+ cmake: avoid testing when FetchContent by other project
+ class DictTrie: removed unused var

## v5.3.2

+ removed test/demo.cpp and linked https://github.com/yanyiwu/cppjieba-demo
+ Update Demo Link in README.md
+ [github/actions] stale 1 year ago issues
+ limonp v0.9.0 -> v1.0.0

## v5.3.1

+ [cmake] fetch googletest
+ [submodules] rm test/googletest

## v5.3.0

+ [c++17,c++20] compatibility
+ limonp version 0.6.7 -> 0.9.0

## v5.2.0

+ [CI] windows-[2019,2022]
+ [googletest] v1.6.0->v1.10.0
+ [CI] ubuntu version from 20 to 22, macos version from 12 to 14
+ [CMake] mini_required 2.6->3.5 and fix CXX_VERSION variable passed from cmd
+ [CI] matrix and multi cpp version [11, 14]

## v5.1.3

+ [googletest] git submodule add googletest-1.6.0

## v5.1.2

+ [submodule:deps/limonp] upgrade to v0.6.7

## v5.1.1

+ Merged [pr-186](https://github.com/yanyiwu/cppjieba/pull/186)

## v5.1.0

+ Merged [feature: add RemoveWord api from gojieba/pull/99 #172](https://github.com/yanyiwu/cppjieba/pull/172)

## v5.0.5

+ Merged [pr-171 submodule update limonp to v0.6.6 #171](https://github.com/yanyiwu/cppjieba/pull/171)

## v5.0.4

+ Merged [pr-168 limonp as submodule #168](https://github.com/yanyiwu/cppjieba/pull/168)

## v5.0.3

+ Upgrade [limonp](https://github.com/yanyiwu/limonp) -> v0.6.3

## v5.0.2

+ Upgrade [limonp](https://github.com/yanyiwu/limonp) -> v0.6.1

## v5.0.1

+ Make Compiler Happier.
+ Add PHP, DLang Links.

## v5.0.0

+ Notice(**api changed**) : Jieba class 3 arguments -> 5 arguments, and use KeywordExtractor in Jieba

## v4.8.1

+ add TextRankExtractor by [@questionfish] in [pull request 65](https://github.com/yanyiwu/cppjieba/pull/65)
+ add Jieba::ResetSeparators api for some special situation, for example in [issue67](https://github.com/yanyiwu/cppjieba/issues/67)
+ fix [issue70](https://github.com/yanyiwu/cppjieba/issues/70)
+ support (word, freq, tag) format in user_dict, see details in [pr74](https://github.com/yanyiwu/cppjieba/pull/74)

## v4.8.0

+ rewrite QuerySegment, make `Jieba::CutForSearch` behaves the same as [jieba] `cut_for_search` api
+ remove Jieba::SetQuerySegmentThreshold

## v4.7.0

api changes:

+ override Cut functions, add location information into Word results;
+ remove LevelSegment;
+ remove Jieba::Locate;

upgrade:

+ limonp -> v0.6.1

## v4.6.0

+ Change Jieba::Locate(deprecated) to be static function.
+ Change the return value of KeywordExtractor::Extract from bool to void.
+ Add KeywordExtractor::Word and add more overrided KeywordExtractor::Extract

## v4.5.3

+ Upgrade limonp to v0.6.0

## v4.5.2

+ Upgrade limonp to v0.5.6 to fix hidden trouble.

## v4.5.1

+ Upgrade limonp to v0.5.5 to solve macro name conficts in some special case. 

## v4.5.0

+ 在 Trie 中去除之前糟糕的针对 uint16 优化的用数组代替 map 的设计，
该设计的主要问题是前提 unicode 每个字符必须是 uint16 ，则无法更全面得支持 unicode 多国字符。 
+ Rune 类型从 16bit 更改为 32bit ，支持更多 Unicode 字符，包括一些罕见汉字。

## v4.4.1

+ 使用 valgrind 检查内存泄露的问题，定位出一个HMM模型初始化的问题导致内存泄露的bug，不过此内存泄露不是致命问题，
因为只会在词典载入的时候发生，而词典载入通常情况下只会被运行一次，故不会导致严重问题。
+ 感谢 [qinwf] 帮我发现这个bug，非常感谢。

## v4.4.0

+ 加代码容易删代码难，思索良久，还是决定把 Server 功能的源码剥离出这个项目。
+ 让 [cppjieba] 回到当年情窦未开时清纯的感觉，删除那些无关紧要的server代码，让整个项目轻装上阵，专注分词的核心代码。
+ By the way, 之前的 server 相关的代码，如果你真的需要它，就去新的项目仓库 [cppjieba-server](https://github.com/yanyiwu/cppjieba-server) 找它们。

## v4.3.3

+ Yet Another Incompatibility Problem Repair: Upgrade [limonp] to version v0.5.3, fix incompatibility problem in Windows

## v4.3.2

+ Upgrade [limonp] to version v0.5.2, fix incompatibility problem in Windows

## v4.3.1

+ 重载 KeywordExtractor 的构造函数，可以传入 Jieba 进行字典和模型的构造。 

## v4.3.0

源码目录布局调整：

1. src/ -> include/cppjieba/
2. src/limonp/ -> deps/limonp/
3. server/husky -> deps/husky/
4. test/unittest/gtest -> deps/gtest

依赖库升级：

1. [limonp] to version v0.5.1
2. [husky] to version v0.2.0

## v4.2.1

1. Upgrade [limonp] to version v0.4.1, [husky] to version v0.2.0

## v4.2.0

1. 修复[issue50]提到的多词典分隔符在Windows环境下存在的问题，从':'修改成'|'或';'。

## v4.1.2

1. 新增 Jieba::Locate 函数接口，作为计算分词结果的词语位置信息，在某些场景下有用，比如搜索结果高亮之类的。

## v4.1.1

1. 在 class Jieba 中新增词性标注的接口函数 Jieba::Tag

## v4.1.0

1. QuerySegment切词时加一层判断，当长词满足IsAllAscii(比如英文单词)时，不进行细粒度分词。
2. QuerySegment新增SetMaxWordLen和GetMaxWordLen接口，用来设置二次分词条件被触发的词长阈值。
3. Jieba新增SetQuerySegmentThreshold设置CutForSearch函数的词长阈值。

## v4.0.0

1. 支持多个userdict载入，多词典路径用英文冒号(:)作为分隔符，就当是向环境变量PATH致敬，哈哈。
2. userdict是不带权重的，之前对于新的userword默认设置词频权重为最大值，现已支持可配置，默认使用中位值。
3. 【兼容性预警】修改一些代码风格，比如命名空间小写化，从CppJieba变成cppjieba。
4. 【兼容性预警】弃用Application.hpp, 取而代之使用Jieba.hpp ，接口也进行了大幅修改，函数风格更统一，和python版本的Jieba分词更一致。

## v3.2.1

1. 修复 Jieba.hpp 头文件保护写错导致的 bug。

## v3.2.0

1. 使用工程上比较 tricky 的 Trie树优化办法。废弃了之前的 `Aho-Corasick-Automation` 实现，可读性更好，性能更高。
2. 新增层次分词器: LevelSegment 。
3. 增加MPSegment的细粒度分词功能。
4. 增加 class Jieba ，提供可读性更好的接口。
5. 放弃了统一接口ISegment，因为统一的接口限制了分词方式的灵活性，限制了一些功能的增加。
6. 增加默认开启新词发现功能的可选参数hmm，让MixSegment和QuerySegment都支持开关新词发现功能。

## v3.1.0

1. 新增可动态增加词典的API: insertUserWord
2. cut函数增加默认参数，默认使用Mix切词算法。关于切词算法详见README.md

## v3.0.1

1. 提升兼容性，修复在某些特定环境下的编译错误问题。 

## v3.0.0

1. 使得 QuerySegment 支持自定义词典（可选参数）。
2. 使得 KeywordExtractor 支持自定义词典（可选参数）。
3. 修改 Code Style ，参照 google code style 。 
4. 增加更详细的错误日志，在初始化过程中合理使用LogFatal。
5. 增加 Application 这个类，整合了所有CppJieba的功能进去，以后用户只需要使用这个类即可。
6. 修改 cjserver 服务，可以通过http参数使用不同切词算法进行切词。
7. 修改 make install 的安装目录，统一安装到同一个目录 /usr/local/cppjieba 。

## v2.4.4

1. 修改两条更细粒度的特殊过滤规则，将连续的数字（包括浮点数）和连续的字母单独切分出来（而不会混在一起）。
2. 修改最大概率法时动态规划过程需要使用的 DAG 数据结构（同时也修改 Trie 的 DAG 查询函数），提高分词速度 8% 。
3. 使用了 `Aho-Corasick-Automation` 算法提速 Trie 查找的过程等优化，提升性能。
4. 增加词性标注的两条特殊规则。

## v2.4.3

1. 更新 [husky] 服务代码，新 [husky] 为基于线程池的服务器简易框架。并且修复当 HTTP POST 请求时 body 过长数据可能丢失的问题。
2. 修改 PosTagger 的参数结构，删除暂时无用的参数。并添加使用自定义字典的参数，也就是支持 **自定义词性**。
3. 更好的支持 `mac osx` (原谅作者如此屌丝，这么晚才买 `mac` )。
4. 支持 `Docker` ，具体请见 `Dockerfile` 。

## v2.4.2

1. 适当使用 `vector`， 的基础上，使用`limonp/LocalVector.hpp`作为`Unicode`的类型等优化，约提高性能 `30%`。
2. 使 `cjserver` 支持用户自定义词典，通过在 `conf/server.conf` 里面配置 `user_dict_path` 来实现。
3. 修复 `MPSegment` 切词时，当句子中含有特殊字符时，切词结果不完整的问题。
4. 修改 `FullSegment` 减少内存使用。 
5. 修改 `-std=c++0x` 或者 `-std=c++11` 时编译失败的问题。

## v2.4.1

1. 完善一些特殊字符和字母串的切词效果。
2. 提高关键词抽取的速度。
3. 提供用户自定义词典的接口。
4. 将server相关的代码独立出来，单独放在`server/`目录下。
5. 修复用户自定义词典中单字会被MixSegment的新词发现功能给忽略的问题。也就是说，现在的词典是用户词典优先级最高，其次是自带的词典，再其次是新词发现出来的词。

## v2.4.0

1. 适配更低级版本的`g++`和`cmake`，已在`g++ 4.1.2`和`cmake 2.6`上测试通过。
2. 修改一些测试用例的文件，减少测试时编译的时间。
3. 修复`make install`相关的问题。
4. 增加HTTP服务的POST请求接口。
5. 拆分`Trie.hpp`成`DictTrie.hpp`和`Trie.hpp`，将trie树这个数据结构抽象出来，并且修复Trie这个类潜在的bug并完善单元测试。
6. 重写cjserver的启动和停止，新启动和停止方法详见README.md。

## v2.3.4

1. 修改了设计上的问题，删除了`TrieManager`这个类，以避免造成一些可能的隐患。
2. 增加`stop_words.utf8`词典，并修改`KeywordExtractor`的初始化函数用以使用此词典。
3. 优化了`Trie`树相关部分代码结构。

## v2.3.3

1. 修复因为使用unordered_map导致的在不同机器上结果不一致的问题。
2. 将部分数据结果从unordered_map改为map，提升了差不多1/6的切词速度。(因为unordered_map虽然查找速度快，但是在范围迭代的效率较低。)

## v2.3.2

1. 修复单元测试的问题，有些case在x84和x64中结果不一致。
2. merge进词性标注的简单版本。

## v2.3.1

1. 修复安装时的服务启动问题（不过安装切词服务只是linux下的一个附加功能，不影响核心代码。）

## v2.3.0 

1. 增加`KeywordExtractor.hpp`来进行关键词抽取。
2. 使用`gtest`来做单元测试。

## v2.2.0

1. 性能优化，提升切词速度约6倍。
2. 其他暂时也想不起来了。

## v2.1.1 (v2.1.1之前的统统一起写在 v2.1.1里面了)

1. 完成__最大概率分词算法__和__HMM分词算法__，并且将他们结合起来成效果最好的`MixSegment`。
2. 进行大量的代码重构，将主要的功能性代码都写成了hpp文件。
3. 使用`cmake`工具来管理项目。
4. 使用 [limonp]作为工具函数库，比如日志，字符串操作等常用函数。
5. 使用 [husky] 搭简易分词服务的服务器框架。

[limonp]:http://github.com/yanyiwu/limonp.git
[husky]:http://github.com/yanyiwu/husky.git
[issue50]:https://github.com/yanyiwu/cppjieba/issues/50
[qinwf]:https://github.com/yanyiwu/cppjieba/pull/53#issuecomment-176264929
[jieba]:https://github.com/fxsjy/jieba
[@questionfish]:https://github.com/questionfish
