/**
 * OpenCC WASM - WebAssembly backend for OpenCC
 *
 * @packageDocumentation
 */

/**
 * Options for creating a converter
 */
export interface ConverterOptions {
  /**
   * Source locale: 'cn' (Simplified Chinese), 'tw' (Traditional Taiwan),
   * 'hk' (Traditional Hong Kong), 't' (Traditional), 'jp' (Japanese)
   */
  from?: string;

  /**
   * Target locale: 'cn' (Simplified Chinese), 'tw' (Traditional Taiwan),
   * 'hk' (Traditional Hong Kong), 't' (Traditional), 'jp' (Japanese)
   */
  to?: string;

  /**
   * Config file name (e.g., 's2t.json', 't2s.json')
   * If specified, 'from' and 'to' will be ignored
   */
  config?: string;
}

/**
 * Async converter function that transforms text
 */
export type ConverterFunction = (text: string) => Promise<string>;

/**
 * Synchronous custom converter function (for custom dictionaries)
 */
export type CustomConverterFunction = (text: string) => string;

/**
 * Custom dictionary entry: [source, target]
 */
export type DictEntry = [string, string];

/**
 * Custom dictionary: array of entries or pipe-separated string
 */
export type CustomDict = DictEntry[] | string;

/**
 * Locale mappings
 */
export interface LocaleMap {
  cn: string;
  tw: string;
  hk: string;
  jp: string;
  t: string;
}

/**
 * OpenCC namespace with all conversion functions
 */
export interface OpenCCNamespace {
  /**
   * Create a converter with the given options
   *
   * @example
   * ```typescript
   * const converter = OpenCC.Converter({ from: 'cn', to: 'tw' });
   * const result = await converter('简体中文');
   * ```
   */
  Converter(opts: ConverterOptions): ConverterFunction;

  /**
   * Create a custom converter with user-defined dictionary
   *
   * @param dict - Array of [source, target] pairs or pipe-separated string
   *
   * @example
   * ```typescript
   * const custom = OpenCC.CustomConverter([
   *   ['"', '「'],
   *   ['"', '」'],
   * ]);
   * const result = custom('He said "hello"');
   * ```
   */
  CustomConverter(dict: CustomDict): CustomConverterFunction;

  /**
   * Create a converter with additional custom dictionaries
   *
   * @param fromLocale - Source locale
   * @param toLocale - Target locale
   * @param extraDicts - Additional custom dictionaries to apply after conversion
   *
   * @example
   * ```typescript
   * const converter = OpenCC.ConverterFactory('cn', 'tw', [
   *   [['"', '「'], ['"', '」']]
   * ]);
   * const result = await converter('简体中文 "test"');
   * ```
   */
  ConverterFactory(
    fromLocale: string,
    toLocale: string,
    extraDicts?: CustomDict[]
  ): ConverterFunction;

  /**
   * Locale constants for 'from' and 'to' options
   */
  Locale: {
    from: LocaleMap;
    to: LocaleMap;
  };
}

/**
 * OpenCC main export
 */
export const OpenCC: OpenCCNamespace;

/**
 * Default export
 */
export default OpenCC;
