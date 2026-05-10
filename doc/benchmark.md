# Benchmark

Results from Github CI, commit ID 2f56960 (ver.1.3.1), 2026-05-09, CMake macos-latest:

```
2026-05-09T21:06:02+00:00
Running /Users/runner/work/OpenCC/OpenCC/build/perf/src/benchmark/performance
Run on (3 X 27.6602 MHz CPU s)
CPU Caches:
  L1 Data 128 KiB
  L1 Instruction 192 KiB
  L2 Unified 12288 KiB (x3)
Load Average: 1.59, 1.40, 1.74

[Initialization]
------------------------------------------------------------------------------------------------------
Benchmark                                                            Time             CPU   Iterations
------------------------------------------------------------------------------------------------------
BM_Initialization/hk2s/ocd2                                        928   us        926   us        752
BM_Initialization/hk2s/text_json                                  2706   us       2695   us        285
BM_Initialization/hk2t/ocd2                                        125   us        125   us       5768
BM_Initialization/hk2t/text_json                                   296   us        296   us       2312
BM_Initialization/jp2t/ocd2                                        189   us        189   us       3799
BM_Initialization/jp2t/text_json                                   480   us        480   us       1403
BM_Initialization/s2hk/ocd2                                      25208   us      25199   us         27
BM_Initialization/s2hk/text_json                                 93256   us      93242   us          7
BM_Initialization/s2t/ocd2                                       26775   us      26771   us         27
BM_Initialization/s2t/text_json                                  90910   us      90895   us          7
BM_Initialization/s2tw/ocd2                                      26382   us      26333   us         27
BM_Initialization/s2tw/text_json                                 91633   us      91628   us          7
BM_Initialization/s2twp/ocd2                                     25375   us      25374   us         28
BM_Initialization/s2twp/text_json                                92903   us      92873   us          8
BM_Initialization/t2hk/ocd2                                       50.1   us       50.1   us      14110
BM_Initialization/t2hk/text_json                                   104   us        104   us       6567
BM_Initialization/t2jp/ocd2                                       91.2   us       91.2   us       7733
BM_Initialization/t2jp/text_json                                   336   us        336   us       2097
BM_Initialization/t2s/ocd2                                         768   us        768   us        914
BM_Initialization/t2s/text_json                                   2251   us       2209   us        337
BM_Initialization/t2tw/ocd2                                       43.7   us       43.7   us      16175
BM_Initialization/t2tw/text_json                                  79.2   us       79.2   us       8844
BM_Initialization/tw2s/ocd2                                        849   us        849   us        837
BM_Initialization/tw2s/text_json                                  2189   us       2189   us        315
BM_Initialization/tw2sp/ocd2                                      1118   us       1118   us        663
BM_Initialization/tw2sp/text_json                                 2836   us       2836   us        247
BM_Initialization/tw2t/ocd2                                       76.7   us       76.7   us       9165
BM_Initialization/tw2t/text_json                                   152   us        152   us       4579
BM_Initialization/s2twp_jieba/ocd2                              355989   us     355956   us          2
BM_Initialization/s2twp_jieba/text_json                         381008   us     380786   us          2

[Convert Long Text]
-----------------------------------------------------------------------------------------------------------------------
Benchmark                                                            Time             CPU   Iterations       Throughput
-----------------------------------------------------------------------------------------------------------------------
BM_ConvertLongText/s2t/ocd2                                       73.5   ms       72.8   ms         11       25.63 MB/s
BM_ConvertLongText/s2t/text_json                                  62.6   ms       62.6   ms         11       29.79 MB/s
BM_ConvertLongText/s2twp/ocd2                                     99.2   ms       99.2   ms          7       18.82 MB/s
BM_ConvertLongText/s2twp/text_json                                99.0   ms       99.0   ms          7       18.86 MB/s
BM_ConvertLongText/s2twp_jieba/ocd2                                211   ms        211   ms          3        8.84 MB/s
BM_ConvertLongText/s2twp_jieba/text_json                           201   ms        201   ms          3        9.29 MB/s

[Convert]
-----------------------------------------------------------------------------------------------------------------------
Benchmark                                                            Time             CPU   Iterations       Throughput
-----------------------------------------------------------------------------------------------------------------------
BM_Convert/s2t/ocd2/100                                          0.094   ms      0.094   ms       7443       44.50 MB/s
BM_Convert/s2t/ocd2/1000                                         0.984   ms      0.984   ms        721       43.60 MB/s
BM_Convert/s2t/ocd2/10000                                         10.1   ms       10.1   ms         70       43.58 MB/s
BM_Convert/s2t/ocd2/100000                                         107   ms        107   ms          7       42.11 MB/s
BM_Convert/s2t/text_json/100                                     0.096   ms      0.096   ms       6860       43.71 MB/s
BM_Convert/s2t/text_json/1000                                     1.05   ms       1.05   ms        721       40.82 MB/s
BM_Convert/s2t/text_json/10000                                    10.4   ms       10.4   ms         65       42.40 MB/s
BM_Convert/s2t/text_json/100000                                    110   ms        110   ms          7       40.82 MB/s
BM_Convert/s2twp/ocd2/100                                        0.178   ms      0.178   ms       3950       23.55 MB/s
BM_Convert/s2twp/ocd2/1000                                        1.92   ms       1.92   ms        375       22.39 MB/s
BM_Convert/s2twp/ocd2/10000                                       19.2   ms       19.2   ms         37       22.88 MB/s
BM_Convert/s2twp/ocd2/100000                                       204   ms        204   ms          3       21.97 MB/s
BM_Convert/s2twp/text_json/100                                   0.178   ms      0.178   ms       3943       23.51 MB/s
BM_Convert/s2twp/text_json/1000                                   2.00   ms       2.00   ms        380       21.47 MB/s
BM_Convert/s2twp/text_json/10000                                  19.4   ms       19.4   ms         36       22.63 MB/s
BM_Convert/s2twp/text_json/100000                                  207   ms        206   ms          4       21.79 MB/s
BM_Convert/s2twp_jieba/ocd2/100                                  0.368   ms      0.367   ms       1892       11.40 MB/s
BM_Convert/s2twp_jieba/ocd2/1000                                  4.48   ms       4.48   ms        180        9.57 MB/s
BM_Convert/s2twp_jieba/ocd2/10000                                 47.2   ms       47.0   ms         17        9.34 MB/s
BM_Convert/s2twp_jieba/ocd2/100000                                 521   ms        520   ms          1        8.63 MB/s
BM_Convert/s2twp_jieba/text_json/100                             0.396   ms      0.396   ms       1922       10.59 MB/s
BM_Convert/s2twp_jieba/text_json/1000                             3.86   ms       3.86   ms        179       11.11 MB/s
BM_Convert/s2twp_jieba/text_json/10000                            44.2   ms       44.2   ms         17        9.94 MB/s
BM_Convert/s2twp_jieba/text_json/100000                            446   ms        446   ms          2       10.07 MB/s

[Command Line Long Text]
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                            Time             CPU   Iterations       Throughput       LoadMs    ConvertMs      WriteMs      TotalMs
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
BM_CommandLineLongText/s2t/ocd2/min_time:0.010                     131   ms      0.475   ms          1       17.05 MB/s     29.99 ms     72.97 ms      0.89 ms    109.45 ms
BM_CommandLineLongText/s2t/text_json/min_time:0.010                190   ms      0.370   ms          1       10.85 MB/s    103.47 ms     66.42 ms      1.05 ms    171.97 ms
BM_CommandLineLongText/s2twp/ocd2/min_time:0.010                   177   ms       1.03   ms          1       12.28 MB/s     40.43 ms    110.09 ms      0.32 ms    151.96 ms
BM_CommandLineLongText/s2twp/text_json/min_time:0.010              250   ms      0.363   ms          1        8.31 MB/s    110.77 ms    111.00 ms      1.17 ms    224.72 ms
BM_CommandLineLongText/s2twp_jieba/ocd2/min_time:0.010            1107   ms      0.397   ms          1        1.77 MB/s    746.43 ms    305.44 ms      2.10 ms   1056.00 ms
BM_CommandLineLongText/s2twp_jieba/text_json/min_time:0.010        859   ms       1.05   ms          1        2.39 MB/s    404.82 ms    374.96 ms      0.47 ms    781.91 ms

[Command Line]
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                            Time             CPU   Iterations       Throughput       LoadMs    ConvertMs      WriteMs      TotalMs
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
BM_CommandLine/s2t/ocd2/100/min_time:0.010                        58.0   ms      0.365   ms          1        0.11 MB/s     37.81 ms      0.13 ms      0.03 ms     38.38 ms
BM_CommandLine/s2t/ocd2/1000/min_time:0.010                       57.1   ms      0.313   ms          1        1.16 MB/s     35.12 ms      1.07 ms      0.06 ms     36.89 ms
BM_CommandLine/s2t/ocd2/10000/min_time:0.010                      79.0   ms       1.18   ms          1        7.96 MB/s     42.06 ms     11.14 ms      0.40 ms     55.16 ms
BM_CommandLine/s2t/ocd2/100000/min_time:0.010                      185   ms      0.494   ms          1       27.16 MB/s     29.60 ms    126.85 ms      0.99 ms    165.28 ms
BM_CommandLine/s2t/text_json/100/min_time:0.010                    110   ms      0.395   ms          1        0.05 MB/s     90.65 ms      0.11 ms      0.03 ms     91.04 ms
BM_CommandLine/s2t/text_json/1000/min_time:0.010                   123   ms      0.304   ms          1        0.41 MB/s    102.99 ms      1.03 ms      0.03 ms    104.43 ms
BM_CommandLine/s2t/text_json/10000/min_time:0.010                  122   ms      0.320   ms          1        4.30 MB/s     91.25 ms     10.33 ms      0.08 ms    101.99 ms
BM_CommandLine/s2t/text_json/100000/min_time:0.010                 226   ms      0.314   ms          1       21.71 MB/s     90.06 ms    108.41 ms      0.75 ms    206.75 ms
BM_CommandLine/s2twp/ocd2/100/min_time:0.010                      43.5   ms      0.380   ms         10        0.16 MB/s     26.35 ms      0.21 ms      0.02 ms     26.85 ms
BM_CommandLine/s2twp/ocd2/1000/min_time:0.010                     43.9   ms      0.303   ms         10        1.54 MB/s     25.66 ms      1.93 ms      0.03 ms     27.88 ms
BM_CommandLine/s2twp/ocd2/10000/min_time:0.010                    65.2   ms      0.530   ms          1        9.27 MB/s     26.61 ms     20.32 ms      0.07 ms     47.35 ms
BM_CommandLine/s2twp/ocd2/100000/min_time:0.010                    254   ms      0.396   ms          1       19.05 MB/s     25.10 ms    203.16 ms      0.60 ms    235.63 ms
BM_CommandLine/s2twp/text_json/100/min_time:0.010                  110   ms      0.873   ms          1        0.05 MB/s     90.88 ms      0.20 ms      0.03 ms     91.35 ms
BM_CommandLine/s2twp/text_json/1000/min_time:0.010                 111   ms      0.308   ms          1        0.46 MB/s     90.28 ms      1.91 ms      0.04 ms     92.46 ms
BM_CommandLine/s2twp/text_json/10000/min_time:0.010                129   ms      0.316   ms          1        3.98 MB/s     89.62 ms     20.23 ms      0.09 ms    110.28 ms
BM_CommandLine/s2twp/text_json/100000/min_time:0.010               323   ms      0.381   ms          1       14.79 MB/s     91.20 ms    203.96 ms      0.65 ms    303.43 ms
BM_CommandLine/s2twp_jieba/ocd2/100/min_time:0.010                 383   ms      0.757   ms          1        0.01 MB/s    328.65 ms      0.51 ms      0.04 ms    329.54 ms
BM_CommandLine/s2twp_jieba/ocd2/1000/min_time:0.010                425   ms      0.383   ms          1        0.12 MB/s    363.27 ms      4.03 ms      0.03 ms    367.65 ms
BM_CommandLine/s2twp_jieba/ocd2/10000/min_time:0.010               408   ms      0.302   ms          1        1.21 MB/s    320.10 ms     42.84 ms      0.10 ms    363.40 ms
BM_CommandLine/s2twp_jieba/ocd2/100000/min_time:0.010              835   ms      0.952   ms          1        5.74 MB/s    326.00 ms    446.04 ms      2.03 ms    781.76 ms
BM_CommandLine/s2twp_jieba/text_json/100/min_time:0.010            438   ms      0.329   ms          1        0.01 MB/s    389.48 ms      0.42 ms      0.03 ms    390.14 ms
BM_CommandLine/s2twp_jieba/text_json/1000/min_time:0.010           398   ms      0.293   ms          1        0.12 MB/s    345.47 ms      3.94 ms      0.03 ms    349.70 ms
BM_CommandLine/s2twp_jieba/text_json/10000/min_time:0.010          437   ms      0.643   ms          1        1.12 MB/s    347.32 ms     42.54 ms      0.08 ms    390.28 ms
BM_CommandLine/s2twp_jieba/text_json/100000/min_time:0.010         849   ms      0.382   ms          1        5.68 MB/s    333.31 ms    448.09 ms      1.14 ms    789.76 ms
```


