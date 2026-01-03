// Lightweight OpenCC WASM wrapper with opencc-js compatible API.
// 假定包内目录结构：
//  - dist/opencc-wasm.js|.wasm
//  - data/config/*.json
//  - data/dict/*.ocd2

// Optional Node helpers (guarded for browser)
let fsMod = null;
let fileURLToPathFn = null;
const hasNode = typeof process !== "undefined" && !!process.versions?.node;
if (hasNode) {
  fsMod = await import("node:fs");
  ({ fileURLToPath: fileURLToPathFn } = await import("node:url"));
}

const BASE_URL = new URL("../", import.meta.url);

const readFileText = (url) => {
  if (!fsMod || !fileURLToPathFn) throw new Error("fs not available in this environment");
  const path = fileURLToPathFn(url);
  return fsMod.readFileSync(path, "utf-8");
};

const readFileBuffer = (url) => {
  if (!fsMod || !fileURLToPathFn) throw new Error("fs not available in this environment");
  const path = fileURLToPathFn(url);
  return fsMod.readFileSync(path);
};

// 预设映射：from -> to -> config 文件名
const CONFIG_MAP = {
  cn: { t: "s2t.json", tw: "s2tw.json", hk: "s2hk.json", cn: null },
  tw: { cn: "tw2s.json", t: "tw2t.json", tw: null },
  hk: { cn: "hk2s.json", t: "hk2t.json", hk: null },
  t: { cn: "t2s.json", tw: "t2tw.json", hk: "t2hk.json", jp: "t2jp.json", t: null },
  jp: { t: "jp2t.json" },
};

// 缓存已加载的配置/字典与打开的句柄，避免重复加载和重复构建
const loadedConfigs = new Set();
const loadedDicts = new Set();
const handles = new Map();
let modulePromise = null;
let api = null;

async function getModule() {
  if (modulePromise) return modulePromise;

  // 1) 先确定包根目录（一定要以 / 结尾）
  const pkgBase = new URL("./", import.meta.url);
  // 如果这段代码在 HTML inline script 里，没有 import.meta.url，那就用绝对路径：
  // const pkgBase = new URL("/vendor/opencc-wasm/", window.location.origin);

  // 2) import glue (from build/ for testing/development)
  const glueUrl = new URL("./opencc-wasm.js", import.meta.url);

  const { default: create } = await import(glueUrl.href);

  // 3) locateFile 必须相对 pkgBase，而不是 glueUrl
  modulePromise = create({
    locateFile: (p) => new URL(`../${p}`, import.meta.url).href
  });

  return modulePromise;
}

async function getApi() {
  const mod = await getModule();
  if (!api) {
    api = {
      create: mod.cwrap("opencc_create", "number", ["string"]),
      convert: mod.cwrap("opencc_convert", "string", ["number", "string"]),
      destroy: mod.cwrap("opencc_destroy", null, ["number"]),
    };
  }
  return { mod, api };
}

function collectOcd2Files(node, acc) {
  if (!node || typeof node !== "object") return;
  if (node.type === "ocd2" && node.file) acc.add(node.file);
  if (node.type === "group" && Array.isArray(node.dicts)) {
    node.dicts.forEach((d) => collectOcd2Files(d, acc));
  }
}

async function fetchText(url) {
  if (url.protocol === "file:") {
    return readFileText(url);
  }
  const resp = await fetch(url.href);
  if (!resp.ok) throw new Error(`Fetch ${url} failed: ${resp.status}`);
  return resp.text();
}

async function fetchBuffer(url) {
  if (url.protocol === "file:") {
    return new Uint8Array(readFileBuffer(url));
  }
  const resp = await fetch(url.href);
  if (!resp.ok) throw new Error(`Fetch ${url} failed: ${resp.status}`);
  return new Uint8Array(await resp.arrayBuffer());
}

