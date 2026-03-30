#include <cstring>
#include <new>

#include "marisa-swig.h"

namespace marisa_swig {

void Key::str(const char **ptr_out, size_t *length_out) const {
  *ptr_out = key_.ptr();
  *length_out = key_.length();
}

size_t Key::id() const {
  return key_.id();
}

float Key::weight() const {
  return key_.weight();
}

void Query::str(const char **ptr_out, size_t *length_out) const {
  *ptr_out = query_.ptr();
  *length_out = query_.length();
}

size_t Query::id() const {
  return query_.id();
}

Keyset::Keyset() : keyset_(new (std::nothrow) marisa::Keyset) {
  MARISA_THROW_IF(keyset_ == NULL, ::MARISA_MEMORY_ERROR);
}

Keyset::~Keyset() {
  delete keyset_;
}

void Keyset::push_back(const marisa::Key &key) {
  keyset_->push_back(key);
}

void Keyset::push_back(const char *ptr, size_t length, float weight) {
  keyset_->push_back(ptr, length, weight);
}

const Key &Keyset::key(size_t i) const {
  return reinterpret_cast<const Key &>((*keyset_)[i]);
}

void Keyset::key_str(size_t i,
    const char **ptr_out, size_t *length_out) const {
  *ptr_out = (*keyset_)[i].ptr();
  *length_out = (*keyset_)[i].length();
}

size_t Keyset::key_id(size_t i) const {
  return (*keyset_)[i].id();
}

size_t Keyset::num_keys() const {
  return keyset_->num_keys();
}

bool Keyset::empty() const {
  return keyset_->empty();
}

size_t Keyset::size() const {
  return keyset_->size();
}

size_t Keyset::total_length() const {
  return keyset_->total_length();
}

void Keyset::reset() {
  keyset_->reset();
}

void Keyset::clear() {
  keyset_->clear();
}

Agent::Agent()
    : agent_(new (std::nothrow) marisa::Agent), buf_(NULL), buf_size_(0) {
  MARISA_THROW_IF(agent_ == NULL, ::MARISA_MEMORY_ERROR);
}

Agent::~Agent() {
  delete agent_;
  delete [] buf_;
}

void Agent::set_query(const char *ptr, size_t length) {
  if (length > buf_size_) {
    size_t new_buf_size = (buf_size_ != 0) ? buf_size_ : 1;
    if (length >= (MARISA_SIZE_MAX / 2)) {
      new_buf_size = MARISA_SIZE_MAX;
    } else {
      while (new_buf_size < length) {
        new_buf_size *= 2;
      }
    }
    char *new_buf = new (std::nothrow) char[new_buf_size];
    MARISA_THROW_IF(new_buf == NULL, MARISA_MEMORY_ERROR);
    delete [] buf_;
    buf_ = new_buf;
    buf_size_ = new_buf_size;
  }
  std::memcpy(buf_, ptr, length);
  agent_->set_query(buf_, length);
}

void Agent::set_query(size_t id) {
  agent_->set_query(id);
}

const Key &Agent::key() const {
  return reinterpret_cast<const Key &>(agent_->key());
}

const Query &Agent::query() const {
  return reinterpret_cast<const Query &>(agent_->query());
}

void Agent::key_str(const char **ptr_out, size_t *length_out) const {
  *ptr_out = agent_->key().ptr();
  *length_out = agent_->key().length();
}

size_t Agent::key_id() const {
  return agent_->key().id();
}

void Agent::query_str(const char **ptr_out, size_t *length_out) const {
  *ptr_out = agent_->query().ptr();
  *length_out = agent_->query().length();
}

size_t Agent::query_id() const {
  return agent_->query().id();
}

Trie::Trie() : trie_(new (std::nothrow) marisa::Trie) {
  MARISA_THROW_IF(trie_ == NULL, ::MARISA_MEMORY_ERROR);
}

Trie::~Trie() {
  delete trie_;
}

void Trie::build(Keyset &keyset, int config_flags) {
  trie_->build(*keyset.keyset_, config_flags);
}

void Trie::mmap(const char *filename, int flags) {
  trie_->mmap(filename, flags);
}

void Trie::load(const char *filename) {
  trie_->load(filename);
}

void Trie::save(const char *filename) const {
  trie_->save(filename);
}

bool Trie::lookup(Agent &agent) const {
  return trie_->lookup(*agent.agent_);
}

void Trie::reverse_lookup(Agent &agent) const {
  trie_->reverse_lookup(*agent.agent_);
}

bool Trie::common_prefix_search(Agent &agent) const {
  return trie_->common_prefix_search(*agent.agent_);
}

bool Trie::predictive_search(Agent &agent) const {
  return trie_->predictive_search(*agent.agent_);
}

size_t Trie::lookup(const char *ptr, size_t length) const {
  marisa::Agent agent;
  agent.set_query(ptr, length);
  if (!trie_->lookup(agent)) {
    return MARISA_INVALID_KEY_ID;
  }
  return agent.key().id();
}

void Trie::reverse_lookup(size_t id,
    const char **ptr_out_to_be_deleted, size_t *length_out) const {
  marisa::Agent agent;
  agent.set_query(id);
  trie_->reverse_lookup(agent);
  char * const buf = new (std::nothrow) char[agent.key().length()];
  MARISA_THROW_IF(buf == NULL, MARISA_MEMORY_ERROR);
  std::memcpy(buf, agent.key().ptr(), agent.key().length());
  *ptr_out_to_be_deleted = buf;
  *length_out = agent.key().length();
}

size_t Trie::num_tries() const {
  return trie_->num_tries();
}

size_t Trie::num_keys() const {
  return trie_->num_keys();
}

size_t Trie::num_nodes() const {
  return trie_->num_nodes();
}

TailMode Trie::tail_mode() const {
  if (trie_->tail_mode() == ::MARISA_TEXT_TAIL) {
    return TEXT_TAIL;
  } else {
    return BINARY_TAIL;
  }
}

NodeOrder Trie::node_order() const {
  if (trie_->node_order() == ::MARISA_LABEL_ORDER) {
    return LABEL_ORDER;
  } else {
    return WEIGHT_ORDER;
  }
}

bool Trie::empty() const {
  return trie_->empty();
}

size_t Trie::size() const {
  return trie_->size();
}

size_t Trie::total_size() const {
  return trie_->total_size();
}

size_t Trie::io_size() const {
  return trie_->io_size();
}

void Trie::clear() {
  trie_->clear();
}

}  // namespace marisa_swig
