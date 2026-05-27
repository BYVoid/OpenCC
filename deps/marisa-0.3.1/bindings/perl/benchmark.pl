use Time::HiRes;
use marisa;

my $time_begin = Time::HiRes::gettimeofday();
my @keys = <STDIN>;
foreach my $key (@keys) {
  chomp($key);
}
my $time_end = Time::HiRes::gettimeofday();
print "input: ", $time_end - $time_begin, "\n";

$time_begin = Time::HiRes::gettimeofday();
my %hash;
foreach my $key (@keys) {
  $hash{$key} = 0;
}
$time_end = Time::HiRes::gettimeofday();
print "hash_build: ", $time_end - $time_begin, "\n";

$time_begin = Time::HiRes::gettimeofday();
foreach my $key (@keys) {
  $hash{$key};
}
$time_end = Time::HiRes::gettimeofday();
print "hash_lookup: ", $time_end - $time_begin, "\n";

$time_begin = Time::HiRes::gettimeofday();
my $keyset = new marisa::Keyset;
foreach my $key (@keys) {
  $keyset->push_back($key)
}
$time_end = Time::HiRes::gettimeofday();
print "keyset_build: ", $time_end - $time_begin, "\n";

$time_begin = Time::HiRes::gettimeofday();
$trie = new marisa::Trie;
$trie->build($keyset);
$time_end = Time::HiRes::gettimeofday();
print "trie_build: ", $time_end - $time_begin, "\n";

$time_begin = Time::HiRes::gettimeofday();
my $agent = new marisa::Agent;
foreach my $key (@keys) {
  $agent->set_query($key);
  $trie->lookup($agent);
  $agent->key_id();
}
$time_end = Time::HiRes::gettimeofday();
print "trie_agent_lookup: ", $time_end - $time_begin, "\n";

$time_begin = Time::HiRes::gettimeofday();
foreach my $key (@keys) {
  $trie->lookup($key);
}
$time_end = Time::HiRes::gettimeofday();
print "trie_lookup: ", $time_end - $time_begin, "\n";

$time_begin = Time::HiRes::gettimeofday();
my $max_key_id = $trie->size() - 1;
for (0..$max_key_id) {
  $agent->set_query($_);
  $trie->reverse_lookup($agent);
  $agent->key_str();
}
$time_end = Time::HiRes::gettimeofday();
print "trie_agent_reverse_lookup: ", $time_end - $time_begin, "\n";

$time_begin = Time::HiRes::gettimeofday();
for (0..$max_key_id) {
  $trie->reverse_lookup($_);
}
$time_end = Time::HiRes::gettimeofday();
print "trie_reverse_lookup: ", $time_end - $time_begin, "\n";

$time_begin = Time::HiRes::gettimeofday();
foreach my $key (@keys) {
  $agent->set_query($key);
  while ($trie->common_prefix_search($agent)) {
    $agent->key_str();
  }
}
$time_end = Time::HiRes::gettimeofday();
print "trie_agent_common_prefix_search: ", $time_end - $time_begin, "\n";

$time_begin = Time::HiRes::gettimeofday();
foreach my $key (@keys) {
  $agent->set_query($key);
  while ($trie->predictive_search($agent)) {
    $agent->key_str();
  }
}
$time_end = Time::HiRes::gettimeofday();
print "trie_agent_predictive_search: ", $time_end - $time_begin, "\n";
