declare class OpenCC {
  constructor(config?: string);
  static readonly version: string;
  static generateDict(inputFileName: string, outputFileName: string, formatFrom: string, formatTo: string): void;
  convert(input: string, callback: (err: string | undefined, convertedText: string) => void): void;
  convertSync(input: string): string;
  convertPromise(input: string): Promise<string>;
}

declare namespace OpenCC {
  export { OpenCC };
}

export = OpenCC;
