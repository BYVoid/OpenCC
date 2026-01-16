/*
 * Open Chinese Convert (OpenCC) LexiconAnnotationTest
 *
 * Copyright 2026 Frank Lin <github@linshuang.info>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Lexicon.hpp"
#include "SerializableDict.hpp"
#include "TestUtils.hpp"
#include "TestUtilsUTF8.hpp"
#include "TextDict.hpp"

namespace opencc {

class LexiconAnnotationTest : public ::testing::Test {
protected:
  const std::string testFileName = "test_annotation_dict.txt";

  void TearDown() override { remove(testFileName.c_str()); }
};

TEST_F(LexiconAnnotationTest, ParseCommentLines) {
  FILE* fp = fopen(testFileName.c_str(), "w");
  fprintf(fp, "# This is a header comment\n");
  fprintf(fp, "# Line 2 of header\n");
  fprintf(fp, "\n");
  fprintf(fp, "A\tB\n");
  fprintf(fp, "C\tD\n");
  fclose(fp);

  FILE* readFp = fopen(testFileName.c_str(), "r");
  const TextDictPtr& dict = TextDict::NewFromFile(readFp);
  fclose(readFp);
  EXPECT_EQ(dict->GetLexicon()->Length(), 2);
}

TEST_F(LexiconAnnotationTest, ParseAttachedComment) {
  FILE* fp = fopen(testFileName.c_str(), "w");
  fprintf(fp, "# Header\n");
  fprintf(fp, "\n");
  fprintf(fp, "# Comment for A\n");
  fprintf(fp, "A\tB\n");
  fprintf(fp, "C\tD\n");
  fclose(fp);

  FILE* readFp = fopen(testFileName.c_str(), "r");
  const TextDictPtr& dict = TextDict::NewFromFile(readFp);
  fclose(readFp);
  EXPECT_EQ(dict->GetLexicon()->Length(), 2);
}

TEST_F(LexiconAnnotationTest, ParseFloatingComment) {
  FILE* fp = fopen(testFileName.c_str(), "w");
  fprintf(fp, "A\tB\n");
  fprintf(fp, "\n");
  fprintf(fp, "# This is a floating comment\n");
  fprintf(fp, "\n");
  fprintf(fp, "C\tD\n");
  fclose(fp);

  FILE* readFp = fopen(testFileName.c_str(), "r");
  const TextDictPtr& dict = TextDict::NewFromFile(readFp);
  fclose(readFp);
  EXPECT_EQ(dict->GetLexicon()->Length(), 2);
}

TEST_F(LexiconAnnotationTest, ParseFooterComment) {
  FILE* fp = fopen(testFileName.c_str(), "w");
  fprintf(fp, "A\tB\n");
  fprintf(fp, "C\tD\n");
  fprintf(fp, "\n");
  fprintf(fp, "# Footer comment\n");
  fprintf(fp, "# Line 2 of footer\n");
  fclose(fp);

  FILE* readFp = fopen(testFileName.c_str(), "r");
  const TextDictPtr& dict = TextDict::NewFromFile(readFp);
  fclose(readFp);
  EXPECT_EQ(dict->GetLexicon()->Length(), 2);
}

TEST_F(LexiconAnnotationTest, SerializeIgnoresComments) {
  FILE* fp = fopen(testFileName.c_str(), "w");
  fprintf(fp, "# Header\n");
  fprintf(fp, "\n");
  fprintf(fp, "# Comment for B\n");
  fprintf(fp, "B\tBB\n");
  fprintf(fp, "A\tAA\n");
  fprintf(fp, "\n");
  fprintf(fp, "# Footer\n");
  fclose(fp);

  FILE* readFp = fopen(testFileName.c_str(), "r");
  const TextDictPtr& dict = TextDict::NewFromFile(readFp);
  fclose(readFp);

  // Serialize back
  const std::string outputFileName = "test_annotation_dict_output.txt";
  FILE* outFp = fopen(outputFileName.c_str(), "w");
  dict->SerializeToFile(outFp);
  fclose(outFp);

  // Read back and verify
  FILE* outputFp = fopen(outputFileName.c_str(), "r");
  char buff[1024];
  std::vector<std::string> lines;
  while (fgets(buff, sizeof(buff), outputFp)) {
    std::string line(buff);
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
      line.pop_back();
    }
    lines.push_back(line);
  }
  fclose(outputFp);
  remove(outputFileName.c_str());

  EXPECT_EQ(lines.size(), 2);
  EXPECT_EQ(lines[0], "A\tAA");
  EXPECT_EQ(lines[1], "B\tBB");
}

TEST_F(LexiconAnnotationTest, SortIgnoresComments) {
  FILE* fp = fopen(testFileName.c_str(), "w");
  fprintf(fp, "# Header\n");
  fprintf(fp, "\n");
  fprintf(fp, "# Comment for C\n");
  fprintf(fp, "C\tCC\n");
  fprintf(fp, "# Comment for A\n");
  fprintf(fp, "A\tAA\n");
  fprintf(fp, "B\tBB\n");
  fclose(fp);

  FILE* readFp = fopen(testFileName.c_str(), "r");
  const TextDictPtr& dict = TextDict::NewFromFile(readFp);
  fclose(readFp);

  EXPECT_EQ(dict->GetLexicon()->Length(), 3);
}

TEST_F(LexiconAnnotationTest, DefaultBehaviorIgnoresComments) {
  FILE* fp = fopen(testFileName.c_str(), "w");
  fprintf(fp, "A\tB\n");
  fprintf(fp, "C\tD\n");
  fclose(fp);

  // Default behavior should ignore comments
  FILE* readFp = fopen(testFileName.c_str(), "r");
  const TextDictPtr& dict = TextDict::NewFromFile(readFp);
  fclose(readFp);

  EXPECT_EQ(dict->GetLexicon()->Length(), 2);
}

TEST_F(LexiconAnnotationTest, DefaultBehaviorAcceptsCommentLines) {
  FILE* fp = fopen(testFileName.c_str(), "w");
  fprintf(fp, "# This is a comment\n");
  fprintf(fp, "A\tB\n");
  fclose(fp);

  FILE* readFp = fopen(testFileName.c_str(), "r");
  const TextDictPtr& dict = TextDict::NewFromFile(readFp);
  fclose(readFp);

  EXPECT_EQ(dict->GetLexicon()->Length(), 1);
}

} // namespace opencc
