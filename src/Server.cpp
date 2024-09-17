#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <future>
#include <chrono>
#include <thread>
#include <mutex>

const std::string ping_msg = "PING";

std::mutex coutMutex, cerrMutex;

// void threadLog(const std::string log);
// void threadErr(const std::string err);

std::vector<std::string> split(const std::string& str, std::string delimiters, int client_fd);
void processTokens(std::vector<std::string> &tokens, int client_fd);
void handleClient(int client_fd);

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  
  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(6379);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 6379\n";
    return 1;
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  //-------Clinet:
  
  std::cout << "Waiting for a clients to connect...\n";

  std::vector< std::future<void> > c_Futures;

  while(true)
  {
    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);

    if (client_fd < 0) 
    {
      std::lock_guard<std::mutex> lock(cerrMutex);
      std::cerr << "Failed to accept connection\n";
      return 1;
    }

    {//Mutex Locking
      std::lock_guard<std::mutex> lock(coutMutex);
      std::cout << "Client connected: " << client_fd << std::endl;
    }

    c_Futures.push_back(std::async(std::launch::async, handleClient, client_fd));
  }

  close(server_fd);

  return 0;
}

std::vector<std::string> split(const std::string& str, std::string delimiters, int client_fd)
{
  {//Mutex Locking
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << client_fd << "  Tokenizing: '" << str << "'...\n\n";
  }

  std::vector<std::string> tokens;

  size_t start = 0;
  size_t end = str.find_first_of(delimiters);

    {//Mutex Locking
      std::lock_guard<std::mutex> lock(coutMutex);
      std::cout << client_fd << "  " << start << " " << end << " " << str.length() << "\n";
    }

  while(end != std::string::npos) 
  {
    if(start != end)
    {
      std::string sub = str.substr(start, end - start);

        {//Mutex Locking
          std::lock_guard<std::mutex> lock(coutMutex);
          std::cout << client_fd << "  Token - " << sub << "\n";
        }

      tokens.push_back(sub);
    }
    start = end+1;
    end = str.find_first_of(delimiters, start);
  }

  if(start < str.length())
  {
    std::string sub = str.substr(start);

    {//Mutex Locking
      std::lock_guard<std::mutex> lock(coutMutex);
      std::cout << client_fd << "  Token - " << sub << "\n";
    }

    tokens.push_back(sub);
  }

  {//Mutex Locking
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << client_fd << "  Tokenization Complete.\n\n";
  }

  return tokens;
}

void processTokens(std::vector<std::string> &tokens, int client_fd) 
{
  {//Mutex Locking
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << client_fd << "  Processing... " << std::endl;
  }

  for(std::string token : tokens)
  { 
    bool requestStatus = std::string(token) == ping_msg;

    {//Mutex Locking
      std::lock_guard<std::mutex> lock(coutMutex);
      std::cout << client_fd << (requestStatus ? "  Accpeted: " : "  Rejected: ") << token << std::endl;
    }

    if(requestStatus)
    {
      std::this_thread::sleep_for(std::chrono::seconds(5)); // sleep for 5 secs
      std::string pong_msg = "+PONG\r\n";
      if(send(client_fd, pong_msg.c_str(), pong_msg.size(), 0) < 0) 
      {
        std::lock_guard<std::mutex> lock(cerrMutex);
        std::cerr << client_fd << "  send(): falied to send message\n";
        return;
      }

      {//Mutex Locking
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << client_fd << "  Sent 'PONG' to client.\n\n";
      }

    } else {
      std::lock_guard<std::mutex> lock(coutMutex);
      std::cout << std::endl;
    }
  }
}

void handleClient(int client_fd) 
{
  char buffer[1024] = {0};
  if(read(client_fd, buffer, sizeof(buffer)) < 0)
  {
    std::lock_guard<std::mutex> lock(cerrMutex);
    std::cerr << client_fd << "  Failed to read message from client\n";
    return;
  }

  {//Mutex Locking
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << client_fd << "  Messaged received from client: " << buffer << std::endl;
  }


  std::string delimiters = " ,\r\n";

  std::vector<std::string> tokens = split(buffer, delimiters, client_fd);

  processTokens(tokens, client_fd);

  close(client_fd);

  {//Mutex Locking
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "Client: " << client_fd << " Closed.\n";
  }

}

// void threadLog(const std::string log) 
// {
//   std::lock_guard<std::mutex> lock(coutMutex);
//   std::cout << log << std::endl;
// }

// void threadErr(const std::string err) 
// {
//   std::lock_guard<std::mutex> lock(cerrMutex);
//   std::cerr << err << std::endl;
// }