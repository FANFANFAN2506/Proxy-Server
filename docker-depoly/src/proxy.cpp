#include "proxy.hpp"

#include "utils.hpp"
#include<unistd.h>
void * runProxy(void * myProxy) {
<<<<<<< Updated upstream
  Proxy * Proxy_instance = (Proxy *)myProxy;
  std::string Line = recvAll(Proxy_instance->socket_des);
  // std::string Line = receiveAll(Proxy_instance->socket_des);
=======
  Proxy * Proxy_instance = (Proxy *) myProxy;
  std::cout << "-----"<<Proxy_instance->return_socket_des() << std::endl;
  //std::string Line = recvAll(Proxy_instance->socket_des);
  std::string Line = recvAll(Proxy_instance->socket_des);
>>>>>>> Stashed changes
  Proxy_instance->setRequest(Line);
  Proxy_instance->judgeRequest();
  return NULL;
}

//Proxy memeber functions:

void Proxy::setRequest(std::string Line) {
  try {
    // std::string Received_time = get_current_Time();
    time_t curr_time;
    time(&curr_time);
    request = new http_Request(this->socket_des, Line, this->clientIP, curr_time);
    request->constructRequest();
    std::cout << "request has the line: " << request->return_Line() << std::endl;
  }
  catch (std::exception & e) {
    delete request;
    std::cerr << "Request construction failed" << std::endl;
  }
}

void Proxy::judgeRequest() {
  if (request->return_method() == "CONNECT") {
    std::cout << "connect" << std::endl;
    /* If the method is CONNECT: 
    1. Setup the connection with target server
    2. The request line should be ignored, send the header & data to server
    3. Reply a HTTP 200 OK 
    */
    int socket_server = connectServer();
<<<<<<< Updated upstream
    // std::stringstream sstream;
    // sstream << request->return_httpver() << " 200 OK\r\n\r\n";
    // const char * message_client = "HTTP/1.1 200 OK\r\n\r\n";
    // std::cout << sstream.str().c_str() << std::endl;
    // error = send(socket_server, sstream.str().c_str(), strlen(sstream.str().c_str()), 0);
    // error = send(socket_client, sstream.str().c_str(), strlen(sstream.str().c_str()), 0);
=======
    //usleep(100);
    // const char * message_client = "HTTP/1.1 200 OK\r\n\r\n";
    int client_fd = this->return_socket_des();
    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
>>>>>>> Stashed changes
    connectTunnel(socket_server);
    //Finish connect
    return;
  }
  else if (request->return_method() == "POST") {
    /*Receive the request, send to server
    Receive the response, send to client
    */
    int socket_server = connectServer();
    int socket_client = socket_des;
    send(socket_server,
         request->return_Line().c_str(),
         strlen(request->return_Line().c_str()),
         0);
    std::string input = recvAll(socket_server);
    if (input.size() == 0) {
      return;
    }
    send(socket_client, input.c_str(), strlen(input.c_str()), 0);
    close(socket_client);
    close(socket_server);
    return;
  }
  else {
    //Thre request is GET
    return;
  }
}

int Proxy::connectServer() {
  int error;
  int socket_server;
  // std::cout << "Request target server ip: " << request->return_Host().c_str()
  //           << std::endl;
  // std::cout << "target port: " << request->return_port().c_str() << std::endl;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  error = getaddrinfo(
      request->return_Host().c_str(), request->return_port().c_str(), &hints, &res);
  if (error != 0) {
    std::cerr << "In connection: ";
    std::cerr << "Error: cannot get the address info from the host " << std::endl;
    pthread_exit(NULL);
  }
  socket_server = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (socket_server == -1) {
    std::cerr << "Error: cannot create socket for " << std::endl;
    pthread_exit(NULL);
  }
  error = connect(socket_server, res->ai_addr, res->ai_addrlen);
  if (error == -1) {
    std::cerr << "Error cannot connect to the socket" << std::endl;
    pthread_exit(NULL);
  }
  freeaddrinfo(res);
  return socket_server;
}

void Proxy::connectTunnel(int socket_server) {
  // int client_fd = this->return_socket_des();
  // send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  usleep(10);
  fd_set listen_ports;
  int socket_client = this->return_socket_des();
  int error;
  error = send(socket_client, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  if (error < 0) {
    cerr << "Cannot send back" << std::endl;
    return;
  }
  int maxdes;
  // int socket_client = request->return_socket_des();
  if (socket_client > socket_server) {
    maxdes = socket_client;
  }
  else {
    maxdes = socket_server;
  }
  int end = 1;
  while (end) {
    FD_ZERO(&listen_ports);
    FD_SET(socket_client, &listen_ports);
    FD_SET(socket_server, &listen_ports);
    // temp_list = listen_ports;
    std::cout << "start select listen on socket:" << request->return_socket_des()
              << std::endl;
    error = select(maxdes + 1, &listen_ports, NULL, NULL, NULL);
    std::cout << "finish select" << std::endl;
    if (error == -1) {
      std::cerr << "Error cannot select" << std::endl;
      pthread_exit(NULL);
    }
    for (int i = 0; i <= maxdes; i++) {
      if (FD_ISSET(i, &listen_ports)) {
        //There is a message received
        int data_rec;
        int increment = 65536;
        int start = 0;
        std::vector<char> data_buff(increment, 0);
        data_rec = recv(i, &data_buff.data()[start], increment, 0);
        data_buff.resize(data_rec);
        // std::string input = recvAll(i);
        if (data_buff.size() == 0) {
          //One of the socket closed on there side
          // close(socket_server);
          // close(socket_client);
          // FD_ZERO(&listen_ports);
          // end = 0;
          return;
        }
        else {
          std::cout << "There is message received" << std::endl;
          if (i == socket_server) {
            //The message is from server
            send(socket_client, &data_buff.data()[0], data_buff.size(), 0);
          }
          else {
            //The message is from client
            send(socket_server, &data_buff.data()[0], data_buff.size(), 0);
          }
        }  //End of if for recv 0
      }    //End of if for check fd_set
    }      //End of go over the potential descriptor
  }        //main while loop
  return;  //End of func
}

void Proxy::handleConnect(int server_fd) {
  int client_fd = this->return_socket_des();
  // send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  fd_set readfds;
  int nfds = server_fd > client_fd ? server_fd + 1 : client_fd + 1;

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    FD_SET(client_fd, &readfds);

    select(nfds, &readfds, NULL, NULL, NULL);
    int fd[2] = {server_fd, client_fd};
    int len;
    for (int i = 0; i < 2; i++) {
      char message[65536] = {0};
      if (FD_ISSET(fd[i], &readfds)) {
        len = recv(fd[i], message, sizeof(message), 0);
        if (len <= 0) {
          return;
        }
        else {
          if (send(fd[1 - i], message, len, 0) <= 0) {
            return;
          }
        }
      }
    }
  }
}