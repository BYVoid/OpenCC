declare class OpenCC {
  constructor(config: string);
  version(): string;
  generateDict(inputFileName: string, outputFileName: string, formatFrom: string, formatTo: string): void;
  convert(input: string, callback: (err: string, convertedText: string) => void): string;
  convertSync(input: string): string;
  convertPromise(input: string): Promise<string>;
}
export { OpenCC };
