#include <iostream>
#include <map>
#include <vector>
#include <string>

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
    {
      v.erase(it);
      return;
    }
    it++;
  }

  cout<<" Element "<<data<<" not in vector \n";
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


  cout<<"Element "<<key<<" not in map \n";
  return false;
}


vector<int> new_sub_task_list(int* divisions)
{
  int d = *divisions;
  vector<int> v;
  int i = 0;

  while(i < d)
  {
    v.push_back(i+1);
    i++;
  }

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


const char* create_payload(vector<string> strings)
{
  string pl;
  size_t n = strings.size();
  int i = 0;
  while(i < n-1)
  {
    pl.append(strings[i]+" ");
    i++;
  }
  pl.append(strings[n-1]);

  return pl.c_str();
}


const char* create_crack_payload(string hash, int task_num, map<int, string>& sub_task_map)
{
  vector<string> strings;
  string c("c");
  string sub_task = sub_task_map[task_num];
  vector<string> spl = strsplit(sub_task, "-");
  strings.push_back(c);
  strings.push_back(hash);
  strings.push_back(spl[0]);
  strings.push_back(spl[1]);

  return create_payload(strings);
}


vector<string> get_divisions(int* pwd_length, int* divisions)
{
  int p = *pwd_length;
  int d = *divisions;
  int skip = d == 13 ? 1 : 0;

  string hyphen("-");
  char first = 'a';
  char last = 'z';

  int j = 0;
  char first_copy = first;
  vector<string> divs;
  while(j < d)
  {
    string div;
    div.append(1, first_copy);
    div.append(p-1, first);
    div.append(hyphen);
    first_copy += skip;
    div.append(1, first_copy);
    div.append(p-1, last);
    divs.push_back(div);
    //cout<<div<<endl;
    first_copy += 1;
    j++;
  }

  return divs;
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
