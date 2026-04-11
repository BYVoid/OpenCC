#ifndef CPPJIEBA_DICT_TRIE_HPP
#define CPPJIEBA_DICT_TRIE_HPP

#include <algorithm>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <deque>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "limonp/StringUtil.hpp"
#include "limonp/Logging.hpp"
#include "Unicode.hpp"
#include "Trie.hpp"

namespace cppjieba {

const double MIN_DOUBLE = -3.14e+100;
const double MAX_DOUBLE = 3.14e+100;
const size_t DICT_COLUMN_NUM = 3;
const char* const UNKNOWN_TAG = "";

class DictTrie {
 public:
  enum UserWordWeightOption {
    WordWeightMin,
    WordWeightMedian,
    WordWeightMax,
  }; // enum UserWordWeightOption

  DictTrie(const std::string& dict_path, const std::string& user_dict_paths = "", UserWordWeightOption user_word_weight_opt = WordWeightMedian) {
    Init(dict_path, user_dict_paths, user_word_weight_opt);
  }

  ~DictTrie() {
    delete trie_;
  }

  bool InsertUserWord(const std::string& word, const std::string& tag = UNKNOWN_TAG) {
    DictUnit node_info;
    if (!MakeNodeInfo(node_info, word, user_word_default_weight_, tag)) {
      return false;
    }
    active_node_infos_.push_back(node_info);
    trie_->InsertNode(node_info.word, &active_node_infos_.back());
    return true;
  }

  bool InsertUserWord(const std::string& word,int freq, const std::string& tag = UNKNOWN_TAG) {
    DictUnit node_info;
    double weight = freq ? log(1.0 * freq / freq_sum_) : user_word_default_weight_ ;
    if (!MakeNodeInfo(node_info, word, weight , tag)) {
      return false;
    }
    active_node_infos_.push_back(node_info);
    trie_->InsertNode(node_info.word, &active_node_infos_.back());
    return true;
  }

  bool DeleteUserWord(const std::string& word, const std::string& tag = UNKNOWN_TAG) {
    DictUnit node_info;
    if (!MakeNodeInfo(node_info, word, user_word_default_weight_, tag)) {
      return false;
    }
    trie_->DeleteNode(node_info.word, &node_info);
    return true;
  }

  const DictUnit* Find(RuneStrArray::const_iterator begin, RuneStrArray::const_iterator end) const {
    return trie_->Find(begin, end);
  }

  void Find(RuneStrArray::const_iterator begin,
        RuneStrArray::const_iterator end,
        std::vector<struct Dag>&res,
        size_t max_word_len = MAX_WORD_LENGTH) const {
    trie_->Find(begin, end, res, max_word_len);
  }

  bool Find(const std::string& word)
  {
    const DictUnit *tmp = NULL;
    RuneStrArray runes;
    if (!DecodeUTF8RunesInString(word, runes))
    {
      XLOG(ERROR) << "Decode failed.";
    }
    tmp = Find(runes.begin(), runes.end());
    if (tmp == NULL)
    {
      return false;
    }
    else
    {
      return true;
    }
  }

  bool IsUserDictSingleChineseWord(const Rune& word) const {
    return IsIn(user_dict_single_chinese_word_, word);
  }

  double GetMinWeight() const {
    return min_weight_;
  }

  void InserUserDictNode(const std::string& line) {
    std::vector<std::string> buf;
    DictUnit node_info;
    limonp::Split(line, buf, " ");
    if(buf.size() == 1){
          MakeNodeInfo(node_info,
                buf[0],
                user_word_default_weight_,
                UNKNOWN_TAG);
        } else if (buf.size() == 2) {
          MakeNodeInfo(node_info,
                buf[0],
                user_word_default_weight_,
                buf[1]);
        } else if (buf.size() == 3) {
          int freq = atoi(buf[1].c_str());
          assert(freq_sum_ > 0.0);
          double weight = log(1.0 * freq / freq_sum_);
          MakeNodeInfo(node_info, buf[0], weight, buf[2]);
        }
        static_node_infos_.push_back(node_info);
        if (node_info.word.size() == 1) {
          user_dict_single_chinese_word_.insert(node_info.word[0]);
        }
  }

  void LoadUserDict(const std::vector<std::string>& buf) {
    for (size_t i = 0; i < buf.size(); i++) {
      InserUserDictNode(buf[i]);
    }
  }

   void LoadUserDict(const std::set<std::string>& buf) {
    std::set<std::string>::const_iterator iter;
    for (iter = buf.begin(); iter != buf.end(); iter++){
      InserUserDictNode(*iter);
    }
  }

  void LoadUserDict(const std::string& filePaths) {
    std::vector<std::string> files = limonp::Split(filePaths, "|;");
    for (size_t i = 0; i < files.size(); i++) {
      std::ifstream ifs(files[i].c_str());
      XCHECK(ifs.is_open()) << "open " << files[i] << " failed";
      std::string line;

      while(getline(ifs, line)) {
        if (line.size() == 0) {
          continue;
        }
        InserUserDictNode(line);
      }
    }
  }


 private:
  struct DictCacheEntry {
    std::shared_ptr<const std::vector<DictUnit> > node_infos;
    double freq_sum;
    double min_weight;
    double max_weight;
    double median_weight;
  };

