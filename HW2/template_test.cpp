#include <iostream>
#include <map>
#include <vector>

using namespace std;
template<typename T> bool inVector(vector<T>& v);
template<typename K, typename V> bool inMap(map<K, V>& m);

template<typename T> 
bool inVector(vector<T>& v, T data)
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

template<typename K, typename V>
bool inMap(map<K, V>& m, K key)
{
  cout<<"ab"<<endl;
  if(m.empty()) return false;

  cout<<"ab1"<<endl;
  typename map<K, V>::iterator it = m.begin();
  while(it != m.end())
  {
  cout<<"ab2"<<endl;
    if(it->first == key)
      return true;
    it++;
  }

  cout<<"ab4"<<endl;
  return false;
}

class Test1
{
  map<int, vector<int>& > m;

  public:
  Test1()
  {
  }
  
  void add_el(int r, int w)
  {
    map<int, vector<int>& >::iterator it = m.begin();
    it = m.find(r);

    if(it != m.end())
      (it->second).push_back(w);
    else
      m[r].push_back(w);
  }

  void print()
  {
    map<int, vector<int>& >::iterator it = m.begin();

    while(it != m.end())
    {
      cout<<"key: "<<it->first<<",,,";
      vector<int> v = it->second;
      for(int i=0; i<v.size(); i++)
        cout<<v[i]<<" ";
      it++;
    }
  }
};

int main(void)
{
  /*cout<<"h"<<endl;
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
  cout<<inMap(m, 1)<<endl;*/

  Test1* t = new Test1();
  t->add_el(1, 10);
  t->add_el(1, 11);
  t->print();
  return 0;
}
