#pragma once

#include <time.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include "Queue.h"
#include "common_utils.h"
#include <map>
#include <arpa/inet.h>
#include <sstream>

// Global Parameters. For both server and clients.

//#define _EPOCH_LTH 2.0
//#define _EPOCH_CNT 5
//#define _DROP_RATE 0.0
#define  MAX_CLIENTS 20
#define MAX_PAYLOAD_SIZE 1000

typedef enum loglevels{
	LOG_DEBUG=1,
	LOG_INFO =2,
	LOG_CRIT =3
}loglevels;

extern int loglevel;
extern std::ofstream outFile;

extern pthread_mutex_t global_mutex;
extern pthread_cond_t global_created;




void lsp_set_epoch_lth(double lth);
void lsp_set_epoch_cnt(int cnt);
void lsp_set_drop_rate(double rate);
void lsp_set_log_level(int lg);
double lsp_get_epoch_lth();
int lsp_get_epoch_cnt();
double lsp_get_drop_rate();

typedef struct
{
int conn_id;
int seq_no;
char* data;
}pckt_fmt;

typedef struct conn_arg
{
	int conn_id;
	int seq_no;
}conn_arg;


typedef struct
{
	bool operator() (const conn_arg& a,const conn_arg& b)
	{
		if(a.conn_id == b.conn_id)
			return (a.seq_no < b.seq_no);
		else
			return (a.conn_id < b.conn_id);
	}
}conn_comp;

typedef std::map<conn_arg,bool,conn_comp> conn_seqno_map;
typedef std::map<long long,std::string> thread_info;

extern thread_info thread_info_map;

typedef enum pckt_type
{
	DATA_PCKT=1,
	DATA_ACK =2,
	CONN_REQ =3,
	CONN_ACK =4,
	DATA_PCKT_RESEND=5
}pckt_type;

inline std::ostream& operator<< (std::ostream& os, pckt_type var) {

	switch (var) {
		case DATA_PCKT:
			return os << "DATA PACKET";
		case DATA_ACK:
			return os << "DATA ACK";
		case CONN_REQ:
			return os << "CONNECTION REQUEST";
		case CONN_ACK:
			return os << "CONNECTION ACK";
	}
	return os;
}


typedef enum conn_type
{
	CONN_REQ_SENT=1,
	CONN_REQ_ACK_RCVD=2,
	CONN_REQ_RCVD=3,
	CONN_REQ_ACK_SENT=4,
	DATA_SENT=5,
	DATA_ACK_RCD=6,
	DATA_RCVD=7,
	DATA_ACK_SENT=8,
	CONN_WAIT=9
}conn_type;

typedef struct inbox_struct
{
	pckt_fmt pkt;
	int payload_size;
}inbox_struct;

typedef struct outbox_struct
{
	pckt_fmt pkt;
	int payload_size;
}outbox_struct;

typedef std::map<std::string, std::string> Configuration;
extern Configuration config;
inline void initialize_configuration()
{
  if(!config.empty())
  {
    PRINT(LOG_INFO, "Initial configuration already set.\n");
    return;
  }

  std::ifstream read;
  std::string file_name = "lsp.conf";
  read.open(file_name.c_str());
  PRINT(LOG_INFO, "Reading configuration...\n");
  std::string s;
  while(std::getline(read, s)) config[s.substr(0, s.find('='))]=s.substr(s.find('=')+1);

  lsp_set_epoch_lth(atof(config["EPOCH_LTH"].c_str()));
  lsp_set_epoch_cnt(atoi(config["EPOCH_CNT"].c_str()));
  lsp_set_drop_rate(atof(config["DROP_RATE"].c_str()));

  std::string lg_level = config["LOG_LEVEL"];
  if(lg_level != "")
  {
    if(lg_level == "LOG_DEBUG")
      lsp_set_log_level(1);
    else if(lg_level == "LOG_INFO")
      lsp_set_log_level(2);
    else if(lg_level == "LOG_CRIT")
      lsp_set_log_level(3);
  }
}


/*
 * generic print function for a vector
 */
template<typename T>
std::string print_vector(std::vector<T>& v)
{
  if(v.empty()) return "[]";

  typename std::vector<T>::iterator it = v.begin();
  std::string res;
  res.append("[ ");
  while(it != v.end())
  {
    std::stringstream ss;
    ss<<(*it);
    res.append(ss.str()+" ");
    it++;
  }
  
  res.append("]");
  return res;
} 

/*
 * generic print function for map<K, vector<V> >
 */
template<typename K, typename V>
std::string print_vector_map(std::map<K, std::vector<V> >& m)
{
  if(m.empty()) return "{}";

  typename std::map<K, std::vector<V> >::iterator it = m.begin();
  std::string res;
  while(it != m.end())
  {
    std::stringstream ss;
    std::vector<V>& v = it->second;
    ss<<it->first<<" = "<<print_vector(v)<<"\n";
    res.append(ss.str());
    it++;
  }  

  return res;
}

/*
 * generic print function for map<K, vector<V> >
 */
template<typename K, typename V>
std::string print_map(std::map<K, V>& m)
{
  if(m.empty()) return "{}";

  typename std::map<K, V>::iterator it = m.begin();
  std::string res;
  while(it != m.end())
  {
    std::stringstream ss;
    ss<<it->first<<"="<<it->second<<"\n";
    res.append(ss.str());
    it++;
  }

  return res;
}


uint8_t* message_encode(int conn_id,int seq_no,const char* payload,int& outlength);
int message_decode(int len,uint8_t* buf,pckt_fmt& pkt);

