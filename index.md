---
layout: default
title: OpenCC 開放中文轉換
heading: 開放中文轉換
nav_title: Home 首頁
nav_order: 1
landing: true
description: Open Chinese Convert is an open source library and toolchain for Traditional Chinese, Simplified Chinese, Japanese Kanji, and regional phrase conversion.
---

<section class="hero">
  <div class="hero-copy">
    <div class="hero-badges" aria-label="Project status">
      <span class="badge success">Stable 1.3.1 · 穩定版</span>
      <span class="badge neutral">C, C++, Node.js, Python</span>
      <span class="badge info">Open source · 開源</span>
    </div>
    <h1>Open Chinese Convert <span class="zh-title">開放中文轉換</span></h1>
    <p class="lead">
      A precise conversion engine for Traditional Chinese, Simplified Chinese, Japanese Kanji, variants, and regional wording across Mainland China, Taiwan, and Hong Kong.
      <span class="zh-copy">精準處理繁體、簡體、日本新字體、異體字，以及中國大陸、台灣、香港地區用詞差異的中文轉換引擎。</span>
    </p>
    <div class="hero-actions">
      <a class="button primary" href="https://opencc.js.org/converter?config=s2t">Try online / 在線轉換</a>
      <a class="button secondary" href="https://github.com/BYVoid/OpenCC/releases">Download / 下載</a>
    </div>
  </div>
  <div class="hero-panel" aria-label="Conversion example">
    <div class="panel-header">
      <span>Convert with OpenCC <span class="zh-inline">使用 OpenCC 轉換</span></span>
      <span class="badge neutral">s2twp.json</span>
    </div>
    <div class="conversion-card">
      <p class="label">Input / 輸入</p>
      <p class="conversion-text" data-conversion-input>汉字、鼠标、数据库连接池</p>
    </div>
    <div class="conversion-card output">
      <p class="label">Output / 輸出</p>
      <p class="conversion-text" data-conversion-output>漢字、滑鼠、資料庫連線池</p>
    </div>
    <pre><code>npm install opencc
opencc -c s2twp.json -i input.txt -o output.txt</code></pre>
  </div>
</section>

<script>
  (function () {
    var examples = [
      ["汉字、鼠标、数据库连接池", "漢字、滑鼠、資料庫連線池"],
      ["软件工程师正在调试日志", "軟體工程師正在偵錯日誌"],
      ["云计算平台和网络服务", "雲端運算平台和網路服務"],
      ["打印机驱动程序已经更新", "印表機驅動程式已經更新"],
      ["用户界面支持繁简转换", "使用者介面支援繁簡轉換"],
      ["他打开文件夹查看项目", "他開啟資料夾檢視專案"],
      ["这条短信包含链接地址", "這則簡訊包含連結位址"],
      ["服务器返回默认设置", "伺服器回傳預設設定"],
      ["后端接口处理批量请求", "後端介面處理批次請求"],
      ["硬盘空间和内存占用", "硬碟空間和記憶體佔用"]
    ];
    var input = document.querySelector("[data-conversion-input]");
    var output = document.querySelector("[data-conversion-output]");
    if (!input || !output) return;

    var index = 0;
    window.setInterval(function () {
      index = (index + 1) % examples.length;
      input.classList.add("is-changing");
      output.classList.add("is-changing");

      window.setTimeout(function () {
        input.textContent = examples[index][0];
        output.textContent = examples[index][1];
        input.classList.remove("is-changing");
        output.classList.remove("is-changing");
      }, 220);
    }, 2600);
  })();
</script>

<section class="status-strip" aria-label="Build status">
  <a href="https://github.com/BYVoid/OpenCC/actions/workflows/cmake.yml"><img src="https://github.com/BYVoid/OpenCC/actions/workflows/cmake.yml/badge.svg" alt="CMake"></a>
  <a href="https://github.com/BYVoid/OpenCC/actions/workflows/bazel.yml"><img src="https://github.com/BYVoid/OpenCC/actions/workflows/bazel.yml/badge.svg" alt="Bazel"></a>
  <a href="https://github.com/BYVoid/OpenCC/actions/workflows/msvc.yml"><img src="https://github.com/BYVoid/OpenCC/actions/workflows/msvc.yml/badge.svg" alt="MSVC"></a>
  <a href="https://github.com/BYVoid/OpenCC/actions/workflows/nodejs.yml"><img src="https://github.com/BYVoid/OpenCC/actions/workflows/nodejs.yml/badge.svg" alt="Node.js CI"></a>
  <a href="https://github.com/BYVoid/OpenCC/actions/workflows/python.yml"><img src="https://github.com/BYVoid/OpenCC/actions/workflows/python.yml/badge.svg" alt="Python CI"></a>
  <a href="https://ci.appveyor.com/project/Carbo/OpenCC"><img src="https://img.shields.io/appveyor/ci/Carbo/OpenCC.svg" alt="AppVeyor"></a>
  <a href="https://www.npmjs.com/package/opencc"><img src="https://img.shields.io/npm/v/opencc" alt="npm package"></a>
  <a href="https://pypi.org/project/opencc/"><img src="https://img.shields.io/pypi/v/opencc.svg" alt="PyPI version"></a>
  <a href="https://packages.debian.org/search?keywords=opencc"><img src="https://img.shields.io/debian/v/opencc/unstable" alt="Debian package"></a>