async function ensureConfig(configName) {
  if (handles.has(configName)) return handles.get(configName);
  const { mod, api: apiFns } = await getApi();
  mod.FS.mkdirTree("/data/config");
  mod.FS.mkdirTree("/data/dict");
  const cfgUrl = new URL(`./data/config/${configName}`, BASE_URL);
  const cfgJson = JSON.parse(await fetchText(cfgUrl));

  const dicts = new Set();
  collectOcd2Files(cfgJson.segmentation?.dict, dicts);
  if (Array.isArray(cfgJson.conversion_chain)) {
    cfgJson.conversion_chain.forEach((item) => collectOcd2Files(item?.dict, dicts));
  }
  for (const file of dicts) {
    if (loadedDicts.has(file)) continue; // 避免重复加载同一字典
    const dictUrl = new URL(`./data/dict/${file}`, BASE_URL);
    const buf = await fetchBuffer(dictUrl);
    mod.FS.writeFile(`/data/dict/${file}`, buf);
    loadedDicts.add(file);
  }
  // 重写配置中的 ocd2 路径到 /data/dict 下
  const patchPaths = (node) => {
    if (!node || typeof node !== "object") return;
    if (node.type === "ocd2" && node.file) {
      node.file = `/data/dict/${node.file}`;
    }
    if (node.type === "group" && Array.isArray(node.dicts)) {
      node.dicts.forEach(patchPaths);
    }
  };
  patchPaths(cfgJson.segmentation?.dict);
  if (Array.isArray(cfgJson.conversion_chain)) {
    cfgJson.conversion_chain.forEach((item) => patchPaths(item?.dict));
  }
  mod.FS.writeFile(`/data/config/${configName}`, JSON.stringify(cfgJson));
  loadedConfigs.add(configName);

  const handle = apiFns.create(`/data/config/${configName}`);
  if (!handle || handle < 0) {
    throw new Error(`opencc_create failed for ${configName}`);
  }
  handles.set(configName, handle);
  return handle;
}

function resolveConfig(from, to) {
  const f = (from || "").toLowerCase();
  const t = (to || "").toLowerCase();
  const m = CONFIG_MAP[f];
  if (!m || !(t in m)) {
    throw new Error(`Unsupported conversion from '${from}' to '${to}'`);
  }
  return m[t]; // may be null for identical locale (no-op)
}

function createConverter({ from, to, config }) {
  const configName = config || resolveConfig(from, to);
  return async (text) => {
    if (configName === null) return text; // no-op
    const handle = await ensureConfig(configName);
    const { api: apiFns } = await getApi();
    return apiFns.convert(handle, text);
  };
}

function CustomConverter(dictOrString) {
  let pairs = [];
  if (typeof dictOrString === "string") {
    pairs = dictOrString
      .split("|")
      .map((seg) => seg.trim())
      .filter(Boolean)
      .map((seg) => seg.split(/\s+/))
      .filter((arr) => arr.length >= 2)
      .map(([a, b]) => [a, b]);
  } else if (Array.isArray(dictOrString)) {
    pairs = dictOrString;
  }
  // 按键长度降序，保证长词优先
  pairs.sort((a, b) => b[0].length - a[0].length);
  return (text) => {
    let out = text;
    for (const [src, dst] of pairs) {
      out = out.split(src).join(dst);
    }
    return out;
  };
}

function ConverterFactory(fromLocale, toLocale, extraDicts = []) {
  const conv = createConverter({ from: fromLocale, to: toLocale });
  const extras = extraDicts.map((d) => CustomConverter(d));
  return async (text) => {
    let result = await conv(text);
    extras.forEach((fn) => {
      result = fn(result);
    });
    return result;
  };
}

export const OpenCC = {
  Converter(opts) {
    const fn = createConverter(opts);
    return (text) => fn(text);
  },
  CustomConverter,
  ConverterFactory,
  Locale: {
    from: { cn: "cn", tw: "t", hk: "hk", jp: "jp", t: "t" },
    to: { cn: "cn", tw: "tw", hk: "hk", jp: "jp", t: "t" },
  },
};

export default OpenCC;
