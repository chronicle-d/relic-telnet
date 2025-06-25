/*
 * Relic Telnet is a header only library.
 * This file is only to test the functionality of rtelnet.
 */

#include "rtelnet.hpp"

#include <exception>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "[ERROR]: Usage: " << argv[0] << " ADDRESS PORT \n";
    return 1;
  }

  rtnt::session Session;

  Session._address = argv[1];
  Session._port = std::atoi(argv[2]);
  Session._username = "toker";
  Session._password = "[nM3r2>W31S_";

  try {
    unsigned int connectSuccess = Session.Connect();
    if (connectSuccess != RTELNET_SUCCESS) { std::cerr << "[CONNECTION_ERROR]: " << rtnt::readError(connectSuccess) << "\n"; return 1; }

    unsigned int loginSuccess = Session.Login();
    if (loginSuccess != RTELNET_SUCCESS) { std::cerr << "[LOGGIN_ERROR]: " << rtnt::readError(loginSuccess) << "\n"; return 1; }

    unsigned int flushSuccess = Session.FlushBanner();
    if (flushSuccess != RTELNET_SUCCESS) { std::cerr << "[FLUSH_BANNER_ERROR]: " << rtnt::readError(loginSuccess) << "\n"; return 1; }

    std::string buffer;
    unsigned int execSuccess = Session.Execute("ls -al", buffer);
    if (execSuccess != RTELNET_SUCCESS) { std::cerr << "[EXEC_ERROR]: " << rtnt::readError(loginSuccess) << "\n"; return 1; }

    std::cout << buffer << "\n";

  } catch (std::exception &e) {
    std::cerr << "[ERROR]: " << e.what() << "\n";
  }

  Session._tcp.Close(Session._fd);
  return 0;
}