</section>

<section class="section-block">
  <div class="section-heading">
    <p class="eyebrow">Install / 安裝</p>
    <h2>Use it from your platform of choice <span class="zh-title">選擇你慣用的平台安裝</span></h2>
  </div>
  <div class="install-grid">
    <article class="install-card">
      <span class="badge neutral">Node.js</span>
      <h3>Package and CLI <span class="zh-title">套件與命令列工具</span></h3>
      <pre><code>npm install opencc opencc-jieba
npm install -g opencc opencc-jieba</code></pre>
    </article>
    <article class="install-card">
      <span class="badge neutral">Python</span>
      <h3>Python package <span class="zh-title">Python 套件</span></h3>
      <pre><code>pip install opencc</code></pre>
    </article>
    <article class="install-card">
      <span class="badge neutral">macOS</span>
      <h3>Homebrew</h3>
      <pre><code>brew install opencc</code></pre>
    </article>
    <article class="install-card">
      <span class="badge neutral">Windows</span>
      <h3>WinGet</h3>
      <pre><code>winget install opencc</code></pre>
    </article>
  </div>
</section>

<section class="section-block">
  <div class="section-heading">
    <p class="eyebrow">Conversion model / 轉換模型</p>
    <h2>Pure rule-based Chinese conversion without LLM dependency <span class="zh-title">不依賴大模型的純規則中文轉換</span></h2>
  </div>
  <div class="feature-list">
    <div>
      <h3>Phrase-level conversion <span class="zh-title">詞彙級轉換</span></h3>
      <p>Handles vocabulary and idiom differences instead of only replacing individual characters.<span class="zh-copy">處理詞彙與慣用語差異，而不只是逐字替換。</span></p>
    </div>
    <div>
      <h3>Variant-aware rules <span class="zh-title">異體字規則</span></h3>
      <p>Separates one-to-many Traditional Chinese mappings from variant character mappings.<span class="zh-copy">嚴格區分「一簡對多繁」與異體字映射。</span></p>
    </div>
    <div>
      <h3>Regional dictionaries <span class="zh-title">地區詞庫</span></h3>
      <p>Supports Mainland China, Taiwan, Hong Kong, and Japanese Shinjitai conversion profiles.<span class="zh-copy">支援中國大陸、台灣、香港與日本新字體轉換配置。</span></p>
    </div>
    <div>
      <h3>Extensible data <span class="zh-title">可擴展資料</span></h3>
      <p>Dictionary data is separate from the library so teams can import, replace, and extend rules.<span class="zh-copy">詞庫與函式庫分離，方便導入、替換與擴展規則。</span></p>
    </div>
    <div>
      <h3>Open code, dictionaries, and configs <span class="zh-title">代碼、字典、配置完全開放</span></h3>
      <p>All core assets are transparent and editable, making conversion behavior auditable and reproducible.<span class="zh-copy">核心代碼、轉換字典與配置文件全部開放，轉換行為可審查、可復現、可自訂。</span></p>
    </div>
  </div>
</section>

<section class="section-block split-section">
  <div>
    <p class="eyebrow">Documentation / 文檔</p>
    <h2>API references and release artifacts <span class="zh-title">API 參考與發佈檔案</span></h2>
    <p>Read the generated API documentation, browse historical versions, or inspect the source on GitHub.<span class="zh-copy">查看自動生成的 API 文檔、歷史版本，或在 GitHub 檢視原始碼。</span></p>
  </div>
  <div class="link-list">
    <a href="/1.3.1/">Latest API docs / 最新 API 文檔 <span>1.3.1</span></a>
    <a href="/docs/">Historical docs / 歷史文檔 <span>All versions</span></a>
    <a href="https://github.com/BYVoid/OpenCC">Source code / 原始碼 <span>GitHub</span></a>
    <a href="https://t.me/open_chinese_convert">Discussion / 討論 <span>Telegram</span></a>
  </div>
</section>
