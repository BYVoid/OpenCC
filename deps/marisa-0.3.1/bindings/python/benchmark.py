import datetime
import marisa
import sys

time_begin = datetime.datetime.now()
keys = []
for line in sys.stdin:
  keys.append(line.rstrip())
time_end = datetime.datetime.now()
print "input:", time_end - time_begin

time_begin = datetime.datetime.now()
dic = dict()
for i in range(len(keys)):
  dic[keys[i]] = i
time_end = datetime.datetime.now()
print "dict_build:", time_end - time_begin

time_begin = datetime.datetime.now()
for key in keys:
  dic.get(key)
time_end = datetime.datetime.now()
print "dict_lookup:", time_end - time_begin

time_begin = datetime.datetime.now()
keyset = marisa.Keyset()
for key in keys:
  keyset.push_back(key)
time_end = datetime.datetime.now()
print "keyset_build:", time_end - time_begin

time_begin = datetime.datetime.now()
trie = marisa.Trie()
trie.build(keyset)
time_end = datetime.datetime.now()
print "trie_build:", time_end - time_begin

time_begin = datetime.datetime.now()
agent = marisa.Agent()
for key in keys:
  agent.set_query(key)
  trie.lookup(agent)
  agent.key_id()
time_end = datetime.datetime.now()
print "trie_agent_lookup:", time_end - time_begin

time_begin = datetime.datetime.now()
for key in keys:
  trie.lookup(key)
time_end = datetime.datetime.now()
print "trie_lookup:", time_end - time_begin

time_begin = datetime.datetime.now()
for i in range(len(keys)):
  agent.set_query(i)
  trie.reverse_lookup(agent)
  agent.key_str()
time_end = datetime.datetime.now()
print "trie_agent_reverse_lookup:", time_end - time_begin

time_begin = datetime.datetime.now()
for i in range(len(keys)):
  trie.reverse_lookup(i)
time_end = datetime.datetime.now()
print "trie_reverse_lookup:", time_end - time_begin

time_begin = datetime.datetime.now()
for key in keys:
  agent.set_query(key)
  while trie.common_prefix_search(agent):
    agent.key_str()
time_end = datetime.datetime.now()
print "trie_agent_common_prefix_search:", time_end - time_begin

time_begin = datetime.datetime.now()
for key in keys:
  agent.set_query(key)
  while trie.predictive_search(agent):
    agent.key_str()
time_end = datetime.datetime.now()
print "trie_agent_predictive_search:", time_end - time_begin
