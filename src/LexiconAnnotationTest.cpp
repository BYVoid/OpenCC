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
  const TextDictPtr& dict = TextDict::NewFromFile(readFp, true);
  fclose(readFp);
  EXPECT_EQ(dict->GetLexicon()->Length(), 2);
  EXPECT_TRUE(dict->GetLexicon()->HasAnnotations());

  const auto& headerBlocks = dict->GetLexicon()->GetHeaderBlocks();
  EXPECT_EQ(headerBlocks.size(), 1);
  EXPECT_EQ(headerBlocks[0].lines.size(), 2);
  EXPECT_EQ(headerBlocks[0].lines[0], "# This is a header comment");
  EXPECT_EQ(headerBlocks[0].lines[1], "# Line 2 of header");
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
  const TextDictPtr& dict = TextDict::NewFromFile(readFp, true);
  fclose(readFp);
  const auto& annotated = dict->GetLexicon()->GetAnnotatedEntries();

  EXPECT_EQ(annotated.size(), 2);
  EXPECT_TRUE(annotated[0].attachedComment != nullptr);
  EXPECT_EQ(annotated[0].attachedComment->lines.size(), 1);
  EXPECT_EQ(annotated[0].attachedComment->lines[0], "# Comment for A");
  EXPECT_TRUE(annotated[1].attachedComment == nullptr);
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
  const TextDictPtr& dict = TextDict::NewFromFile(readFp, true);
  fclose(readFp);
  const auto& floatingBlocks = dict->GetLexicon()->GetFloatingBlocks();

  EXPECT_EQ(floatingBlocks.size(), 1);
  EXPECT_EQ(floatingBlocks[0].first, 1); // Anchored to second entry (C)
  EXPECT_EQ(floatingBlocks[0].second.lines.size(), 1);
  EXPECT_EQ(floatingBlocks[0].second.lines[0], "# This is a floating comment");
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
  const TextDictPtr& dict = TextDict::NewFromFile(readFp, true);
  fclose(readFp);
  const auto& footerBlocks = dict->GetLexicon()->GetFooterBlocks();

  EXPECT_EQ(footerBlocks.size(), 1);
  EXPECT_EQ(footerBlocks[0].lines.size(), 2);
  EXPECT_EQ(footerBlocks[0].lines[0], "# Footer comment");
  EXPECT_EQ(footerBlocks[0].lines[1], "# Line 2 of footer");
}

TEST_F(LexiconAnnotationTest, SerializeWithAnnotations) {
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
  const TextDictPtr& dict = TextDict::NewFromFile(readFp, true);
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

  // Verify structure (header, entries, footer)
  EXPECT_TRUE(lines[0] == "# Header");
  EXPECT_TRUE(lines[1] == "");
  // Should still have comment attached to B even though entries may be reordered
  bool foundCommentForB = false;
  for (size_t i = 0; i < lines.size(); ++i) {
    if (lines[i] == "# Comment for B" && i + 1 < lines.size() &&
        lines[i + 1].find("B\tBB") == 0) {
      foundCommentForB = true;
      break;
    }
  }
  EXPECT_TRUE(foundCommentForB);
}

TEST_F(LexiconAnnotationTest, SortWithAnnotations) {
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
  const TextDictPtr& dict = TextDict::NewFromFile(readFp, true);
  fclose(readFp);

  // Entries should be sorted, but comments should follow their entries
  const auto& annotated = dict->GetLexicon()->GetAnnotatedEntries();
  EXPECT_EQ(annotated.size(), 3);

  // After sorting: A, B, C
  EXPECT_EQ(annotated[0].Key(), "A");
  EXPECT_TRUE(annotated[0].attachedComment != nullptr);
  EXPECT_EQ(annotated[0].attachedComment->lines[0], "# Comment for A");

  EXPECT_EQ(annotated[1].Key(), "B");
  EXPECT_TRUE(annotated[1].attachedComment == nullptr);

  EXPECT_EQ(annotated[2].Key(), "C");
  EXPECT_TRUE(annotated[2].attachedComment != nullptr);
  EXPECT_EQ(annotated[2].attachedComment->lines[0], "# Comment for C");
}

TEST_F(LexiconAnnotationTest, DefaultBehaviorPreservesComments) {
  FILE* fp = fopen(testFileName.c_str(), "w");
  fprintf(fp, "A\tB\n");
  fprintf(fp, "C\tD\n");
  fclose(fp);

  // Default behavior should preserve comments
  FILE* readFp = fopen(testFileName.c_str(), "r");
  const TextDictPtr& dict = TextDict::NewFromFile(readFp);
  fclose(readFp);

  EXPECT_EQ(dict->GetLexicon()->Length(), 2);
  EXPECT_TRUE(dict->GetLexicon()->HasAnnotations());
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
  EXPECT_TRUE(dict->GetLexicon()->HasAnnotations());
}

} // namespace opencc
