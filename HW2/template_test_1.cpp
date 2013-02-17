#include <iostream>
#include <map>
#include <vector>
#include "server_utils.h"
#include "lsp.h"
//#include "lsp.c"

using namespace std;
template<typename T> bool inVector(vector<T>& v);
template<typename K, typename V> bool inMap(map<K, V>& m);
bool break_pwd_r(string, string&, string&, string&);

int first = 97;
int last = 122;

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
  map<int, vector<int> > m;

  public:
  Test1()
  {
  }
  
  void add_el(int r, int w)
  { 
    if(m.empty())
    {
      vector<int> v;
      v.push_back(w);
      m[r].swap(v);
      //m.insert(pair<int, vector<int> >(r, v));
      cout<<"done"<<endl;
      //cout<<"vec size is "<<m[r].size();
      return;
    }

    map<int, vector<int> >::iterator it = m.begin();
    it = m.find(r);

    if(it != m.end())
    {
      (it->second).push_back(w);
      cout<<"done 1"<<endl;
      //cout<<"vec size is "<<m[r].size();
    }
    else
    {
      vector<int> v;
      v.push_back(w);
      //m.insert(pair<int, vector<int> >(r, v));
      m[r].swap(v);
      cout<<"done33"<<endl;
      //cout<<"vec size is "<<m[r].size();
    }
  }

  map<int, vector<int> >& getMap()
  {
    return m;
  }

  void setMap(map<int, vector<int> >& mp)
  {
    m = mp;
  }

  void modify(int r, int val)
  {
    vector<int> v = m[r];
    v[0] = val;
    m[r].swap(v);
  }

  void modify2(vector<int>& v)
  {
    //size_t i = 0;
    //for(; i<v.size(); i++)
    //  v.at(i)++;
    v.push_back(44);
  }

  void print()
  {
    map<int, vector<int> >::iterator it = m.begin();
    
    cout<<"printing"<<endl;
    while(it != m.end())
    {
      cout<<"hi"<<endl;
      cout<<"key: "<<it->first<<",,,";
      cout<<"abcd"<<endl;
      vector<int> v = it->second;
      cout<<"size: "<<v.size()<<endl;
      for(size_t i=0; i<v.size(); i++)
        cout<<v.at(i)<<" ";
      cout<<endl;
      it++;
    }
  }
};

/*
vector<string> get_divisions(int* pwd_length, int* divisions)
{
  int p = *pwd_length;
  int d = *divisions;
  int skip = d == 13 ? 1 : 0;
  //int first = 97;
  //int last = 122;

  string hyphen("-");
  char first = 'a';
  char last = 'z';

  int j = 0;
  char first_copy = first;
  while(j < d)
  {
    //string first_copy(first);
    string div;
    div.append(1, first_copy);
    div.append(p-1, first);
    div.append(hyphen);
    first_copy += skip;
    div.append(1, first_copy);
    div.append(p-1, last);
    cout<<div<<endl;
    first_copy += 1;
    j++;
  }
}
*/

bool break_pwd(string lo, string hi, string target)
{
  return break_pwd_r(lo, lo, hi, target);
}

bool break_pwd_r(string s, string& lo, string& hi, string& target)
{
    if(s == hi)
    {
      cout<<s<<endl;
      cout<<" Not found"<<endl;
      return false;
    }

    if(s == target)
    {
      cout<<s<<" Found!"<<endl;
      return true;
    }


    cout<<s<<endl;
    int ch = s.length()-1;
    bool res = false;
    while(ch >= 0)
    {
      if(int(s[ch]) == last)
      {
        char a = char(first);
        char y = *((char*)s.data()+ch);
        *((char*)s.data()+ch) = a;
        ch--;
        continue;
      }

      char y = *((char*)s.data()+ch);
      *((char*)s.data()+ch) = y+1;

      res = break_pwd_r(s, lo, hi, target);
      if(res || !res) //weird
        break;
      ch--;
    }

  return res;
}

/*
bool break_pwd_i(string s, string& lo, string& hi, string& target)
{
  while(s != hi)
  {
    if(s == target) 
      return true;

    cout<<s<<endl;
    int ch = s.length()-1;
    bool res = false;
    while(ch >= 0)
    {
      if(int(s[ch]) == last)
      {
        char a = char(first);
        char y = *((char*)s.data()+ch);
        *((char*)s.data()+ch) = a;
        ch--;
        continue;
      }

      char y = *((char*)s.data()+ch);
      *((char*)s.data()+ch) = y+1;

      res = break_pwd_r(s, lo, hi, target);
      if(res || !res) //weird
        break;
      ch--;
    }
  }
}
*/
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

  /*Test1* t = new Test1();
  t->add_el(1, 10);
  t->add_el(1, 11);
  t->print();
  t->modify(1, 30);
  t->print();
  map<int, vector<int> >& m = t->getMap();
  vector<int>& v1 = m[1];
  cout<<"vec size: "<<v1.size();
  t->modify2(v1);
  cout<<"vec size: "<<v1.size();
  m[1].swap(v1);
  //t->setMap(m);
  t->print();*/
 
  /*int pwd = 4;
  int divisions = 13;
  get_divisions(&pwd, &divisions);
  cout<<"----------------------"<<endl;
  pwd = 7;
  divisions = 13;
  get_divisions(&pwd, &divisions);*/
  //break_pwd("aaa", "aae", "aad");
  //cout<<"-----------------------"<<endl;
  //break_pwd("aaaa", "bzzz", "bzyx");
  //cout<<"-----------------------"<<endl;
  //break_pwd("aaaaaa", "bzzzzz", "bzyxzx");
  //cout<<"-----------------------"<<endl;
  //break_pwd("aaaa", "bzzz", "zzzz");
  //cout<<"-----------------------"<<endl;
  break_pwd("aaaaa", "bzzzz", "zzzzz");
  cout<<"-----------------------"<<endl;
  return 0;
}
