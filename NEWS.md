# Change History of OpenCC

## Version 1.3.0

2026年4月17日

* **實驗性 Jieba 分詞插件**：新增可載入式 Jieba 分詞插件（`libopencc_jieba`），並支援獨立打包發佈；插件 ABI 採用碼點長度；`winget` 可攜包一併包含插件（[#1091](https://github.com/BYVoid/OpenCC/pull/1091), [#1092](https://github.com/BYVoid/OpenCC/pull/1092), [#1093](https://github.com/BYVoid/OpenCC/pull/1093), [#1099](https://github.com/BYVoid/OpenCC/pull/1099), [#1103](https://github.com/BYVoid/OpenCC/pull/1103), [#1105](https://github.com/BYVoid/OpenCC/pull/1105), [#1111](https://github.com/BYVoid/OpenCC/pull/1111)）。
* **詞典與詞條更新**：新增臺灣 IT 用語及「巨集」術語、距離量詞繁體對應、「奥巴馬↔歐巴馬」、「公元→西元」、「福建面」等詞條；修正「乾/干/余/系/面」等字轉換、「移動資料→行動資料」錯誤、「倒霉→倒黴」錯誤、「文本→文字」過度轉換、「控制台」不應轉換、「背包→揹包」錯誤及「念佛→唸佛」錯誤；修正「星露谷物語」在 `s2twp` 模式下的錯誤分詞（[#1075](https://github.com/BYVoid/OpenCC/pull/1075), [#1079](https://github.com/BYVoid/OpenCC/pull/1079), [#1080](https://github.com/BYVoid/OpenCC/pull/1080), [#1088](https://github.com/BYVoid/OpenCC/pull/1088), [#1089](https://github.com/BYVoid/OpenCC/pull/1089), [#1090](https://github.com/BYVoid/OpenCC/pull/1090), [#1094](https://github.com/BYVoid/OpenCC/pull/1094), [#1096](https://github.com/BYVoid/OpenCC/pull/1096), [#1097](https://github.com/BYVoid/OpenCC/pull/1097), [#1100](https://github.com/BYVoid/OpenCC/pull/1100), [#1106](https://github.com/BYVoid/OpenCC/pull/1106), [#1110](https://github.com/BYVoid/OpenCC/pull/1110), [#1117](https://github.com/BYVoid/OpenCC/pull/1117), [#1122](https://github.com/BYVoid/OpenCC/pull/1122), [#1123](https://github.com/BYVoid/OpenCC/pull/1123)）。
* **安全與穩定性修復**：修復 `UTF8Util` 堆越界讀取（[#794](https://github.com/BYVoid/OpenCC/issues/794), [#799](https://github.com/BYVoid/OpenCC/issues/799)）、二進制字典值邊界檢查、`const_cast` 未定義行為；轉換失敗時返回非零退出碼（[#1072](https://github.com/BYVoid/OpenCC/pull/1072), [#1073](https://github.com/BYVoid/OpenCC/pull/1073), [#1083](https://github.com/BYVoid/OpenCC/pull/1083), [#1098](https://github.com/BYVoid/OpenCC/pull/1098)）。
* **Windows 支援改善**：配置文件改用寬字符路徑檢查；Jieba 插件在 Windows 使用 Unicode 安全文件讀取；修復 Windows Bazel 構建與測試（[#1082](https://github.com/BYVoid/OpenCC/pull/1082), [#1084](https://github.com/BYVoid/OpenCC/pull/1084), [#1086](https://github.com/BYVoid/OpenCC/pull/1086), [#1087](https://github.com/BYVoid/OpenCC/pull/1087), [#1102](https://github.com/BYVoid/OpenCC/pull/1102), [#1104](https://github.com/BYVoid/OpenCC/pull/1104)）。
* **構建與依賴更新**：MARISA 升級至 0.3.1、更新 Bazel BCR 依賴、抑制第三方頭文件編譯警告、新增 Debian 包發佈工作流（[#1078](https://github.com/BYVoid/OpenCC/pull/1078), [#1081](https://github.com/BYVoid/OpenCC/pull/1081), [#1113](https://github.com/BYVoid/OpenCC/pull/1113), [#1116](https://github.com/BYVoid/OpenCC/pull/1116)）。
* **平台與 CI 調整**：移除 EOL Python 3.9 支援；Node.js CI 矩陣移除 Node 20、新增 Node 25；修復 CI 工作流權限設置；更新 README（[#1074](https://github.com/BYVoid/OpenCC/pull/1074), [#1076](https://github.com/BYVoid/OpenCC/pull/1076), [#1109](https://github.com/BYVoid/OpenCC/pull/1109), [#1114](https://github.com/BYVoid/OpenCC/pull/1114), [#1120](https://github.com/BYVoid/OpenCC/pull/1120)）。

## Version 1.2.0

2026年1月22日

* 詞典與詞條更新：STPhrases/TWPhrases整合與修訂，新增醫療術語若干、Nvidia譯名、日文「兔/兎」等詞彙，並補充測試（[#948](https://github.com/BYVoid/OpenCC/pull/948), [#1007](https://github.com/BYVoid/OpenCC/pull/1007), [#1009](https://github.com/BYVoid/OpenCC/pull/1009), [#1011](https://github.com/BYVoid/OpenCC/pull/1011), [#1012](https://github.com/BYVoid/OpenCC/pull/1012), [#1023](https://github.com/BYVoid/OpenCC/pull/1023), [#992](https://github.com/BYVoid/OpenCC/pull/992)）。
* 修正`tw2sp`/`s2twp`配置導致的與詞條錯誤（[#1013](https://github.com/BYVoid/OpenCC/pull/1013), [#1024](https://github.com/BYVoid/OpenCC/pull/1024), [#1025](https://github.com/BYVoid/OpenCC/pull/1025)）。
* `txt`詞典新增註釋語法與排序規則；`TWPhrasesRev`改爲直接納入倉庫並加入一致性測試（[#1016](https://github.com/BYVoid/OpenCC/pull/1016), [#1012](https://github.com/BYVoid/OpenCC/pull/1012)）。
* 測試用例改用JSON格式（[#1006](https://github.com/BYVoid/OpenCC/pull/1006)）。
* 修正截斷UTF-8處理的越界讀取問題（[#1005](https://github.com/BYVoid/OpenCC/pull/1005)）。
* 構建與依賴更新：Bazel 8.5.1、C++17 + marisa 0.3.0、`nan`升級，修復GCC 15與C++17棄用警告（[#1021](https://github.com/BYVoid/OpenCC/pull/1021), [#968](https://github.com/BYVoid/OpenCC/pull/968)）。
* 語言與平台支持調整：移除Python 2殘留、註冊hermetic Python 3.12工具鏈、更新Node.js測試矩陣並移除Node 18與Windows x86（[#946](https://github.com/BYVoid/OpenCC/pull/946), [#1010](https://github.com/BYVoid/OpenCC/pull/1010), [#999](https://github.com/BYVoid/OpenCC/pull/999), [#960](https://github.com/BYVoid/OpenCC/pull/960)）。
* CI、發佈腳本與文檔完善：macOS 14支持、制品上傳與任務取消、README補充Bazel測試/貢獻者說明、繁體中文貢獻指南與opencc-wasm列表、PyPI腳本修復（[#1022](https://github.com/BYVoid/OpenCC/pull/1022), [#973](https://github.com/BYVoid/OpenCC/pull/973), [#1019](https://github.com/BYVoid/OpenCC/pull/1019), [#1017](https://github.com/BYVoid/OpenCC/pull/1017), [#1020](https://github.com/BYVoid/OpenCC/pull/1020)）。

## Version 1.1.9

2024年8月3日

* 恢復`Config::NewFromFile`單參數接口以保持ABI兼容性。
* 增加Bazel Python庫與測試（[#882](https://github.com/BYVoid/OpenCC/pull/882)）。
* 構建與腳本切換至Python 3。
* 修正`rapidjson`補丁與`npmignore`配置。
* 補充安裝文檔與調整Bazel依賴配置（`googletest`改爲`dev_dependency`）。
* 此版本爲維護性更新，詞典無顯著改動。

## Version 1.1.8

2024年7月27日

* 修正Node新版本編譯的問題（[#782](https://github.com/BYVoid/OpenCC/issues/782), [#798](https://github.com/BYVoid/OpenCC/issues/798)）。
* 進一步修正Python包生成腳本（[#875](https://github.com/BYVoid/OpenCC/pull/875)）。
* 引入Bazel構建系統以及CI（[#879](https://github.com/BYVoid/OpenCC/pull/879)）。
* 引入Github MSVC CI（[#880](https://github.com/BYVoid/OpenCC/pull/880)）。
* 爲`opencc`命令行工具添加了字典和配置的路徑`--path`參數。
* 更新附帶的`googletest`版本到1.15，`pybind11`到2.13.1，`tclap`到1.2.5。
* 若干轉換字詞修正（[#609](https://github.com/BYVoid/OpenCC/pull/609), [#698](https://github.com/BYVoid/OpenCC/pull/698), [#707](https://github.com/BYVoid/OpenCC/pull/707), [#760](https://github.com/BYVoid/OpenCC/pull/760), [#779](https://github.com/BYVoid/OpenCC/pull/779), [#786](https://github.com/BYVoid/OpenCC/pull/786), [#792](https://github.com/BYVoid/OpenCC/pull/792), [#806](https://github.com/BYVoid/OpenCC/pull/806), [#808](https://github.com/BYVoid/OpenCC/pull/808), [#810](https://github.com/BYVoid/OpenCC/pull/810), [#825](https://github.com/BYVoid/OpenCC/pull/825), [#826](https://github.com/BYVoid/OpenCC/pull/826), [#837](https://github.com/BYVoid/OpenCC/pull/837), [#864](https://github.com/BYVoid/OpenCC/pull/864), [#865](https://github.com/BYVoid/OpenCC/pull/865), [#870](https://github.com/BYVoid/OpenCC/pull/870), [#877](https://github.com/BYVoid/OpenCC/pull/877), [#878](https://github.com/BYVoid/OpenCC/pull/878)）。

## Version 1.1.7

2023年10月15日

* 添加提交時 python 包重建以驗證包生成 ([#822](https://github.com/BYVoid/OpenCC/pull/822))。
* 支持Python 3.12 和 Node 20，移除針對Python 3.7和Node 16的構建 ([#820](https://github.com/BYVoid/OpenCC/pull/820))。
* add mingw-w64 ci ([#802](https://github.com/BYVoid/OpenCC/pull/802))。
* Add support of CMake config modules ([#763](https://github.com/BYVoid/OpenCC/pull/763))。
* 若干其他小修復。

## Version 1.1.6

2022年12月08日

* 修復python3.11 macos構建 ([#744](https://github.com/BYVoid/OpenCC/pull/744))。
* Bump gtest 和 benchmark 以與最新的 github runners 一起工作 ([#747](https://github.com/BYVoid/OpenCC/pull/747))。

## Version 1.1.5

2022年12月03日

* 支持Python 3.11 ([#728](https://github.com/BYVoid/OpenCC/pull/728))。
* Automatically name SO files ([#708](https://github.com/BYVoid/OpenCC/pull/708))
* Add support for Apple silicon build tag ([#716](https://github.com/BYVoid/OpenCC/pull/716))
* 若干其他小修復。

## Version 1.1.4

2022年6月4日

* 支持Python 3.10（[#637](https://github.com/BYVoid/OpenCC/issues/637)）。
* 移除針對Python 2.7、3.5、3.6和Node 10的構建（[#690](https://github.com/BYVoid/OpenCC/issues/690), [#691](https://github.com/BYVoid/OpenCC/issues/691)）。
* 若干其他小修復。

## Version 1.1.3

2021年9月3日

* 修復部分頭文件不能單獨使用的問題（#550）。
* 修復引入系統pybind11的方法（#566）。
* 支持Node.js 16（#597）。
* 支持Python 3.9（#603）。
* 修正轉換錯誤。
* 若干其他小修復。

## Version 1.1.2

2021年3月2日

* 新增香港繁體轉換。
* 根據《通用漢字規範表》修正大量簡體異體字轉換。調整臺灣標準，避免過度轉換。
* 修正編譯兼容性問題，包括並行編譯。
* 修正1.1.0以來引入的性能嚴重下降問題。

## Version 1.1.1

2020年5月22日

* 正式提供[Python](https://pypi.org/project/OpenCC/)接口和TypeScript類型標註。
* 更新動態鏈接庫`SOVERSION`到`1.1`，由於C++內部接口發生變更。
* 進一步改進與Windows MSVC的兼容性。
* 簡化頭文件結構，加快編譯速度。刪除不必要的`using`。
* 修復部分香港標準字。

## Version 1.1.0

2020年5月10日

* 新辭典格式`ocd2`，基於Marisa Trie 0.2.5。辭典大小大幅減少。`STPhrases.ocd`從4.3MB減少到`STPhrases.ocd2`的924KB。
* 升級依賴的rapidjson版本到1.1.0，tclap到1.2.2，gtest到1.11.0。
* 更改「涌/湧」的默認轉換，修正多個詞組轉換。
* 提供Windows的預編譯版本下載（AppVeyor）。
* 增加基準測試結果。

## Version 1.0.6

2020年4月13日

* 正式支持日本語新字體轉換。
* 升級Node.js依賴，改進兼容性。
* 修復多處多平臺編譯和兼容性問題。
* 修正大量轉換錯誤。

## Version 1.0.5

2017年2月6日

* 修正Windows下CMake和Visual Studio的問題。
* 修正FNV Hash的32位編譯警告。
* 增加若干臺灣常用詞彙轉換和異體字轉換。
* 增加和修正若干轉換問題。
* 加快Node模塊編譯速度。
* 增加Node模塊的詞典轉換接口和Promise接口。

## Version 1.0.4

2016年4月1日

* 使編譯時的腳本兼容Python 3。
* 修正Visual C++ 2015的編譯問題。
* 增補臺灣、香港地區用字用詞轉換。
* 更新nan以修正Node.js擴展編譯兼容性問題。

## Version 1.0.3

2015年7月22日

* 添加化學元素臺灣用字轉換。
* 增補100餘組缺失的簡繁轉換字對。
* 增補香港標準字。
* 使用nan解決Node.js擴展編譯兼容性問題。
* 命令行轉換工具支持就地轉換。
* 測試框架遷移到GTest。
* 修正Visual C++的編譯問題。
* 實現無詞典詞彙抽取和分詞算法。
* 優化轉換性能。

## Version 1.0.2

2014年11月8日

* 修正C語言接口的編譯錯誤問題
* 修正默認簡繁轉換文件名錯誤問題
* `DictEntry`增加`Values()`方法

## Version 1.0.1

2014年10月18日

* 使用C++11完全重寫OpenCC
* 修復大量轉換錯誤
* 增加香港繁體轉換

## Version 0.4.3

2013年5月17日

* 增加接口`opencc_convert_utf8_free`
* 修正Node.js插件內存泄漏問題
* 修正Windows下獲取當前目錄的問題

## Version 0.4.2

2013年4月14日

* 修正「阪」、「薰」繁簡轉換
* 增加四對缺失的簡繁轉換
* 增加API文檔，由Doxygen生成
* 重構大量代碼

## Version 0.4.1

2013年3月21日

* 修正Node.js 0.10兼容性問題。
* 從Unihan數據庫增加若干缺失的簡繁轉換單字。

## Version 0.4.0

2013年3月2日

* 修正「雕」「谥」「峯」轉換，新增數百條臺灣科技詞彙。
* 修正命令行-h錯誤。
* 修正長行讀取錯誤。
* 修正錯誤類型拼寫錯誤。
* 修正UTF-8編碼轉換錯誤。
* 自動跳過UTF-8的BOM。
* 修正配置和數據文件相對路徑問題。
* 增加了gyp編譯系統。
* 增加了Node.js接口。

## Version 0.3.0

2011年12月2日

* 增加中國大陸、臺灣地區異體字和習慣用詞轉換功能。
* 修正詞典轉換鏈爲奇數時的緩衝區複製Bug。
* 修正Big Endian平臺上的UTF-8轉換錯誤。
* 修正「齣」「薑」詞組的問題。
* 修正「钁」「卷」「干」「薰」「糉」「蝨」「麺」。
* 增加「綑」到「捆」的繁簡轉換。
* 增加「跡」「蹟」對立。
* 增加「夫」「伕」對立。
* 增加「毀」「譭」「燬」對立。
* 增加「背」「揹」對立。

## Version 0.2.0

2010年12月23日

* 取消libopencc對iconv的依賴。
* 增加UTF8編碼格式錯誤時提示信息。
* 重構Python封裝。
* 修正讀取一行長度超過緩衝區時的UTF8截斷錯誤。
* 使用CMake代替Autotools構建編譯框架。
* 修正包括「拿不準」在內諸多簡繁轉換問題。

## Version 0.1.2

2010年9月16日

* 增加「僅分詞」和「顯示多重候選字詞」的轉換接口。
* 改進辭典文件的結構。
* 修正轉換緩衝區永遠不足的Bug。
* 修正多辭典轉換時略過某個辭典的Bug。
* 修正輸入爲空時轉換的Bug。
* 改進opencc命令行工具參數提示和幫助。

## Version 0.1.1

2010年8月10日

* 增加簡繁混雜到簡體或繁體的轉換。
* 增加多詞典/詞典組的轉換支持。
* 修正big endian平臺上的兼容性問題。
* 修正apple平臺下編譯iconv依賴的問題。
* 修正辭典中詞條長度長度不相等時轉換錯誤的Bug。
* 重構辭典代碼抽象。
* 增加編譯時的測試。
* 分離辭典爲字典和詞典。

## Version 0.1.0

2010年7月28日

* 修正文件名緩衝區不足的Bug。
* libopencc版本更新至1.0.0。
* 分離臺灣特有的繁簡轉換「著」「么」。
* 修改「众」「教」「查」「污」對應默認異體。
* 加入「齧啮」「灩滟」繁簡轉換。
* 增加「岳嶽」一簡對多繁轉換。
* 隱藏不必要的類型，更新接口註釋。

## Version 0.0.5

2010年7月21日

* 修正`wchar_t`兼容性問題，使用`ucs4`。
* 增加Windows移植分支。
* 修正一個文件名緩衝區分配的問題。
* 增加「囉」「溼」「廕」「彷」「徵」繁簡轉換。

## Version 0.0.4

2010年7月16日

* 增加「卹」「牴」「皁」「羶」「薹」等轉換。
* 精簡辭典中大量不必要的數詞（含「千」「萬」）。
* 修正最短路徑分詞時優先後向匹配的實現問題。
* 修正辭典加載兼容性問題，當無法mmap時直接申請內存。
* 修正C++接口在64位平臺下編譯的問題。

## Version 0.0.3

2010年6月22日

* 加入繁體到簡體的轉換。
* 增加提示信息的中文翻譯，使用`GNU Gettext`。
* 增加辭典配置文件支持。
* 修正一些兼容性Bug。

## Version 0.0.2

2010年6月19日

* 分離詞庫。
* 增加平面文件詞庫讀取的支持。
* 增加平面文件詞庫到`Datrie`詞庫的轉換工具`opencc_dict`。
* 提供UTF8文本直接轉換的接口。

## Version 0.0.1

2010年6月11日

* OpenCC初始版本釋出。
* 支持簡繁轉換。
