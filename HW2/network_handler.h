#include<sys/socket.h>
#include<sys/time.h>
#include<error.h>

void c_network_handler(void*);
void s_network_handler(void*);
void *get_in_addr(struct sockaddr *sa);
int socket_cmp_addr(const struct sockaddr *sa1, const struct sockaddr *sa2);
int socket_cmp_port(const struct sockaddr *sa1, const struct sockaddr *sa2);

