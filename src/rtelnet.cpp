#include "rtelnet.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
    return 1;
  }

  std::string address = argv[1];
  int port = std::atoi(argv[2]);

  std::string username, password;
  std::cout << "Username: ";
  std::getline(std::cin, username);
  std::cout << "Password: ";
  std::getline(std::cin, password);

  rtnt::session session(address.c_str(), username, password, port, 4, 0);
  unsigned int connectionStatus = session.Connect();

  if (session.isBackgroundError() || connectionStatus != RTELNET_SUCCESS) {
    std::cerr << "Failed to connect. (" << rtnt::readError(connectionStatus) << ")\n";
    std::cerr << "Background error: (" << rtnt::readError(session.getBackgroundError()) << ")\n";
    return 1;
  }

  session.FlushBanner();
  

  while (true) {
    std::string input;
    std::string output;
    std::cout << "$ ";
    std::getline(std::cin, input);
    unsigned int execStatus = session.Execute(input, output);
    if (execStatus != RTELNET_SUCCESS) {
      std::cerr << "Failed to connect. (" << rtnt::readError(connectionStatus) << ")\n";
      std::cerr << "Background error: (" << rtnt::readError(session.getBackgroundError()) << ")\n";
      return 1;
    }
    std::cout << output << std::endl;
  }

  return 0;
}
