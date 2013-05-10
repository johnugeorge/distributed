#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "common_utils.h"


using namespace std;
template<typename T> bool in_vector(vector<T>& v);
template<typename K, typename V> bool in_map(map<K, V>& m);
template<typename T> void remove_from_vector(vector<T>& v, T data);
template<typename K, typename V> void remove_from_map(map<K, V>& m, K key);


/*
 * checks for the presence of data in a vector
 */
template<typename T> 
bool in_vector(vector<T>& v, T data)
{
  PRINT(LOG_DEBUG, "Entering method in_vector");
  if(v.empty()) return false;

  typename vector<T>::iterator it = v.begin();
  while(it != v.end())
  {
    if((*it) == data)
      return true;
    it++;
  }

  PRINT(LOG_DEBUG, "Exiting method in_vector");
  return false;
}


/*
 * removes the FIRST occurance of data from v
 */
template<typename T>
void remove_from_vector(vector<T>& v, T data)
{
  PRINT(LOG_DEBUG, "Entering method remove_from_vector");
  if(v.empty()) return;

  typename vector<T>::iterator it = v.begin();
  while(it != v.end())
  {
    if((*it) == data)
    {
      v.erase(it);
      PRINT(LOG_DEBUG, "Element "<<data<<" deleted");
      return;
    }
    it++;
  }

  PRINT(LOG_DEBUG, "Element "<<data<<" not in vector");
  PRINT(LOG_DEBUG, "Exiting method remove_from_vector");
}


/*
 * removes the FIRST occurance of the key from a map
 */
template<typename K, typename V>
void remove_from_map(map<K, V>& m, K key)
{
  PRINT(LOG_DEBUG, "Entering method remove_from_map");
  if(m.empty()) return;

  typename map<K, V>::iterator it = m.begin();
  while(it != m.end())
  {
    if(it->first == key)
    {
      m.erase(it);
      return;
    }
    it++;
  }

  PRINT(LOG_DEBUG, "Element "<<key<<" not in map");
  PRINT(LOG_DEBUG, "Exiting method remove_from_map");
}


/*
 * checks for the presence of key in map
 */
template<typename K, typename V>
bool in_map(map<K, V>& m, K key)
{
  PRINT(LOG_DEBUG, "Entering method in_map");
  if(m.empty()) return false;

  typename map<K, V>::iterator it = m.begin();
  while(it != m.end())
  {
    if(it->first == key)
      return true;
    it++;
  }


  PRINT(LOG_DEBUG, "Element "<<key<<" not in map");
  PRINT(LOG_DEBUG, "Exiting method in_map");
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


/*
 * util method to split a string
 */
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


/*
 * helper method to create a payload
 */
string create_payload(vector<string> strings)
{
  PRINT(LOG_DEBUG, "Entering method create_payload");
  string pl;
  size_t n = strings.size();
  int i = 0;
  while(i < n-1)
  {
    pl.append(strings[i]);
    pl.append(" ");
    i++;
  }
  pl.append(strings[n-1]);

  PRINT(LOG_DEBUG, "Exiting method create_payload");
  return pl;
}


string create_crack_payload(string hash, int task_num, map<int, string> store)
{
  vector<string> strings;
  string c("c");
  string sub_task = store[task_num];
  vector<string> spl = strsplit(sub_task, "-");
  strings.push_back(c);
  strings.push_back(hash);
  strings.push_back(spl[0]);
  strings.push_back(spl[1]);

  string s = create_payload(strings);
  return s;
}


/*
 * helper method to create request divisions
 */
vector<string> get_divisions(int* pwd_length, int* divisions)
{
  int p = *pwd_length;
  int d = 13;
  if(divisions != NULL)
    d = *divisions;

  int skip = d == 13 ? 1 : 0;

  string hyphen("-");
  char first = 'a';
  char last = 'z';

  if(p <= 3)
  {
    //=== make only one division
    vector<string> v;
    string div;
    div.append(p, first);
    div.append(hyphen);
    div.append(p, last);
    v.push_back(div);
    return v;
  }

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
    first_copy += 1;
    j++;
  }

  return divs;
}