Results from Github CI, commit ID 9e80d5d, 2026-04-16, CMake macos-latest:

```
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
BM_Initialization/hk2s                868 us          868 us          665
BM_Initialization/hk2t                139 us          139 us         5059
BM_Initialization/jp2t                203 us          203 us         3448
BM_Initialization/s2hk              26201 us        26200 us           27
BM_Initialization/s2t               26385 us        26382 us           27
BM_Initialization/s2tw              27108 us        27108 us           27
BM_Initialization/s2twp             26446 us        26445 us           25
BM_Initialization/s2twp_jieba      142754 us       141974 us            5
BM_Initialization/t2hk               66.7 us         66.7 us        10519
BM_Initialization/t2jp                166 us          166 us         4215
BM_Initialization/t2s                 797 us          797 us          883
BM_Initialization/t2tw               58.1 us         58.1 us        12075
BM_Initialization/tw2s                845 us          845 us          831
BM_Initialization/tw2sp              1004 us         1004 us          697
BM_Initialization/tw2t               93.3 us         93.3 us         7492
BM_ConvertLongText/s2t                327 ms          327 ms            2 bytes_per_second=5.45069M/s
BM_ConvertLongText/s2twp              554 ms          554 ms            1 bytes_per_second=3.21299M/s
BM_ConvertLongText/s2twp_jieba        742 ms          741 ms            1 bytes_per_second=2.40096M/s
BM_Convert/s2t_100                  0.649 ms        0.649 ms         1083 bytes_per_second=6.15628M/s
BM_Convert/s2t_1000                  6.64 ms         6.64 ms          106 bytes_per_second=6.16118M/s
BM_Convert/s2t_10000                 68.1 ms         68.1 ms           10 bytes_per_second=6.14608M/s
BM_Convert/s2t_100000                 718 ms          717 ms            1 bytes_per_second=5.96785M/s
BM_Convert/s2twp_100                 1.20 ms         1.20 ms          552 bytes_per_second=3.32407M/s
BM_Convert/s2twp_1000                12.3 ms         12.3 ms           57 bytes_per_second=3.32311M/s
BM_Convert/s2twp_10000                126 ms          126 ms            6 bytes_per_second=3.31205M/s
BM_Convert/s2twp_100000              1296 ms         1296 ms            1 bytes_per_second=3.3027M/s
BM_Convert/s2twp_jieba_100           1.51 ms         1.49 ms          495 bytes_per_second=2.67698M/s
BM_Convert/s2twp_jieba_1000          15.0 ms         15.0 ms           48 bytes_per_second=2.72292M/s
BM_Convert/s2twp_jieba_10000          153 ms          153 ms            5 bytes_per_second=2.73681M/s
BM_Convert/s2twp_jieba_100000        1728 ms         1728 ms            1 bytes_per_second=2.47784M/s
```
