#include "rtelnet.hpp"
/*
* Relic Telnet is a header only library, this file tests the functionality of rtelnet.
*/

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc < 2) { std::cerr << "Usage: " << argv[0] << " ADDRESS PORT"; return 1; }

  rtnt::tcp TCP;

  try {
    sockaddr_in serverAddress = rtnt::unwrapOrThrow(TCP.getSocketAddr(argv[1], std::atoi(argv[2])));

    int fd = TCP.Connect(serverAddress);
    if (fd < 0) { throw std::runtime_error(rtnt::readError(fd));}

    std::cout << "Sending...\n";
    TCP.Send("Hello!\n", fd);
    std::cout << "Sent.\n";

    std::cout << "Reading.\n";
    std::string buffer = rtnt::unwrapOrThrow(TCP.Read(fd));
    std::cout << "Got: " << buffer;
    std::cout << "Done.\n";

    TCP.Close(fd);
  } catch (std::exception& e) {
    std::cerr << "[ERROR]: " << e.what() << "\n";
  }

  return 0;
}
