/*
  Functions relating to the communication between tracker and client
  
  Author: Daniel Bishop
*/

#include "tracker.h"

/*
  Request that the tracker server send all ips of other clients which have "joined"
  The response comes in many parts.
  The first part details how many ips will be sent, and by extension how many more messages are coming.
  Each extra message contains 1 ip.
*/
void requestPeers(int tracker_socket, char *peers, int *numPeers) {
  fprintf(stdout, "Requesting peer list...\n");
  if (sendMsg(tracker_socket, "list") == 0) {
    char *recv_msg = recvMsg(tracker_socket);
    fprintf(stdout, "Sizeof data: %d\n", recv_msg[0]);
    memcpy(numPeers, recv_msg, 1);
    memcpy(peers, recv_msg + 1, *numPeers*16);
    free(recv_msg);
  }
  
}

/*
  Request to add our ip to the list of connected ips to be returned when the peer list is requested.
*/
void joinTracker(int tracker_socket) {
  fprintf(stdout, "Registering as available peer...\n");
  if (sendMsg(tracker_socket, "join") == 0) {
    char *recv_msg = recvMsg(tracker_socket);
    // The confirmation of a successful join is the reception of a "joined" message
    fprintf(stdout, "Received message of length: %d\n", recv_msg[0]);
    if (strncmp(recv_msg + 1, "joined", 6) == 0) {
      fprintf(stdout, "Successfully joined as available peer.\n");
    } else {
      unsigned int msg_len = recv_msg[0];
      int i;
      fprintf(stderr, "[Error ");
      for (i = 0; i < msg_len; i++) {
        fprintf(stderr, "%s", recv_msg[i + 1]);
      }
      fprintf(stderr, "] ");
      fprintf(stderr, "Error joining as available peer. Please try again later.\n");
    }
    free(recv_msg);
  }
}

/*
  Given a string message, prepend it with a singly byte containing the length of the message.
*/
char *buildMsg(char *msg) {
  int len = strlen(msg);
  char *data = malloc(len + 1);
  memcpy(data, &len, 1);
  memcpy(data + 1, msg, len);
  return data;
}

/*
  Send a string message over a given socket filedef. The string message is automatically
  prepended with the length of the string, allowing for easy transmission over the network.
  
  Returns 0 if successful, or -1 if an error occured (error message to stderr).
*/
int sendMsg(int tracker_socket, char *msg) {
  char *data = buildMsg(msg);
  if (send(tracker_socket, data, strlen(msg) + 1, MSG_NOSIGNAL) == -1) {
    fprintf(stderr, "[Error %s] ", strerror(errno));
    fprintf(stderr, "Error communicating with socket. Please try again later.\n");
    return -1;
  }
  return 0;
}

/*
  Sends a message over a given socket. This differs to sendMsg() in that the char * message
  is not necessarily a string, and so the length of the data must be passed as a parameter.
  
  Returns 0 if message was sent successfully, or -1 if an error occured (error msg to stderr).
*/
int sendData(int socket, char *data, int len) {
  if (send(socket, data, len, MSG_NOSIGNAL) == -1) {
    fprintf(stderr, "[Error %s] ", strerror(errno));
    fprintf(stderr, "Error communicating with socket. Please try again later.\n");
    return -1;
  }
  return 0;
}

/*
  Blocking wrapper for recv() on socket provided. 
  
  Returns pointer to buffer containing message.
*/
char *recvMsg(int tracker_socket) {
  ssize_t recv_len;
  char *buffer = malloc(BUFSIZ);
  if ((recv_len = recv(tracker_socket, buffer, BUFSIZ, 0)) == -1) {
    fprintf(stderr, "[Error %s] ", strerror(errno));
    fprintf(stderr, "Error receiving data socket. Please try again later.\n");
  }

  return buffer;
}

/*
  Given a non-empty linked list of clients, add a new client if the client's IP
  is not represented in the list.
  If a new client is added to the list, update numClients accordingly.
  O(n) time complexity
*/
void addIfAbsent(Client *head, char *ip, int *numClients) {
  Client *curr_node;
  curr_node = head;
  // if (strcmp(curr_node->ip, ip) == 0) {
  //   // Special case for when linked list contains only 1 node
  //   printf("Head was duplicate.\n");
  //   return;
  // }
  while (1) {
    if (strcmp(curr_node->ip, ip) == 0) {
      printf("Node was duplicate.\n");
      return;
    } else {
      if (curr_node->next == 0) {
        break;
      } else {
        printf("Checking next.\n");
        curr_node = curr_node->next;
      }
    }
  }

  // When the while loop ends, curr_node should point to the tail of the list
  printf("Adding new node: %s\n", ip);
  Client *new_node;
  new_node = malloc(sizeof(Client));
  new_node->ip = strdup(ip);
  new_node->next = 0;
  curr_node->next = new_node;
  (*numClients)++;
}

/*
  Given a linked list, free up the memory used by the linked list.
*/
void destroyClientList(Client *head) {
  Client *curr_node = head;
  Client *placeholder;
  while (curr_node->next != 0) {
    placeholder = curr_node->next;
    free(curr_node);
    curr_node = placeholder;
  }
  free(curr_node);
}

/*
  Configure and return the filedef of a listening socket that is ready to accept connections.
*/
int configureSocket() {
  int reuse = 1;
  int listening_socket = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in listening_addr;
  listening_addr.sin_family = PF_INET;
  listening_addr.sin_port = (in_port_t)htons(TRACKER_PORT);
  listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1) {
    fprintf(stderr, "Can't set the 'reuse' option on the socket.\n");
    exit(1);
  }
    
  if (bind(listening_socket, (struct sockaddr *)&listening_addr, sizeof(listening_addr)) == -1) {
    fprintf(stderr, "Can't bind to socket.\n");
    exit(1);
  }

  if (listening_socket < 0) {
    fprintf(stderr, "Error creating socket: error %d\n", listening_socket);
    exit(1);
  }

  if (listen(listening_socket, 10) == -1) {
    fprintf(stderr, "Can't listen.\n");
    exit(1);
  }

  return listening_socket;
}

/*
  Given a linked list of clients, write their ip's to a buffer and send
  it over the socket given.

  Does not include the ignore_ip in the message.
*/
void sendClients(int socket, Client *head, int numClients, char *ignore_ip) {
  int numClientsToSend = numClients - 1;
  if (numClients <= 0) {
    // Do not attempt to send no clients
    printf("No clients to send.\n");
    return;
  }

  // Bundle all client ip's into one message
  char *data = malloc(1 + (IP_SIZE * numClientsToSend));
  memcpy(data, &numClientsToSend, 1);

  Client *curr_node;
  curr_node = head;
  int counted_nodes = 1;
  // Ensure that the length of the linked list given is equal to the
  // number of clients given
  while (curr_node-> next != 0) {
    curr_node = curr_node->next;
    counted_nodes++;
  }
  if (counted_nodes != numClients) {
    fprintf(stderr, "Unexpected number of clients (%d expected %d)\n", counted_nodes, numClients);
    return;
  }

  int i;
  int numIpsCopied = 0;
  curr_node = head;

  // Copy all ip's into data buffer for sending
  for (i = 0; i < numClients; i++) {
    if (strcmp(curr_node->ip, ignore_ip) != 0) {
      memcpy(data + 1 + (IP_SIZE * numIpsCopied), curr_node->ip, IP_SIZE);
      numIpsCopied++;
    }
    curr_node = curr_node->next;
  }

  sendData(socket, data, 1 + (IP_SIZE * numClientsToSend));
  free(data);
}
