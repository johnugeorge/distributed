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
  PRINT(LOG_INFO, "Entering method in_vector");
  if(v.empty()) return false;

  typename vector<T>::iterator it = v.begin();
  while(it != v.end())
  {
    if((*it) == data)
      return true;
    it++;
  }

  PRINT(LOG_INFO, "Exiting method in_vector");
  return false;
}

/*
 * removes the FIRST occurance of data from v
 */
template<typename T>
void remove_from_vector(vector<T>& v, T data)
{
  PRINT(LOG_INFO, "Entering method remove_from_vector");
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

  PRINT(LOG_INFO, "Element "<<data<<" not in vector");
  PRINT(LOG_INFO, "Exiting method remove_from_vector");
}


template<typename K, typename V>
bool in_map(map<K, V>& m, K key)
{
  PRINT(LOG_INFO, "Entering method in_map");
  if(m.empty()) return false;

  typename map<K, V>::iterator it = m.begin();
  while(it != m.end())
  {
    if(it->first == key)
      return true;
    it++;
  }


  PRINT(LOG_INFO, "Element "<<key<<" not in map");
  PRINT(LOG_INFO, "Exiting method in_map");
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


string create_payload(vector<string> strings)
{
  PRINT(LOG_INFO, "Entering method create_payload");
  string pl;
  size_t n = strings.size();
  int i = 0;
  while(i < n-1)
  {
    //cout<<"appending "<<strings[i]<<endl;
    pl.append(strings[i]+" ");
    i++;
  }
  pl.append(strings[n-1]);

  //cout<<pl<<endl;
  PRINT(LOG_INFO, "Exiting method create_payload");
  return pl;
}


string create_crack_payload(string hash, int task_num, map<int, string>& sub_task_map)
{
  vector<string> strings;
  string c("c");
  string sub_task = sub_task_map[task_num];
  vector<string> spl = strsplit(sub_task, "-");
  strings.push_back(c);
  strings.push_back(hash);
  strings.push_back(spl[0]);
  strings.push_back(spl[1]);
  //cout<<strings.size()<<endl;

  string s = create_payload(strings);
  //cout<<"Creating crack payload: "<<s<<endl;
  return s;
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

