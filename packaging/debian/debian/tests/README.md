CommandLineConvertTest.cpp is a fork of the ConvertFromJson test in
../../test/CommandLineConvertTest.cpp, trimmed down and adjusted to run
against the installed packages (binary and configurations from
/usr/bin and /usr/share/opencc) instead of the build tree. Please
refresh it when the upstream test or testcases.json format changes.

Tests:
- cli-smoke: quick conversion checks with the installed opencc CLI.
- integration: builds the forked gtest against libopencc-dev via
  pkg-config and runs every entry of ../../test/testcases/testcases.json
  through the installed opencc tool and configurations.
- jieba-plugin: checks that the opencc-jieba plugin loads and the
  *_jieba.json configurations produce segmentation-aware output.
