use marisa;

$keyset = new marisa::Keyset;
$keyset->push_back("cake");
$keyset->push_back("cookie");
$keyset->push_back("ice");
$keyset->push_back("ice-cream");

$trie = new marisa::Trie;
$trie->build($keyset);
print("no. keys: ", $trie->num_keys(), "\n");
print("no. tries: ", $trie->num_tries(), "\n");
print("no. nodes: ", $trie->num_nodes(), "\n");
print("size: ", $trie->io_size(), "\n");

$agent = new marisa::Agent;

$agent->set_query("cake");
$trie->lookup($agent);
print($agent->query_str(), ": ", $agent->key_id(), "\n");

$agent->set_query("cookie");
$trie->lookup($agent);
print($agent->query_str(), ": ", $agent->key_id(), "\n");

$agent->set_query("cockoo");
if ($trie->lookup(agent)) {
  print($agent->query_str(), ": not found\n");
}

print("ice: ", $trie->lookup("ice"), "\n");
print("ice-cream: ", $trie->lookup("ice-cream"), "\n");
if ($trie->lookup("ice-age") == $marisa::INVALID_KEY_ID) {
  print("ice-age: not found\n");
}

$trie->save("sample.dic");
$trie->load("sample.dic");

$agent->set_query(0);
$trie->reverse_lookup($agent);
print($agent->query_id(), ": ", $agent->key_str(), "\n");
$agent->set_query(1);
$trie->reverse_lookup($agent);
print($agent->query_id(), ": ", $agent->key_str(), "\n");

print("2: ", $trie->reverse_lookup(2), "\n");
print("3: ", $trie->reverse_lookup(3), "\n");

$trie->mmap("sample.dic");

$agent->set_query("ice-cream soda");
while ($trie->common_prefix_search($agent)) {
  print($agent->query_str(), ": ", $agent->key_str(), " (",
        $agent->key_id(), ")\n");
}

$agent->set_query("ic");
while ($trie->predictive_search($agent)) {
  print($agent->query_str(), ": ", $agent->key_str(), " (",
        $agent->key_id(), ")\n");
}
