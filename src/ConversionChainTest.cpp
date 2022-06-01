/*
 * Open Chinese Convert
 *
 * Copyright 2015-2021 Carbo Kuo <byvoid@byvoid.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ConversionChain.hpp"
#include "DictGroupTestBase.hpp"

namespace opencc {

class ConversionChainTest : public DictGroupTestBase {
protected:
  ConversionChainTest() {}

  virtual void SetUp() {
    dict = CreateDictGroupForConversion();
    conversion = ConversionPtr(new Conversion(dict));
  }

  void SegmentsAssertEquals(const SegmentsPtr& expected,
                            const SegmentsPtr& actual) {
    const size_t length = expected->Length();
    EXPECT_TRUE(length == actual->Length());
    for (size_t i = 0; i < length; i++) {
      EXPECT_EQ(std::string(expected->At(i)), std::string(actual->At(i)))
          << "i = " << i;
    }
  }

  DictPtr dict;
  ConversionPtr conversion;
};

TEST_F(ConversionChainTest, Convert) {
  // Variants
  const DictPtr& dictVariants = CreateDictForTaiwanVariants();
  const ConversionPtr& conversionVariants =
      ConversionPtr(new Conversion(dictVariants));
  const std::list<ConversionPtr> conversions{conversion, conversionVariants};
  const ConversionChainPtr& conversionChain =
      ConversionChainPtr(new ConversionChain(conversions));
  const SegmentsPtr& converted =
      conversionChain->Convert(SegmentsPtr(new Segments{utf8("里面")}));
  SegmentsAssertEquals(SegmentsPtr(new Segments{utf8("裡面")}), converted);
}

} // namespace opencc
