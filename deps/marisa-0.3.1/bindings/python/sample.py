import marisa

keyset = marisa.Keyset()
keyset.push_back("cake")
keyset.push_back("cookie")
keyset.push_back("ice")
keyset.push_back("ice-cream")

trie = marisa.Trie()
trie.build(keyset)
print("no. keys: %d" % trie.num_keys())
print("no. tries: %d" % trie.num_tries())
print("no. nodes: %d" % trie.num_nodes())
print("size: %d" % trie.io_size())

agent = marisa.Agent()

agent.set_query("cake")
trie.lookup(agent)
print("%s: %d" % (agent.query_str(), agent.key_id()))

agent.set_query("cookie")
trie.lookup(agent)
print("%s: %d" % (agent.query_str(), agent.key_id()))

agent.set_query("cockoo")
if not trie.lookup(agent):
  print("%s: not found" % agent.query_str())

print("ice: %d" % trie.lookup("ice"))
print("ice-cream: %d" % trie.lookup("ice-cream"))
if trie.lookup("ice-age") == marisa.INVALID_KEY_ID:
  print("ice-age: not found")

trie.save("sample.dic")
trie.load("sample.dic")

agent.set_query(0)
trie.reverse_lookup(agent)
print("%d: %s" % (agent.query_id(), agent.key_str()))

agent.set_query(1)
trie.reverse_lookup(agent)
print("%d: %s" % (agent.query_id(), agent.key_str()))

print("2: %s" % trie.reverse_lookup(2))
print("3: %s" % trie.reverse_lookup(3))

trie.mmap("sample.dic")

agent.set_query("ice-cream soda")
while trie.common_prefix_search(agent):
  print("%s: %s (%d)" % (agent.query_str(), agent.key_str(), agent.key_id()))

agent.set_query("ic")
while trie.predictive_search(agent):
  print("%s: %s (%d)" % (agent.query_str(), agent.key_str(), agent.key_id()))
