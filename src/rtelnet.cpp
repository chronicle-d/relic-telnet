/*
 * Relic Telnet is a header only library.
 * This file is only to test the functionality of rtelnet.
 */

#include "rtelnet.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "[ERROR]: Usage: " << argv[0] << " ADDRESS PORT \n";
    return 1;
  }

  rtnt::session Session(argv[1], "example", "[nM3r2>W31S_", std::atoi(argv[2]));

  unsigned int connectSuccess = Session.Connect();
  if (connectSuccess != RTELNET_SUCCESS) { std::cerr << "[CONNECTION_ERROR]: " << rtnt::readError(connectSuccess) << "\n"; return 1; }

  unsigned int flushSuccess = Session.FlushBanner();
  if (flushSuccess != RTELNET_SUCCESS) { std::cerr << "[FLUSH_BANNER_ERROR]: " << rtnt::readError(flushSuccess) << "\n"; return 1; }

  std::string buffer;
  unsigned int execSuccess = Session.Execute("ls -al", buffer);
  if (execSuccess != RTELNET_SUCCESS) { std::cerr << "[EXEC_ERROR]: " << rtnt::readError(execSuccess) << "\n"; return 1; }

  std::cout << buffer << "\n";

  return 0;
}
