/*
* Relic Telnet is a header only telnet client implementation.
*/
#ifndef RTELNET_H
#define RTELNET_H

#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include <variant>
#include <unistd.h>
#include <vector>
#include <stdexcept>

inline constexpr int RTELNET_PORT = 23;
inline constexpr int RTELNET_BUFFER_SIZE = 1024;
inline constexpr int RTELNET_IP_VERSIN = 4;


// 0 | 200 > 210 : Relic telnet
inline constexpr int RTELNET_SUCCESS = 0;

// From 1 to 199 - errno errors

// 210 > : TCP connection errors.
inline constexpr int RTELNET_TCP_ERROR_ADDRESS_NOT_VALID = 210;
inline constexpr int RTELNET_TCP_ERROR_CANNOT_ALOCATE_FD = 211;
inline constexpr int RTELNET_TCP_ERROR_CANNOT_CONNECT    = 212;
inline constexpr int RTELNET_TCP_ERROR_NOT_CONNECTED     = 213;
inline constexpr int RTELNET_TCP_ERROR_FAILED_SEND       = 214;
inline constexpr int RTELNET_TCP_ERROR_PARTIAL_SEND      = 215;


namespace rtnt {

  std::string readError(int rtntErrno) {
    if (rtntErrno < 200) { return strerror(errno); }
    switch (rtntErrno) {
      case RTELNET_SUCCESS: return                            "No error.";
      case RTELNET_TCP_ERROR_ADDRESS_NOT_VALID: return        "address is not valid.";
      case RTELNET_TCP_ERROR_CANNOT_ALOCATE_FD: return        "cannot alocate a file descriptor.";
      case RTELNET_TCP_ERROR_CANNOT_CONNECT: return           "faild connection to server."; // Unused currenly.
      case RTELNET_TCP_ERROR_NOT_CONNECTED: return            "operation failed, tcp connection was not established.";
      case RTELNET_TCP_ERROR_FAILED_SEND: return              "could not send message.";
      case RTELNET_TCP_ERROR_PARTIAL_SEND: return             "message was sent partially.";
      default: return                                         "Unknonw error.";
    }
  }


  template<typename T, typename E>
  class Result {
    public:
      Result(const T& value) : _value(value) {}
      Result(const E& error) : _value(error) {}

      bool is_ok() const { return std::holds_alternative<T>(_value); }
      bool is_err() const { return std::holds_alternative<E>(_value); }

      T& value() { return std::get<T>(_value); }
      E& error() { return std::get<E>(_value); }

    private:
      std::variant<T, E> _value;
  };

  template<typename T, typename E>
  T unwrapOrThrow(rtnt::Result<T, E>&& result) {
      if (result.is_err())
          throw std::runtime_error(readError(result.error()));
      return result.value();
  }

  class tcp {
    public:
      bool _connected = false;

      Result<sockaddr_in, int> getSocketAddr(const char* address, int port = RTELNET_PORT, int ipVersion = RTELNET_IP_VERSIN) const {

        struct sockaddr_in server_address;

        server_address.sin_family = (ipVersion == 4) ? AF_INET : AF_INET6;
        server_address.sin_port = htons(port);

        if (inet_pton(AF_INET, address, &server_address.sin_addr) <= 0) {
            return RTELNET_TCP_ERROR_ADDRESS_NOT_VALID;
        }

        return server_address;
      }

      unsigned int Connect(sockaddr_in& address, int ipVersion = RTELNET_IP_VERSIN) {

        int sockfd = 0;

        if ((sockfd = socket((ipVersion == 4) ? AF_INET : AF_INET6, SOCK_STREAM, 0)) < 0) {
            return RTELNET_TCP_ERROR_CANNOT_ALOCATE_FD;
        }

        if (connect(sockfd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
            return errno;
        }

        _connected = true;

        return sockfd;
      }

      void Close(int sockfd) {
        close(sockfd);
        _connected = false;
      }

      unsigned int Send(const std::string& message, int sockfd) const {
        if (!_connected) { return RTELNET_TCP_ERROR_NOT_CONNECTED; }
        
        errno = 0;
        ssize_t bytesSent = send(sockfd, message.c_str(), message.size(), 0);

        if (bytesSent == 0) { return RTELNET_TCP_ERROR_FAILED_SEND; }
        if (static_cast<size_t>(bytesSent) != message.size()) { return RTELNET_TCP_ERROR_PARTIAL_SEND; }
        if (bytesSent < 0) { return errno; }

        return RTELNET_SUCCESS;
      }

      Result<std::string, int> Read(int socketfd, int bufferSize = RTELNET_BUFFER_SIZE) const {
        if (!_connected) { return RTELNET_TCP_ERROR_NOT_CONNECTED; }

        std::vector<char> buffer(bufferSize);
        errno = 0;
        ssize_t bytesRead = ::read(socketfd, buffer.data(), buffer.size());
        
        if (bytesRead < 0) { return errno; }

        std::string result(buffer.begin(), buffer.begin() + bytesRead);

        return result;
      }
  };
}
#endif // RTELNET_H
