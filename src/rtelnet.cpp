/*
 * Minimal Telnet Client using Relic Telnet
*/

#include "rtelnet.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>

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

  rtnt::session session(address.c_str(), username, password, port, 4, 2);

  unsigned int connectionStatus = session.Connect();

  if (connectionStatus != RTELNET_SUCCESS) {
    std::cerr << "Failed to connect. (" + rtnt::readError(connectionStatus) + ")\n";
  }

  std::cout << "[Connected] — type 'exit' to quit.\n";

  std::atomic<bool> done{false};

  // Thread to read server output
  std::thread reader([&]() {
    std::vector<unsigned char> buffer;
    while (!done) {
      unsigned int status = session.Read(buffer, 1024, 0);
      if (status == RTELNET_SUCCESS && !buffer.empty()) {
        std::string output(buffer.begin(), buffer.end());
        std::cout << output << std::flush;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  // Main loop to read user input
  std::string input;
  while (true) {
    std::getline(std::cin, input);
    if (input == "exit") break;

    unsigned int status = session.Execute(input, input); // re-use same string as output buffer
    if (status != RTELNET_SUCCESS) {
      std::cerr << "Command failed: " << rtnt::readError(status) << "\n";
    }
  }

  done = true;
  if (reader.joinable()) reader.join();

  return 0;
}
