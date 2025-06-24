/*
 * Relic Telnet is a header only library.
 * This file is only to test the functionality of rtelnet.
 */

#include "rtelnet.hpp"

#include <exception>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "[ERROR]: Usage: " << argv[0] << " ADDRESS PORT \n";
    return 1;
  }
 
  rtnt::session Session;
  int fd;

  Session._address = argv[1];
  Session._port = std::atoi(argv[2]);

  try {
    fd = rtnt::Connect(Session);
    if (fd != 0) {
      std::cerr << "[ERROR]: " << rtnt::readError(fd) << "\n";
      return 1;
    }
  } catch (std::exception &e) {
    std::cerr << "[ERROR]: " << e.what() << "\n";
  }


  // while (true) {
  //   try {
  //
  //     std::cout << "> ";
  //     std::string message;
  //     getline(std::cin, message);
  //
  //     if (message == "quit") { break; }
  //
  //     int status = Session._tcp.Send(message + "\n", fd);
  //     if (status != 0) {
  //         std::cerr << "[ERROR]: " << rtnt::readError(status) << "\n";
  //         return 1;
  //     }
  //
  //     std::string buffer = rtnt::unwrapOrThrow(Session._tcp.Read(fd));
  //     std::cout << "< " << buffer;
  //
  //   } catch (std::exception &e) {
  //     std::cerr << "[ERROR]: " << e.what() << "\n";
  //   }
  // }
  //
  Session._tcp.Close(fd);
  return 0;
}
