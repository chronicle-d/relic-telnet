#include "rtelnet.hpp"
#include <iostream>

using namespace rtnt;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "[ERROR]: Usage: " << argv[0] << " ADDRESS PORT \n";
    return 1;
  }

  session Session(argv[1], "example", "124", std::atoi(argv[2]));

  unsigned int connectSuccess = Session.Connect();
  if (connectSuccess != RTELNET_SUCCESS) {
    std::cerr << "[CONNECTION_ERROR]: " << readError(connectSuccess) << "\n";
    Session.throwErrorStack();
    return 1;
  }

  std::string buffer;

  // Flush the banner
  unsigned int fbSuccess = Session.Execute("\n", buffer);
  if (fbSuccess != RTELNET_SUCCESS) {
    std::cerr << "[EXEC_ERROR]: " << rtnt::readError(fbSuccess) << "\n";
    Session.throwErrorStack();
    return 1;
  }

  // Execute command
  unsigned int execSuccess = Session.Execute("ls -atl /demo", buffer);
  if (execSuccess != RTELNET_SUCCESS) {
    std::cerr << "[EXEC_ERROR]: " << rtnt::readError(execSuccess) << "\n";
    Session.throwErrorStack();
    return 1;
  }

  std::cout << buffer << "\n";
  return 0;
}
