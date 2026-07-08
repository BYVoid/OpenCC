type OpenCCDict = OpenCCFileDict | OpenCCInlineDict | OpenCCGroupDict;
type OpenCCSegmentation = OpenCCMmsegSegmentation | OpenCCPluginSegmentation;
type OpenCCPluginSegmentation = {
  type: string;
  resources?: Record<string, string>;
  [key: string]: string | Record<string, string> | undefined;
};

interface OpenCCConfig {
  name: string;
  normalization?: [OpenCCConversion, ...OpenCCConversion[]];
  segmentation?: OpenCCSegmentation;
  conversion_chain: [OpenCCConversion, ...OpenCCConversion[]];
}
interface OpenCCConversion {
  dict: OpenCCDict;
}
interface OpenCCFileDict {
  type: "text" | "ocd" | "ocd2";
  file: string;
  may_output_tofu?: boolean;
}
interface OpenCCInlineDict {
  type: "inline";
  entries: {
    [k: string]: string;
  };
}
interface OpenCCGroupDict {
  type: "group";
  match_policy: "short_circuit" | "union";
  dicts: [OpenCCDict, ...OpenCCDict[]];
  may_output_tofu?: boolean;
}
interface OpenCCMmsegSegmentation {
  type: "mmseg";
  dict: OpenCCDict;
}

interface OpenCCOptions {
  includeTofuRiskDictionaries?: boolean;
  configDirectory?: string;
  resourceZip?: string;
}

declare class OpenCC {
  constructor(config?: string | OpenCCConfig, options?: OpenCCOptions);
  static readonly version: string;
  static fromConfig(config: OpenCCConfig, options?: OpenCCOptions): OpenCC;
  static generateDict(inputFileName: string, outputFileName: string, formatFrom: string, formatTo: string): void;
  convert(input: string, callback: (err: string | undefined, convertedText: string) => void): void;
  convertSync(input: string): string;
  convertPromise(input: string): Promise<string>;
}
export { OpenCC, OpenCCConfig, OpenCCOptions };
export default OpenCC;
