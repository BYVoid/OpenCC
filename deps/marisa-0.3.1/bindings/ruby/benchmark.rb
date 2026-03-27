require "marisa"

time_begin = Time.now
keys = STDIN.read.split("\n")
time_end = Time.now
print "input: ", time_end - time_begin, "\n"

time_begin = Time.now
hash = Hash.new
for key in keys
  hash[key] = 0
end
time_end = Time.now
print "hash_build: ", time_end - time_begin, "\n"

time_begin = Time.now
hash = Hash.new
for key in keys
  hash[key]
end
time_end = Time.now
print "hash_lookup: ", time_end - time_begin, "\n"

time_begin = Time.now
keyset = Marisa::Keyset.new
for key in keys
  keyset.push_back(key)
end
time_end = Time.now
print "keyset_build: ", time_end - time_begin, "\n"

time_begin = Time.now
trie = Marisa::Trie.new
trie.build(keyset)
time_end = Time.now
print "trie_build: ", time_end - time_begin, "\n"

time_begin = Time.now
agent = Marisa::Agent.new
for key in keys
  agent.set_query(key)
  trie.lookup(agent)
  agent.key_str
end
time_end = Time.now
print "trie_agent_lookup: ", time_end - time_begin, "\n"

time_begin = Time.now
for key in keys
  trie.lookup(key)
end
time_end = Time.now
print "trie_lookup: ", time_end - time_begin, "\n"

time_begin = Time.now
max_key_id = trie.size() - 1
0.upto(max_key_id) { |i|
  agent.set_query(i)
  trie.reverse_lookup(agent)
  agent.key_str
}
time_end = Time.now
print "trie_agent_reverse_lookup: ", time_end - time_begin, "\n"

time_begin = Time.now
0.upto(max_key_id) { |i|
  trie.reverse_lookup(i)
}
time_end = Time.now
print "trie_reverse_lookup: ", time_end - time_begin, "\n"

time_begin = Time.now
for key in keys
  agent.set_query(key)
  while trie.common_prefix_search(agent)
    agent.key_str
  end
end
time_end = Time.now
print "trie_agent_common_prefix_search: ", time_end - time_begin, "\n"

time_begin = Time.now
for key in keys
  agent.set_query(key)
  while trie.predictive_search(agent)
    agent.key_str
  end
end
time_end = Time.now
print "trie_agent_predictive_search: ", time_end - time_begin, "\n"