  void Init(const std::string& dict_path, const std::string& user_dict_paths, UserWordWeightOption user_word_weight_opt) {
    const DictCacheEntry& cache = GetDictCache(dict_path);
    base_static_node_infos_ = cache.node_infos;
    freq_sum_ = cache.freq_sum;
    min_weight_ = cache.min_weight;
    max_weight_ = cache.max_weight;
    median_weight_ = cache.median_weight;
    switch (user_word_weight_opt) {
      case WordWeightMin:
        user_word_default_weight_ = min_weight_;
        break;
      case WordWeightMedian:
        user_word_default_weight_ = median_weight_;
        break;
      default:
        user_word_default_weight_ = max_weight_;
        break;
    }

    if (user_dict_paths.size()) {
      LoadUserDict(user_dict_paths);
    }
    Shrink(static_node_infos_);
    CreateTrie();
  }

  void CreateTrie() {
    const size_t total_size = base_static_node_infos_->size() + static_node_infos_.size();
    assert(total_size);
    std::vector<Unicode> words;
    std::vector<const DictUnit*> valuePointers;
    words.reserve(total_size);
    valuePointers.reserve(total_size);

    for (size_t i = 0; i < base_static_node_infos_->size(); i++) {
      words.push_back((*base_static_node_infos_)[i].word);
      valuePointers.push_back(&(*base_static_node_infos_)[i]);
    }
    for (size_t i = 0; i < static_node_infos_.size(); i++) {
      words.push_back(static_node_infos_[i].word);
      valuePointers.push_back(&static_node_infos_[i]);
    }

    trie_ = new Trie(words, valuePointers);
  }

  bool MakeNodeInfo(DictUnit& node_info,
        const std::string& word,
        double weight,
        const std::string& tag) {
    if (!DecodeUTF8RunesInString(word, node_info.word)) {
      XLOG(ERROR) << "UTF-8 decode failed for dict word: " << word;
      return false;
    }
    node_info.weight = weight;
    node_info.tag = tag;
    return true;
  }

  static DictCacheEntry BuildDictCacheEntry(const std::string& filePath) {
    DictCacheEntry entry;
    std::vector<DictUnit> node_infos;
    std::ifstream ifs(filePath.c_str());
    XCHECK(ifs.is_open()) << "open " << filePath << " failed.";
    std::string line;
    std::vector<std::string> buf;
    while (getline(ifs, line)) {
      limonp::Split(line, buf, " ");
      XCHECK(buf.size() == DICT_COLUMN_NUM) << "split result illegal, line:" << line;
      DictUnit node_info;
      XCHECK(DecodeUTF8RunesInString(buf[0], node_info.word)) << "UTF-8 decode failed for dict word: " << buf[0];
      node_info.weight = atof(buf[1].c_str());
      node_info.tag = buf[2];
      node_infos.push_back(node_info);
    }
    XCHECK(!node_infos.empty()) << "dict file is empty: " << filePath;

    entry.freq_sum = CalcFreqSum(node_infos);
    CalculateWeight(node_infos, entry.freq_sum);
    std::vector<DictUnit> sorted = node_infos;
    std::sort(sorted.begin(), sorted.end(), WeightCompare);
    entry.min_weight = sorted.front().weight;
    entry.max_weight = sorted.back().weight;
    entry.median_weight = sorted[sorted.size() / 2].weight;

    entry.node_infos = std::shared_ptr<const std::vector<DictUnit> >(new std::vector<DictUnit>(node_infos));
    return entry;
  }

  static const DictCacheEntry& GetDictCache(const std::string& filePath) {
    static std::unordered_map<std::string, DictCacheEntry> cache;
    static std::mutex cache_mutex;
    std::lock_guard<std::mutex> lock(cache_mutex);
    std::unordered_map<std::string, DictCacheEntry>::const_iterator it = cache.find(filePath);
    if (it != cache.end()) {
      return it->second;
    }
    DictCacheEntry entry = BuildDictCacheEntry(filePath);
    std::pair<std::unordered_map<std::string, DictCacheEntry>::iterator, bool> result =
      cache.insert(std::make_pair(filePath, entry));
    return result.first->second;
  }

  static bool WeightCompare(const DictUnit& lhs, const DictUnit& rhs) {
    return lhs.weight < rhs.weight;
  }

  static double CalcFreqSum(const std::vector<DictUnit>& node_infos) {
    double sum = 0.0;
    for (size_t i = 0; i < node_infos.size(); i++) {
      sum += node_infos[i].weight;
    }
    return sum;
  }

  static void CalculateWeight(std::vector<DictUnit>& node_infos, double sum) {
    assert(sum > 0.0);
    for (size_t i = 0; i < node_infos.size(); i++) {
      DictUnit& node_info = node_infos[i];
      assert(node_info.weight > 0.0);
      node_info.weight = log(double(node_info.weight)/sum);
    }
  }

  void Shrink(std::vector<DictUnit>& units) const {
    std::vector<DictUnit>(units.begin(), units.end()).swap(units);
  }

  std::shared_ptr<const std::vector<DictUnit> > base_static_node_infos_;
  std::vector<DictUnit> static_node_infos_;
  std::deque<DictUnit> active_node_infos_; // must not be std::vector
  Trie * trie_;

  double freq_sum_;
  double min_weight_;
  double max_weight_;
  double median_weight_;
  double user_word_default_weight_;
  std::unordered_set<Rune> user_dict_single_chinese_word_;
};
}

#endif
