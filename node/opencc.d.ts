type OpenCCConfig = Record<string, unknown>;

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
