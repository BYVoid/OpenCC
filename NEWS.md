# Change History of OpenCC

## Version 1.4.2

2026年8月22日

* **發佈重點**：轉換熱路徑大幅加速（純文字語料的整體轉換時間最多降至原本的 1/7），修復大端序平台載入 legacy `.ocd` 字典得到空字典的問題，並新增兩個實驗性 API（詞級候選查詢與一對多歧義標註，含 CLI `--ambiguities`）。詞庫方面修正了 `s2twp` 的數處貪婪匹配錯誤與 `tw2t`／`tw2s` 對簡體「么」的破壞性轉換，補充臺灣／香港地區詞，並修正古籍語境下 `s2t` 的一對多字用字。C++ ABI 與 1.4.1 相同（SOVERSION 1.4），實驗性 API 皆位於未安裝的私有標頭或巨集開關之後，下游程式無需重新連結。
* **詞庫更新**：
    * 修復 `tw2t`／`tw2s`／`tw2sp` 會把已是簡體的「什么」轉成「什幺」的問題：生成的 `TWVariantsRev` 含 `么 -> 幺`（臺灣標準 `幺 -> 么` 的反向），在 `TWVariantsRevPhrases.txt` 補上「什么／怎么／这么／那么／多么／要么／好么／甚么」等例外映射至「麼」形，同時保留「老么 → 老幺」等真正的 yao1 詞（[#1469](https://github.com/BYVoid/OpenCC/issues/1469), [#1473](https://github.com/BYVoid/OpenCC/pull/1473)）。
    * 修復 `s2twp` 因較短詞條先命中而拆錯詞的問題：補上「互联网络」「快闪存储器」「老挝人民民主共和国」的完整詞條，並將「老撾人民民主共和國」對應到臺灣官方譯名「寮人民民主共和國」（[#1467](https://github.com/BYVoid/OpenCC/issues/1467), [#1468](https://github.com/BYVoid/OpenCC/pull/1468)）。
    * 「梁／樑」：新增臺灣地區用字映射與人名、地名保護詞條（[#1407](https://github.com/BYVoid/OpenCC/pull/1407)）；並將 `TWVariantsRevPhrases` 與 `STPhrases` 對齊，補上 41 條梁（棟梁義）的反向詞組映射，使 `tw2t` 能從上下文還原「橋樑」而不誤傷姓氏、地名（[#1424](https://github.com/BYVoid/OpenCC/pull/1424)）。
    * 新增奧斯卡最佳導演與最佳影片製片人的臺灣／香港譯名詞條（取自中文維基百科 `NoteTA` 地區詞表並以 Wikidata 交叉核對，僅收錄真正存在音譯差異者，如 卡梅隆／卡麥隆／金馬倫）（[#1466](https://github.com/BYVoid/OpenCC/pull/1466)）。
    * 新增地區詞「內存條 → 記憶體模組」（`s2twp`／`s2hkp`，含反向詞條）（[#1475](https://github.com/BYVoid/OpenCC/pull/1475)）、「数字人文 → 數位人文」（`s2twp`／`tw2sp`）（[#1465](https://github.com/BYVoid/OpenCC/pull/1465)）。
    * `s2t`：修正古籍語境下一對多字的用字。`征`／`咸`／`斗`／`里`／`范`／`岳`／`冲`／`郁`／`谷` 在 `STPhrases` 沒有對應詞條時會退回 `STCharacters`，而那裏的候選次序是按現代用法排的，碰上文言文和書名就常常選錯。此次按義項補入詞條：征伐義（`征南`、`从征实录`）、年號與副詞義（`咸淳`、`少长咸集`）、星宿義（`斗姆`、`东斗星君`）、地名與音譯（`甫里`、`摩里支`）、姓氏義（`范文正`、`岳武穆`）、道教與梵語音譯（`道冲`、`郁迦罗越`）；`谷神` 改為保留「谷」（《老子》「谷神不死」），`谷神星` 則因鍵更長，仍映射為「穀神星」。這批問題是在清理殆知閣文獻時發現的，該批書名中 47 條誤轉修正了 46 條，現代語義的轉換一律不受影響（[#1479](https://github.com/BYVoid/OpenCC/pull/1479)）。
    * `s2t`：「小丑」不再預設轉換為「小醜」（[#1423](https://github.com/BYVoid/OpenCC/pull/1423)）；「雇」改為 `STCharacters` 中「僱」以外的非預設候選（[#906](https://github.com/BYVoid/OpenCC/pull/906)）；新增「这周／那周／若干周／几周／周线 → 週」等指示與量化用法，並補上「周围／周边／周遭」的分詞保護詞條（[#1428](https://github.com/BYVoid/OpenCC/pull/1428)）；新增顯式詞條「复盘 → 復盤」，使其不再只依賴 `复` 的候選順序（[#1427](https://github.com/BYVoid/OpenCC/pull/1427)）。
    * `t2s`：修正過度轉換「乾斷食 → 干断食」「乾紅 → 干红」（[#621](https://github.com/BYVoid/OpenCC/issues/621), [#1429](https://github.com/BYVoid/OpenCC/pull/1429)）。
    * 修正「鑒」相關詞條與若干提稱語，如「郝铭鉴 → 郝銘鑒」（[#1421](https://github.com/BYVoid/OpenCC/pull/1421)）。
* **效能**：
    * 轉換熱路徑跳過不可能匹配的字元區段：`PrefixMatch` 在建構時建立 key 起始字元表並提供 `SkipUnmatchable()`，讓 `Conversion::AppendConverted` 與 `MaxMatchSegmentation` 一次消化整段不可能作為詞條開頭的字元（ASCII、標點、emoji 以及不作為任何 key 開頭的漢字），取代逐字元的 trie 查詢。過濾以精確碼位而非 UTF-8 首位元組進行（`0xE4..0xE9` 這些常見漢字首位元組幾乎都有 key，按首位元組只能跳過約 10%，按碼位則有 69–76% 的位置不匹配任何 key）。22 MB 繁體語料的整體 CLI 時間（含字典載入）：`t2s` 3.40s → 0.52s、`t2tw` 3.43s → 0.30s、`s2twp` 8.40s → 4.86s；4 MB 中英混合語料的 `t2s` 0.83s → 0.11s。啟動時間、最大 RSS 與缺頁次數不變，轉換輸出逐位元組不變（以對照逐字元參考實作的差分 fuzz 測試保證）（[#1464](https://github.com/BYVoid/OpenCC/pull/1464)）。
* **實驗性 API / CLI**：
    * 新增 librime 風格的詞級候選查詢 `Converter::GetConversionCandidates`，列舉單一詞經整條轉換鏈後的所有形式（如 `s2t` 的 `里` 展開為 `里`、`裏`）。該 API 以 `OPENCC_ENABLE_UNSTABLE_API` 巨集開關隔離，不進入已安裝標頭與 `OPENCC_ABI_VERSION` 契約；成員刻意設為非虛擬，開啟巨集不改變 `Converter` 的 vtable（[#1431](https://github.com/BYVoid/OpenCC/pull/1431)）。
    * 新增一對多歧義標註與 CLI `--ambiguities` 模式：轉換結果與 `Converter::Convert` 完全一致，同時標出每一個字典匹配為一對多的輸出區段（如 `STPhrases` 的 `文丑 -> 文丑 文醜`、`TSCharacters` 的 `鍾 -> 钟 锺`），呼叫方可據此定位歧義位置並按需以 `GetAllConversions` 取得候選。檔案輸入輸出 JSONL 記錄且記憶體用量有界，互動式 stdin 則逐行輸出 JSON 封包。實作位於未安裝的私有標頭，不改變已安裝標頭與 ABI，轉換熱路徑不受影響（[#1461](https://github.com/BYVoid/OpenCC/pull/1461), [#1462](https://github.com/BYVoid/OpenCC/pull/1462)）。
* **`.ocd` 字典格式**：
    * 修復大端序平台（s390x、ppc64、sparc64、hppa）上 legacy 64-bit `.ocd` 佈局的偵測：原本以探測位元組 [4..7] 是否為零來判斷，該寫法僅在小端序主機成立，導致大端序平台將舊檔誤判為固定寬度格式並載入為空字典。三處判斷統一改用 `LooksLikeLegacy64Field()`，以原生 `uint64_t` 讀取探測值並檢查高 32 位，在兩種位元組序下皆正確（[#1471](https://github.com/BYVoid/OpenCC/issues/1471), [#1472](https://github.com/BYVoid/OpenCC/pull/1472)）。
* **Node.js**：
    * npm 套件的源碼編譯 fallback 從 node-gyp 改為 Bazel：原生 addon 優先透過 `@opencc/opencc-<platform>-<arch>` 預編譯 scoped 套件安裝（涵蓋 macOS x64/arm64、Linux x64/arm64、Windows x64，均以 Bazel 構建，字典隨包預編譯）；沒有預編譯二進位套件的平台，`npm install` 會以 Bazel 完整地從源碼構建——編譯 `//node:opencc` 並重新生成字典（`bazel` / `bazelisk` 取自 PATH，或經 `npx` 自動下載 bazelisk；字典生成腳本的 Python 由 Bazel hermetic 工具鏈提供）。`binding.gyp` 與 `.gypi` 構建定義刪除，`node-gyp`、`node-gyp-build`、`node-addon-api`、`prebuildify` 依賴與 `prebuild` npm script 一併移除；tarball 不再攜帶 `deps/`（C++ 三方依賴改由 Bazel Central Registry 提供），改為攜帶 Bazel workspace、`src/` C++ 源碼與 `data/` 字典源檔（[#1426](https://github.com/BYVoid/OpenCC/pull/1426), [#1430](https://github.com/BYVoid/OpenCC/pull/1430)）。
    * CI 測試任務、Makefile target 與源碼 checkout 的開發構建一併改用 Bazel，與 npm 發佈所用的構建路徑一致；Windows CI 任務移至 `windows-2025-vs2026`（[#1426](https://github.com/BYVoid/OpenCC/pull/1426)）。
* **打包 / 構建系統**：
    * `packaging/debian/` 同步至 Debian 的 `1.4.1+ds1-8`。Debian 已採納上游先前提出的 jieba 插件打包提案（拆為 `opencc-jieba` 與 arch:all 的 `opencc-jieba-data` 兩個套件，內嵌 cppjieba 的理由記於 `debian/README.source`）；Debian 端亦重新啟用了 autopkgtest。`packaging/debian/SPLIT_PROPOSAL.md` 隨提案落地移除。另外 `1.4.1+ds1-8` 修復了 `.ocd2` 被放進 `Architecture: all` 套件的問題：`.ocd2` 是依主機位元組序寫出的 marisa-trie 映像，所有架構都會拿到 arch:all buildd 上構建的小端序副本，opencc 在 s390x 上因而以 `size > avail_` 失敗（Debian bug [#1145010](https://bugs.debian.org/1145010)，由重新啟用的 s390x autopkgtest 揪出）。`*.ocd2` 已移回 arch:any 的 `libopencc1.4`、`jieba_merged.ocd2` 移回 arch:any 的 `opencc-jieba`；`libopencc-data` 與 `opencc-jieba-data` 現在只含 JSON 配置與純文字 jieba 詞典來源。
    * Debian 套件構建改在 `debian:sid` 容器中進行（同步來的打包以 unstable 為目標，Ubuntu LTS runner 無法滿足），並在 `dpkg-buildpackage` 前套用 `debian/patches` quilt series（自動跳過已在樹中的補丁）；arm64 改用 `ubuntu-24.04-arm` runner 原生構建，取代先前的交叉構建（[#1420](https://github.com/BYVoid/OpenCC/pull/1420)）。
    * 新增 `packaging/freebsd/`：FreeBSD ports tree `chinese/opencc` 條目的參考副本，供上游審閱與準備提交給 port maintainer 的改動。
    * 新增精簡源碼包發佈流程（`release-source.yml`）：`OPENCC_SOURCE_PACKAGE_PROFILE` 提供 `opencc`（C++ 核心與 Jieba 插件）與 `bazel`（再加上 `node/`、`python/`、`scripts/`，可直接以 Bazel 構建並測試 `//node`、`//python`）兩種精簡設定，各自產生 CPack 壓縮包，以 CMake 與 Bazel 雙路徑冒煙測試（含 Jieba 插件建置；`bazel` 設定另跑 CI 的完整 Bazel 測試目標）後，重新打包以正規化檔案擁有者、時間戳與排序，產生 `.tar.gz` 與 `.tar.xz` 上傳至 draft release；為此保留 Bazel workspace 檔案與頂層 `patches/`，僅排除可能含 API key 的 `.bazelrc.user`。打包與冒煙測試邏輯改由 `scripts/build-source-package.sh` 承載，workflow 只負責呼叫，便於本機重現。
    * 修正 `cmake/GitVersion.cmake` 的 fallback 版本號：1.4.1 發佈時仍停留在 1.4.0，導致從釋出的 tarball（無 git 資訊）構建時版本回報為 1.4.0；本次發佈同步將其更新至 1.4.2，並讓 `scripts/compute-version.sh` 的 fallback 與之一致（先前停留在 1.2.1）。因此 Debian 打包的 `0010-fix-git-version-fallback.patch` 對 1.4.2 已無作用而移除，`0012-fix-legacy-dict-detection-on-big-endian.patch` 亦已包含於本次發佈。
    * Windows 發佈自本版起啟用程式碼簽章憑證：可攜式 CLI 壓縮包中的執行檔，以及 win32-x64 npm 二進位（`opencc.node`、`opencc-jieba.dll`）皆經 Authenticode 簽章，簽章服務由 [SignPath.io](https://about.signpath.io/) 免費提供、憑證由 [SignPath Foundation](https://signpath.org/) 免費提供，發佈流程改用其 release-signing policy；相關聲明與簽章／隱私政策見 README 的「Code Signing 程式碼簽章」一節（[#1458](https://github.com/BYVoid/OpenCC/pull/1458), [#1459](https://github.com/BYVoid/OpenCC/pull/1459)）。
* **測試**：
    * `PrefixMatchTest` 改用測試專屬的字典檔名，避免與 `MarisaDictTest` 在共享工作目錄下爭用同一個 `dict.ocd2`，該競爭會使並行 ctest（如 `dh_auto_test` 的 `ctest -j8`）偶發失敗。

## Version 1.4.1

2026年7月12日

* **發佈重點**：修復 `opencc` npm 套件在沒有預編譯二進制的平台（如 Intel Mac）上退回源碼編譯時，因源碼包缺少檔案而安裝失敗的問題，並新增 darwin-x64（Intel Mac）預編譯 npm 套件；另包含一批詞庫修正、`.ocd` 字典跨平台輸出修復與 Debian 打包流程重建。C++ ABI 與 1.4.0 相同（SOVERSION 1.4），下游程式無需重新連結。
* **詞庫更新**：
    * 修正「托 / 託」候選順序：`s2t` 中 `托` 預設保留不轉換（如 `托着`、`茶托`、人名音譯），並補充「託」委託義的詞組轉換（[#1405](https://github.com/BYVoid/OpenCC/pull/1405)）。
    * 依《通用規範漢字表》與《現代漢語詞典》修正「牴 → 抵」，以及「復 / 複 / 覆」相關轉換（[#758](https://github.com/BYVoid/OpenCC/pull/758)）。
    * 修正「黄粱美梦」相關轉換：不再輸出「黃樑美夢」，並將 `STPhrases.txt` 中誤作「黄梁」的詞條統一為「黄粱」（[#1408](https://github.com/BYVoid/OpenCC/pull/1408), [#1410](https://github.com/BYVoid/OpenCC/pull/1410)）。
    * `t2s` 新增「遶 → 绕」轉換（[#1399](https://github.com/BYVoid/OpenCC/pull/1399)）；`t2s` 中保留簡體字形「坏」不再誤轉（[#1391](https://github.com/BYVoid/OpenCC/pull/1391)）。
    * 新增書法家「梁昇卿」人名詞條（[#1402](https://github.com/BYVoid/OpenCC/pull/1402)）；新增香港地區詞「舒芙蕾 / 梳芙厘」（soufflé）（[#1387](https://github.com/BYVoid/OpenCC/pull/1387)）。
    * 日文字體：補充 `JPShinjitaiCharacters` 缺失的 11 個新字體字元（依安岡孝一「人名用漢字の新字旧字」），以相容性自映射收錄並用 `@reverse-prefer` 修正 `t2jp` 反向優先序（[#1394](https://github.com/BYVoid/OpenCC/pull/1394)）。
* **`.ocd` 字典格式 / 字典工具**：
    * `BinaryDict` 序列化改為固定寬度（uint32）欄位，使 32-bit 與 64-bit 構建寫出的 `.ocd` 位元組一致；讀取端自動偵測舊版 64-bit 佈局，舊檔案仍可在任何平台載入（[#1414](https://github.com/BYVoid/OpenCC/pull/1414)）。
    * CMake 新增 `OPENCC_DICT_FORMAT` 選項（`ocd2` / `text` / `ocd`），可選擇構建輸出的字典格式，配置 JSON 隨之改寫（[#1396](https://github.com/BYVoid/OpenCC/pull/1396)）。
    * `opencc_dict` 整合 cppjieba 字典轉換（新增 `cppjieba_utf8` 輸入格式），取代並停止安裝原 `cppjieba_dict` 工具（[#1392](https://github.com/BYVoid/OpenCC/pull/1392)）；CMake / Bazel 構建流程進一步改為直接以 `opencc_dict` 生成 jieba 合併詞典並移除私有構建工具，獨立構建 jieba 插件時需系統安裝 OpenCC ≥ 1.4.1 的 `opencc_dict`。
    * vendored `darts-clone` 改為從 BCR 引入 `0.32h.bcr.1`，移除本地補丁（[#1386](https://github.com/BYVoid/OpenCC/pull/1386), [#1401](https://github.com/BYVoid/OpenCC/pull/1401)）。
* **Node.js**：
    * 修復 npm 源碼包缺少 `src/ConfigBasedConverter.hpp`、`src/PipelineConverter.cpp/.hpp`、`src/SingleStageConverter.cpp/.hpp`、`src/opencc_config_schema.inc` 六個編譯所需檔案，導致 `npm install` 源碼編譯失敗（[#1409](https://github.com/BYVoid/OpenCC/issues/1409), [#1411](https://github.com/BYVoid/OpenCC/pull/1411)）。
    * 新增 `@opencc/opencc-darwin-x64` 與 `@opencc/opencc-jieba-darwin-x64` 預編譯套件，Intel Mac 不再需要源碼編譯（[#1411](https://github.com/BYVoid/OpenCC/pull/1411)）。
    * `opencc-jieba` 預編譯二進制的 macOS 最低版本從構建機的 15.0 降至 11.0，與 `opencc` 主套件的預編譯二進制一致。
    * `prepack` 驗證腳本新增 gyp 源碼與 `#include` 閉包檢查，確保源碼編譯所需的所有檔案都在 npm 包的 `files` 白名單內，防止同類問題再次發生。
    * CI 新增「從 `npm pack` 產出的 tarball 進行源碼編譯並安裝」的測試（ubuntu / macOS ARM / macOS Intel），覆蓋無預編譯二進制平台的實際安裝路徑。
    * JavaScript API 支援以 inline config 物件建立轉換器，保留原有字串配置行為；並覆蓋 bundled assets、自定義配置目錄與 tofu-risk 過濾的測試（[#1412](https://github.com/BYVoid/OpenCC/pull/1412)）。
    * `OpenCCConfig` TypeScript 型別改由 JSON schema 自動生成，取代原本寬鬆的 `Record` 型別（[#1413](https://github.com/BYVoid/OpenCC/pull/1413)）。
    * Node 預編譯二進制（`@opencc/opencc-*` scoped 套件）改用 Bazel 構建發佈，`npm run prebuild`（prebuildify / node-gyp）不再是官方發佈二進制的構建方式；Bazel 不支援的平台從此不再提供官方預編譯二進制，在這些平台上 `npm install` 目前仍會退回 node-gyp 源碼編譯，但此 fallback（`binding.gyp`）已計劃在後續版本中移除，屆時 Node 構建將統一使用 Bazel（[#1416](https://github.com/BYVoid/OpenCC/pull/1416)）。
    * 重寫 Node demo 範例（[commit](https://github.com/BYVoid/OpenCC/commit/6b382c6489efe4874350ec33531b46b32e08c2d5)）。
* **打包 / 構建系統**：
    * macOS 頂層 CMake 構建現在預設啟用 jieba 插件（`BUILD_OPENCC_JIEBA_PLUGIN=ON`），Homebrew 等以預設選項構建的套件管理器將直接提供 `opencc-jieba` 插件與 `*_jieba.json` 配置；其他平台與以子項目方式（FetchContent / `add_subdirectory`）引入的構建維持預設關閉，Python wheel 構建亦顯式關閉。
    * 重建 Debian 打包流程：新增頂層 `debian/` 目錄使 `dpkg-buildpackage` 可端到端重現 Debian 構建，`packaging/debian/` 補丁鏡像與 ver.1.4.0 源碼對齊，並改進 deb 構建腳本與 workflow（[#1398](https://github.com/BYVoid/OpenCC/pull/1398), [#1385](https://github.com/BYVoid/OpenCC/pull/1385), [#1406](https://github.com/BYVoid/OpenCC/pull/1406)）。
    * Bazel 升級至 9.1.1（[#1384](https://github.com/BYVoid/OpenCC/pull/1384)）；`rules_node_addon` 更新至 BCR 1.0.3（[#1370](https://github.com/BYVoid/OpenCC/pull/1370)）。
    * MSVC `/W4` 改用 `add_compile_options` 設定，避免 `rc.exe` 資源編譯失敗（[#1393](https://github.com/BYVoid/OpenCC/pull/1393)）；CMake 恢復支援舊式 Python executable 選項（[#1389](https://github.com/BYVoid/OpenCC/pull/1389)）。
    * 新增 `opencc-v<version>-resources.zip` 資源包發佈 workflow；`release-pypi` workflow 新增 push tag 觸發，與其他發佈 workflow 一致。
    * 穩定 Doxygen 文檔頁面 anchor ID，不再隨構建路徑變化（[#1400](https://github.com/BYVoid/OpenCC/pull/1400)）。
* **測試**：
    * 新增 librime OpenCC 相容性測試，覆蓋 `GetConversionChain`、精確詞條展開與前綴匹配 fallback（[#1395](https://github.com/BYVoid/OpenCC/pull/1395)）。

## Version 1.4.0

2026年7月1日

* **發佈重點**：本版本主要目的是將 C++ ABI / SOVERSION 從 1.3 提升至 1.4，避免舊版下游程式靜默載入 ABI 不相容的新 `libopencc`；另外修復一個 1.3.2 與 `librime` 不相容之行為。詞表相較 1.3.2 有少量改動。
* **詞庫更新**：
    * 修正 `s2twp` 中 `芯片` 的分詞與轉換結果，避免區域詞被拆開後無法套用臺灣用語（[commit](https://github.com/BYVoid/OpenCC/commit/773a9e11d2e3b0c13f018f0effaab3ded8e0e033)）。
    * 修正「批覆 / 批复」、「陞 / 升」、「锺繇」、「魏徵」、「搧 / 扇」、「沈厚 / 沉厚」、「芝柏表」、U+20F24 相關詞條，以及若干含「陞」「钜」字的人名（[#1365](https://github.com/BYVoid/OpenCC/pull/1365)）。
    * 修正含「台」字的人名、「今周刊」、「爱丽舍 / 爱丽舍宫」、「舖 / 铺」等轉換；調整 `HKVariantsRevPhrases` 中的香港用字反向映射（[#1369](https://github.com/BYVoid/OpenCC/pull/1369)）。
    * 新增「自干五 → 自乾五」；補充動詞用法「扇 → 搧」的詞組轉換（`呼扇`、`扇火`、`扇風`、`扇風耳朵`、`鋪眉搧眼`）並在字符映射中加入 `搧` 作為 `扇` 的候選（[#1371](https://github.com/BYVoid/OpenCC/pull/1371)）。
    * 修正 `别强` 預設轉換為 `別強`（[#1366](https://github.com/BYVoid/OpenCC/pull/1366), [#1378](https://github.com/BYVoid/OpenCC/pull/1378)）；移除 `STPhrases.txt` 中以全形句號「．」分隔、實際難以命中的人名詞條（[#1379](https://github.com/BYVoid/OpenCC/pull/1379)）。
* **Darts / `.ocd` 字典格式**：
    * 升級 vendored `darts-clone` 至 v0.32h，並加強 malformed `.ocd` 驗證，避免讀取損壞字典時越界或接受不一致資料（[#1372](https://github.com/BYVoid/OpenCC/pull/1372)）。
    * 新寫出的 `.ocd` Darts unit 固定為 32-bit，同時自動偵測並讀取 legacy 64-bit `.ocd`，修復舊檔 prefix search 邊界問題（[#1373](https://github.com/BYVoid/OpenCC/pull/1373)）。
    * 改善 `DartsDict` 讀取錯誤或 legacy `.ocd` 檔案時的記憶體所有權與例外安全，避免驗證失敗路徑洩漏或留下不完整狀態（[#1382](https://github.com/BYVoid/OpenCC/pull/1382)）。
    * Darts 支援改為常態啟用，移除 `ENABLE_DARTS` / `USE_SYSTEM_DARTS` 分支；內建 `opencc_dict` 的 `ocd` 轉換和 runtime `.ocd` 載入，`.ocd` 字典可從 filesystem 或 resource zip 載入（[#1374](https://github.com/BYVoid/OpenCC/pull/1374)）。
* **效能提升**：
    * 純 `union` 詞典組中的單詞典前綴匹配新增 fast-path，減少不必要的群組遍歷（[#1367](https://github.com/BYVoid/OpenCC/pull/1367)）。
    * 從 filesystem 或 resource zip 載入 `.txt` 格式詞典時，runtime 會直接構建 in-memory Darts 字典，提升初始化和長文本前綴匹配效能；新增 `SpeedBenchmark` Bazel 目標供後續基準測試使用（[#1376](https://github.com/BYVoid/OpenCC/pull/1376)）。
* **C++ ABI / API**：
    * SOVERSION 從 1.3 升至 1.4，避免已連結 `libopencc.so.1.3` 的舊 C++ 下游程式靜默載入 1.3.2 之後 ABI 不相容的 OpenCC；下游程式需重新連結（[#1377](https://github.com/BYVoid/OpenCC/pull/1377)）。
    * 修復啟用 `normalization` 前處理時 `Converter::GetConversionChain()` 返回空指標的問題；現在配置載入器會保留主 `conversion_chain` 供 librime 等下游 introspection 使用，`PipelineConverter` 本身仍保留「無單一 conversion chain」語義（[#1380](https://github.com/BYVoid/OpenCC/pull/1380)）。
    * 清理公開標頭安裝範圍：`DictConverter.hpp`、`PhraseExtract.hpp`、`UTF8StringSlice.hpp`、`BinaryDict.hpp`、`DartsDict.hpp` 不再安裝至 `include/opencc/`；`DictGroup.hpp`、`MaxMatchSegmentation.hpp`、`MarisaDict.hpp` 等配置可見或插件／字典支援標頭仍保留公開（[#1380](https://github.com/BYVoid/OpenCC/pull/1380)）。
* **Node.js / Python / 構建系統**：
    * 修復 `opencc-jieba` npm 套件中 normalization 相關資源路徑，並補充 Node.js 測試覆蓋（[commit](https://github.com/BYVoid/OpenCC/commit/6f431a2b1d83fdfed0aec5f53341bb93b3c3288c)）。
    * Bazel 依賴更新：`rules_node_addon` 切換至 BCR 1.0.2，`node_addon_api` 更新至 BCR 8.9.0；vendored RapidJSON 與 `1.1.0.bcr.20250205` 對齊（[#1368](https://github.com/BYVoid/OpenCC/pull/1368)）。
    * Bazel BUILD 檔改用顯式規則載入並補充必要 `exports_files`，提升 Bazel 9/10 前向相容性（[#1381](https://github.com/BYVoid/OpenCC/pull/1381)）。
    * 修復 native CLI 與 Node.js CLI 的內建設定檔說明清單遺漏 `s2hkp.json`、`hk2sp.json`；兩個設定在 v1.3.2（[#506](https://github.com/BYVoid/OpenCC/pull/506)）新增但未列於說明文字。
* **測試**：
    * 補充 `s2twp` mixed-script 測試、Darts `.ocd` 32-bit / legacy 64-bit 讀取測試、resource zip 載入測試、normalization conversion-chain introspection 測試，以及多組詞庫回歸測試。
    * Windows Zig 構建腳本更新以配合 vendored Darts 與 Node 原生模組構建（[#1375](https://github.com/BYVoid/OpenCC/pull/1375)）。
    * 清理配置 schema warning 測試（[commit](https://github.com/BYVoid/OpenCC/commit/7bb8af01585337a94d293ec591861f461450e4af)）。

## Version 1.3.2

注意：此版本 ABI 與先前不相容，問題已於 1.4.0 修復；建議此版本的使用者更新至 1.4.0 或更新的版本。

2026年6月28日

* **詞庫更新**：
    * 地區詞新增：20 種基本胺基酸及常見醫學／藥物臺灣名詞（[#1289](https://github.com/BYVoid/OpenCC/pull/1289), [#1299](https://github.com/BYVoid/OpenCC/pull/1299)）、`Fortune 500` 公司譯名（[#1270](https://github.com/BYVoid/OpenCC/pull/1270)）、香港專有名詞（[#1256](https://github.com/BYVoid/OpenCC/pull/1256), [#1267](https://github.com/BYVoid/OpenCC/pull/1267)）、美國州名／領地譯名（[#1269](https://github.com/BYVoid/OpenCC/pull/1269)）；持續修正多組因分詞界限導致的轉換失敗（[#1214](https://github.com/BYVoid/OpenCC/pull/1214), [#1296](https://github.com/BYVoid/OpenCC/pull/1296), [#1318](https://github.com/BYVoid/OpenCC/pull/1318)）。
    * 字符映射修正：「后 / 後」「松 / 鬆」「虱目魚」（[commit](https://github.com/BYVoid/OpenCC/commit/ab3fea0a80b85feb7fba1164e8d56e0ab2cd4002)）、「欲」「丰 / 豐」「划 / 劃」「游 / 遊」「托 / 託」（[#1213](https://github.com/BYVoid/OpenCC/pull/1213)）、各式果乾詞條（葡萄乾、芒果乾等）、「冻干 / 凍乾」「榴莲 / 榴槤」（[#1233](https://github.com/BYVoid/OpenCC/pull/1233)）、「皂」「並」（[#788](https://github.com/BYVoid/OpenCC/pull/788)）、「內」「潘岳白髮」「采椽不斲」（[#1232](https://github.com/BYVoid/OpenCC/pull/1232)）、「毀」「凌」「仇」（[#1243](https://github.com/BYVoid/OpenCC/pull/1243)）、「焕 / 煥」「冢」（[#1244](https://github.com/BYVoid/OpenCC/pull/1244)）、「丁當」相關詞組（[#1036](https://github.com/BYVoid/OpenCC/pull/1036)）、「想象 / 想像」（[#1035](https://github.com/BYVoid/OpenCC/pull/1035)）、「周济 / 周濟」（[#1251](https://github.com/BYVoid/OpenCC/pull/1251)）、「坏 / 坯」（[#1250](https://github.com/BYVoid/OpenCC/pull/1250)）、「沈 / 沉」（[#1293](https://github.com/BYVoid/OpenCC/pull/1293)）、「甦」（[#1310](https://github.com/BYVoid/OpenCC/pull/1310)）、「里」（[#1295](https://github.com/BYVoid/OpenCC/pull/1295)）、「麪 / 麵 / 面」（[#1297](https://github.com/BYVoid/OpenCC/pull/1297)）、「硷」（[#461](https://github.com/BYVoid/OpenCC/pull/461)）、「鏟 / 剷 / 铲」（[#1319](https://github.com/BYVoid/OpenCC/pull/1319)）、「幷 / 并」（[#1335](https://github.com/BYVoid/OpenCC/issues/1335)）、「市場行銷 / 市場營銷」（[#1327](https://github.com/BYVoid/OpenCC/pull/1327)）、「妝髮」（[#1326](https://github.com/BYVoid/OpenCC/pull/1326)）、「去干擾」（[#1325](https://github.com/BYVoid/OpenCC/pull/1325)）、「乾闥婆」（[#1307](https://github.com/BYVoid/OpenCC/pull/1307)）；調整「内卷 / 內捲」（[#1203](https://github.com/BYVoid/OpenCC/pull/1203)）、「信道 / 通道」（[#508](https://github.com/BYVoid/OpenCC/pull/508)）、「調色板 / 調色盤」（[#1288](https://github.com/BYVoid/OpenCC/pull/1288)）等單向轉換策略。
    * 新增 `HKVariantsPhrases` 與 `TWVariantsPhrases` 詞典，補充字級映射無法覆蓋的詞組例外（[#1247](https://github.com/BYVoid/OpenCC/pull/1247)）；新增 `s2hkp`、`hk2sp` 轉換模式（[#506](https://github.com/BYVoid/OpenCC/pull/506)）。
    * 日文字體：重構 `jp2t` / `t2jp` 詞典與配置（[#1302](https://github.com/BYVoid/OpenCC/pull/1302), [#1303](https://github.com/BYVoid/OpenCC/pull/1303)），與日本官方字表對齊（[#1315](https://github.com/BYVoid/OpenCC/pull/1315)），修正多組歷史性錯誤對照，補充 `予 -> 預/豫`、`弁 -> 辯/辨/瓣`、`連 -> 聯`、`值`、`姬` 等詞語對應（[#1265](https://github.com/BYVoid/OpenCC/pull/1265)）。
* **轉換行為調整**：
    * 繁轉簡模式預設不再輸出大部分現代平台無法顯示的「類推簡化字」（取消 894 餘組轉換）；如需舊行為，可通過 `--include-tofu-risk-dictionaries` 啟用（[#1234](https://github.com/BYVoid/OpenCC/pull/1234)）。
    * 所有內建配置均新增 CJK 相容表意文字正規化前處理（[#1358](https://github.com/BYVoid/OpenCC/pull/1358)）：Unicode U+F900–U+FAFF 區塊的字元在轉換前先映射至標準碼位，確保各移植實作行為一致。
    * 命令列工具改善：串流輸入處理（[#1226](https://github.com/BYVoid/OpenCC/pull/1226)）、保留未映射位元組與表意描述序列（[#1292](https://github.com/BYVoid/OpenCC/pull/1292)）、對異體選擇符發出警告、Windows UTF-8 路徑支援（[#1220](https://github.com/BYVoid/OpenCC/pull/1220)）。
* **配置文件新功能**（供自定義配置的使用者）：
    * 配置文件全面支援 JSONC（[#1280](https://github.com/BYVoid/OpenCC/pull/1280), [#1301](https://github.com/BYVoid/OpenCC/pull/1301)）；違反 schema 時會發出警告（[#1351](https://github.com/BYVoid/OpenCC/pull/1351)）。
    * 新增 inline dictionary：可在配置文件內直接嵌入詞條，無需另建 `.ocd2` 文件（[#1284](https://github.com/BYVoid/OpenCC/pull/1284)）；提供 `compile_to_inline_config.py` 工具將現有配置打包為自含格式（[#1300](https://github.com/BYVoid/OpenCC/pull/1300)）。詳見 [README.md 的「內聯字典」章節](https://github.com/BYVoid/OpenCC/blob/master/README.md#內聯字典inline-dictionary)。
    * 詞典組新增 `match_policy` 欄位：`short_circuit`（短路，原有行為）或 `union`（最長前綴合并，取所有子詞典中最長的命中結果）（[#1352](https://github.com/BYVoid/OpenCC/pull/1352), [#1353](https://github.com/BYVoid/OpenCC/pull/1353)）。
    * 新增 `normalization` 欄位，可在分詞與轉換前插入正規化步驟（[#1358](https://github.com/BYVoid/OpenCC/pull/1358)）。
    * 從本版本起發佈純 `json + txt` 詞典資源包，供其他工具鏈互操作（[#1312](https://github.com/BYVoid/OpenCC/pull/1312), [#1321](https://github.com/BYVoid/OpenCC/pull/1321), [#1322](https://github.com/BYVoid/OpenCC/pull/1322)）。
* **效能提升**：
    * 前綴匹配流程優化，典型長文本提升 40%-60%（[#1287](https://github.com/BYVoid/OpenCC/pull/1287)）；新增單詞典 fast-path（[#1331](https://github.com/BYVoid/OpenCC/pull/1331), [#1332](https://github.com/BYVoid/OpenCC/pull/1332)）；記憶體載入 `ocd2` 支援延遲重建 lexicon（[#1328](https://github.com/BYVoid/OpenCC/pull/1328)）；消除轉換熱路徑中的多餘字串拷貝（[#1345](https://github.com/BYVoid/OpenCC/pull/1345), [#1359](https://github.com/BYVoid/OpenCC/pull/1359)）。
    * 移除 `s2t`、`t2s`、`t2hk`、`t2tw`、`hk2t`、`jp2t`、`tw2t` 七種配置中冗餘的 mmseg 步驟（對這些單階段配置，mmseg 與轉換本身等價），消除一次額外輸入掃描（[#1362](https://github.com/BYVoid/OpenCC/pull/1362)）。
* **C++ API**：
    * `SimpleConverter`、`Segmentation`、`Conversion` 等主要介面新增 `std::string_view` 重載（[#1345](https://github.com/BYVoid/OpenCC/pull/1345), [#1359](https://github.com/BYVoid/OpenCC/pull/1359)）。
    * 修復 `Dict` 析構相關 ABI 未定義行為（[#1278](https://github.com/BYVoid/OpenCC/pull/1278)）、MARISA `FlatVector` 初始化問題（修正後 32-bit 與 64-bit 系統生成的 `.ocd2` 內容一致，[commit](https://github.com/BYVoid/OpenCC/commit/c3e07074e282b80083ed7734be51caecf9e7804f)）；更新 marisa-trie 至 0.3.1.bcr.2。
* **Python / Node.js**：
    * Python 套件 [OpenCC](https://pypi.org/project/OpenCC/) 新增 CLI 入口（`pip install` 後即可直接使用 `opencc` 指令）（[#1337](https://github.com/BYVoid/OpenCC/pull/1337)）。
    * npm 預編譯原生模組拆分為 scoped packages，安裝體積更小（[#1261](https://github.com/BYVoid/OpenCC/pull/1261), [#1263](https://github.com/BYVoid/OpenCC/pull/1263)）；更新 `cppjieba`（[#1320](https://github.com/BYVoid/OpenCC/pull/1320)）。

## Version 1.3.1

2026年5月9日

* **辭典與詞條更新**：新增古典音樂人物專有名詞譯名轉換近百名；修正「里、制、岩、净、炼、铲斗」、「拉链 / 拉鏈」、「以太网 / 乙太網路」等轉換問題；補充「汉坦病毒 / 漢他病毒」、「二𫫇英 / 戴奧辛」等新詞彙；修正所有「査 (U+67FB)」詞條為「查 (U+67E5)」；修正「乾安县」等錯誤轉換；並移除或調整「通过 / 透過」、「范式 / 正規化」、「程序 / 程式」、「缺省 / 預設」等若干會造成語義偏移的過度轉換。合計 `.txt` 詞典：共新增 294 行，刪除 72 行（[#1140](https://github.com/BYVoid/OpenCC/pull/1140), [#1155](https://github.com/BYVoid/OpenCC/pull/1155), [#1157](https://github.com/BYVoid/OpenCC/pull/1157), [#1162](https://github.com/BYVoid/OpenCC/pull/1162), [#1168](https://github.com/BYVoid/OpenCC/pull/1168), [#1171](https://github.com/BYVoid/OpenCC/pull/1171), [#1173](https://github.com/BYVoid/OpenCC/pull/1173), [#1177](https://github.com/BYVoid/OpenCC/pull/1177), [#1181](https://github.com/BYVoid/OpenCC/pull/1181), [#1184](https://github.com/BYVoid/OpenCC/pull/1184), [#1185](https://github.com/BYVoid/OpenCC/pull/1185), [#1189](https://github.com/BYVoid/OpenCC/pull/1189), [#1191](https://github.com/BYVoid/OpenCC/pull/1191)）。
* **`s2twp` 分詞修復**：修復因分詞邊界導致 `s2twp` 未能正確轉換的 33 個詞組，例如「云计算 / 雲端計算」等（[#1193](https://github.com/BYVoid/OpenCC/pull/1193)）。
* **轉換算法更新**：重寫核心轉換算法，典型長文本基準測試中轉換速度提升 3-10 倍（[#1130](https://github.com/BYVoid/OpenCC/pull/1130), [#1156](https://github.com/BYVoid/OpenCC/pull/1156), [#1180](https://github.com/BYVoid/OpenCC/pull/1180)）。
* **npm package 全新升級**：Node.js 原生模組改用 N-API 重寫，提升跨 Node.js 版本兼容性；新增可選 Jieba 分詞支持，提供簡易 `opencc` CLI，並更新預編譯包打包方式（[#1142](https://github.com/BYVoid/OpenCC/pull/1142), [#1145](https://github.com/BYVoid/OpenCC/pull/1145), [#1149](https://github.com/BYVoid/OpenCC/pull/1149), [#1151](https://github.com/BYVoid/OpenCC/pull/1151), [#1153](https://github.com/BYVoid/OpenCC/pull/1153), [#1195](https://github.com/BYVoid/OpenCC/pull/1195)）。
* **構建與發佈基礎設施更新**：新增 golden test 覆蓋轉換結果回歸；加固 Python PyPI release 流程；更新 Node.js、Bazel 與跨平台 prebuild/release 流程（[#1043](https://github.com/BYVoid/OpenCC/pull/1043), [#1128](https://github.com/BYVoid/OpenCC/pull/1128), [#1158](https://github.com/BYVoid/OpenCC/pull/1158), [#1159](https://github.com/BYVoid/OpenCC/pull/1159), [#1166](https://github.com/BYVoid/OpenCC/pull/1166), [#1169](https://github.com/BYVoid/OpenCC/pull/1169), [#1190](https://github.com/BYVoid/OpenCC/pull/1190)）。
* **文檔更新**：新增「[OpenCC 設計思想](https://github.com/BYVoid/OpenCC/blob/master/DESIGN_PRINCIPLES.md)」等說明文件，明確「避免過度轉換」等詞典維護原則（[#1152](https://github.com/BYVoid/OpenCC/pull/1152)）。
* **下游協作**：與 [`opencc-js`](https://www.npmjs.com/package/opencc-js) 建立數據同步協作機制，提升 OpenCC 不同實作間的一致性。

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
