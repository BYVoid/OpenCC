# Publications Using OpenCC

OpenCC is widely used as a Chinese script conversion and text normalization tool in natural language processing, computational linguistics, machine translation, corpus construction, and language model evaluation.

This page lists selected academic publications that explicitly use, compare against, or cite OpenCC. The list is not exhaustive. Pull requests adding more publications are welcome.

## Chinese Script Conversion

These papers directly study Simplified/Traditional Chinese conversion or use OpenCC as a baseline system.

| Year | Publication                                                                                                                                    | Venue            | OpenCC usage                                                                  |
| ---- | ---------------------------------------------------------------------------------------------------------------------------------------------- | ---------------- | ----------------------------------------------------------------------------- |
| 2024 | [An Unsupervised Framework for Adaptive Context-aware Simplified-Traditional Chinese Conversion](https://aclanthology.org/2024.lrec-main.118/) | LREC-COLING 2024 | Compares against OpenCC as a public conversion baseline.                      |
| 2020 | [2kenize: Tying Subword Sequences for Chinese Script Conversion](https://aclanthology.org/2020.acl-main.648/)                                  | ACL 2020         | Uses OpenCC as an off-the-shelf script conversion baseline.                   |
| 2017 | [Simplified-Traditional Chinese Conversion and Proofreading](https://aclanthology.org/I17-3016/)                                               | IJCNLP 2017      | Compares OpenCC with other Simplified/Traditional Chinese conversion systems. |

## Chinese NLP and Corpus Preprocessing

These papers use OpenCC to normalize Chinese corpora before training, evaluation, or downstream NLP experiments.

| Year | Publication                                                                                                               | Venue       | OpenCC usage                                                                                         |
| ---- | ------------------------------------------------------------------------------------------------------------------------- | ----------- | ---------------------------------------------------------------------------------------------------- |
| 2024 | [Machine Translation Evaluation Benchmark for Wu Chinese: Workflow and Analysis](https://aclanthology.org/2024.wmt-1.47/) | WMT 2024    | Uses OpenCC to convert Traditional Chinese data to Simplified Chinese during benchmark construction. |
| 2022 | [ParaZh-22M: A Large-Scale Chinese Parabank via Machine Translation](https://aclanthology.org/2022.coling-1.341/)         | COLING 2022 | Uses OpenCC to convert Traditional Chinese to Simplified Chinese in the data cleaning pipeline.      |
| 2018 | [Analogical Reasoning on Chinese Morphological and Semantic Relations](https://aclanthology.org/P18-2023/)                | ACL 2018    | Uses OpenCC to convert Traditional Chinese characters into Simplified Chinese during preprocessing.  |
| 2017 | [Earth Mover’s Distance Minimization for Unsupervised Bilingual Lexicon Induction](https://aclanthology.org/D17-1207/)    | EMNLP 2017  | Uses OpenCC in Chinese corpus preprocessing.                                                         |

## Chinese Spelling Correction and Grammatical Error Correction

These papers use OpenCC to normalize Traditional Chinese datasets, especially SIGHAN-style datasets, into Simplified Chinese for Chinese spelling correction or grammatical error correction experiments.

| Year | Publication                                                                                                                             | Venue                       | OpenCC usage                                                                            |
| ---- | --------------------------------------------------------------------------------------------------------------------------------------- | --------------------------- | --------------------------------------------------------------------------------------- |
| 2024 | [Uncertainty Guidance for Multimodal Chinese Spelling Correction](https://aclanthology.org/2024.lrec-main.1002/)                        | LREC-COLING 2024            | Uses OpenCC in Chinese spelling correction data processing.                             |
| 2024 | [Error-Robust Retrieval for Chinese Spelling Check](https://aclanthology.org/2024.lrec-main.553/)                                       | LREC-COLING 2024            | Uses OpenCC to preprocess Traditional Chinese data.                                     |
| 2024 | [EdaCSC: Two Easy Data Augmentation Methods for Chinese Spelling Correction](https://arxiv.org/abs/2409.05105)                          | arXiv                       | Uses OpenCC to convert SIGHAN Traditional Chinese data into Simplified Chinese.         |
| 2023 | [General and Domain-adaptive Chinese Spelling Check with Error Consistent Pretraining](https://dl.acm.org/doi/10.1145/3564271)          | ACM TOIS                    | Uses OpenCC to convert Traditional Chinese datasets into Simplified Chinese.            |
| 2021 | [Read, Listen, and See: Leveraging Multimodal Information Helps Chinese Spell Checking](https://aclanthology.org/2021.findings-acl.64/) | Findings of ACL-IJCNLP 2021 | Uses OpenCC to convert Traditional Chinese SIGHAN data into Simplified Chinese.         |
| 2020 | [Heterogeneous Recycle Generation for Chinese Grammatical Error Correction](https://aclanthology.org/2020.coling-main.199/)             | COLING 2020                 | Includes OpenCC in the preprocessing pipeline for Chinese grammatical error correction. |

## Machine Translation, Low-Resource Chinese Varieties, and Multilingual Processing

These papers use OpenCC in machine translation, Cantonese/Wu Chinese processing, or cross-script normalization.

| Year | Publication                                                                                                                                              | Venue        | OpenCC usage                                                                                 |
| ---- | -------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------ | -------------------------------------------------------------------------------------------- |
| 2024 | [Leveraging Mandarin as a Pivot Language for Low-Resource Machine Translation between Cantonese and English](https://aclanthology.org/2024.loresmt-1.8/) | LoResMT 2024 | Uses OpenCC to convert Simplified Chinese data to Traditional Chinese for transfer learning. |
| 2023 | [Cantonese to Written Chinese Translation via HuggingFace Translation Pipeline](https://dl.acm.org/doi/10.1145/3639233.3639332)                          | NLPIR 2023   | Uses OpenCC to convert Mandarin text into Traditional Chinese.                               |
| 2020 | [Korean-to-Japanese Neural Machine Translation System using Hanja Information](https://aclanthology.org/2020.wat-1.15/)                                  | WAT 2020     | Uses OpenCC in Hanja/Kanji-related text processing.                                          |

## Language Model Evaluation and Benchmarks

These papers use OpenCC to construct or normalize benchmark data for evaluating language models.

| Year | Publication                                                                                                                                                 | Venue              | OpenCC usage                                                                                    |
| ---- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------ | ----------------------------------------------------------------------------------------------- |
| 2024 | [Chat Vector: A Simple Approach to Equip LLMs with Instruction Following and Model Alignment in New Languages](https://aclanthology.org/2024.acl-long.590/) | ACL 2024           | Uses OpenCC to convert Simplified Chinese benchmark data into Traditional Chinese.              |
| 2024 | [TMMLU+: An Improved Traditional Chinese Evaluation Suite for LLMs](https://openreview.net/forum?id=95TayIeqJ4)                                             | OpenReview / arXiv | Uses OpenCC to convert TMMLU+ questions and prompts between Traditional and Simplified Chinese. |

## Adding Publications

To add a publication, please include:

* title
* authors, if available
* year and venue
* stable link, preferably DOI, ACL Anthology, ACM, arXiv, OpenReview, or publisher page
* a short note describing how OpenCC is used

Suggested format:

```markdown
| YEAR | [TITLE](URL) | VENUE | Uses OpenCC to ... |
```

## Citing OpenCC

If you use OpenCC in academic work, please cite the project repository:

```bibtex
@misc{opencc,
  title        = {OpenCC: Open Chinese Convert},
  author       = {Kuo, Carbo and contributors},
  howpublished = {\url{https://github.com/BYVoid/OpenCC}},
  note         = {Accessed: YYYY-MM-DD}
}
```
