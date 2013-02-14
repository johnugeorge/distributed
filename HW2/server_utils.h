#include <vector>
#include <map>

using namespace std;

template<typename T> bool in_vector(vector<T>&, T);
template<typename T> void remove_from_vector(vector<T>&, T);
template<typename K, typename V> bool in_map(map<K, V>&, K);
vector<int> init_sub_tasks();
