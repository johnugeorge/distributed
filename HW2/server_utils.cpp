#include <iostream>
#include <map>
#include <vector>

using namespace std;
template<typename T> bool in_vector(vector<T>& v);
template<typename K, typename V> bool in_map(map<K, V>& m);

template<typename T> 
bool in_vector(vector<T>& v, T data)
{
  if(v.empty()) return false;

  typename vector<T>::iterator it = v.begin();
  while(it != v.end())
  {
    if((*it) == data)
      return true;
    it++;
  }

  return false;
}

/*
 * removes the FIRST occurance of data from v
 */
template<typename T>
void remove_from_vector(vector<T>& v, T data)
{
  if(v.empty()) return;

  typename vector<T>::iterator it = v.begin();
  while(it != v.end())
  {
    if((*it) == data)
      break;
    it++;
  }

  v.erase(it);
  return;
}



template<typename K, typename V>
bool in_map(map<K, V>& m, K key)
{
  if(m.empty()) return false;

  typename map<K, V>::iterator it = m.begin();
  while(it != m.end())
  {
    if(it->first == key)
      return true;
    it++;
  }

  return false;
}

vector<int> init_sub_tasks()
{
  vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  v.push_back(4);
  v.push_back(5);
  return v;
}


vector<string> strsplit(string s, string delim)
{
  vector<string> tokens;
  int start = 0;
  int len = s.length();
  size_t f;
  int found;
  string token;
  while(start < len)
  {
    f = s.find(delim, start);
    if(f == string::npos)
    {
      //this could be the last token, or the
      //patter does not exist in this string
      tokens.push_back(s.substr(start));
      break;
    }

    found = int(f);
    token = s.substr(start, found - start);
    tokens.push_back(token);

    start = found + delim.length();
  }

  return tokens;
}
/*int main(void)
{
  cout<<"h"<<endl;
  vector<int> v1;
  v1.push_back(1);
  v1.push_back(2);
  v1.push_back(3);
  vector<char> v2;
  v2.push_back('a');
  cout<<inVector(v1, 3)<<endl;
  cout<<inVector(v1, 6)<<endl;
  cout<<inVector(v2, 'b')<<endl;

  map<int, string> m;
  m[1] = "one";
  cout<<inMap(m, 1)<<endl;
  return 0;
}*/
