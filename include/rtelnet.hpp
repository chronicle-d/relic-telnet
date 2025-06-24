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

#include <iostream>
#include "debug_helpers.hpp"

inline constexpr int RTELNET_PORT = 23;
inline constexpr int RTELNET_BUFFER_SIZE = 1024;
inline constexpr int RTELNET_IP_VERSION = 4;


// 0 | 200 > 210 : Relic telnet
inline constexpr int RTELNET_SUCCESS                        = 0;

// From 1 to 199 - errno errors

// 210 > : TCP connection errors.
inline constexpr int RTELNET_TCP_ERROR_ADDRESS_NOT_VALID    = 210;
inline constexpr int RTELNET_TCP_ERROR_CANNOT_ALOCATE_FD    = 211;
inline constexpr int RTELNET_TCP_ERROR_CONNECTION_CLOSED_R  = 212;
inline constexpr int RTELNET_TCP_ERROR_NOT_CONNECTED        = 213;
inline constexpr int RTELNET_TCP_ERROR_FAILED_SEND          = 214;
inline constexpr int RTELNET_TCP_ERROR_PARTIAL_SEND         = 215;

// 300 > : Telnet logic errors.
inline constexpr int RTELNET_ERROR_NOT_A_NEGOTIATION        = 300;


namespace rtnt {

  inline constexpr unsigned char IAC                    = 255; // Interpret As Command
  inline constexpr unsigned char DO                     = 253; // Please use this option
  inline constexpr unsigned char DONT                   = 254; // Please don’t use this option
  inline constexpr unsigned char WILL                   = 251; // I will use this option
  inline constexpr unsigned char WONT                   = 252; // I won’t use this option
  inline constexpr unsigned char SB                     = 250; // Begin subnegotiation
  inline constexpr unsigned char SE                     = 240; // End subnegotiation
  inline constexpr unsigned char BINARY                 = 0;   // Binary transmission (8-bit clean communication)
  inline constexpr unsigned char ECHO                   = 1;   // Echo input back to sender (commonly server-side)
  inline constexpr unsigned char SGA                    = 3;   // Suppress Go Ahead (stream mode instead of line mode)
  inline constexpr unsigned char STATUS                 = 5;   // Query or send current option status
  inline constexpr unsigned char TIMING_MARK            = 6;   // Timing mark for synchronization
  inline constexpr unsigned char TERMINAL_TYPE          = 24;  // Exchange terminal type (e.g., "ANSI", "VT100")
  inline constexpr unsigned char NAWS                   = 31;  // Negotiate About Window Size (send terminal size)
  inline constexpr unsigned char LINEMODE               = 34;  // Line-by-line input mode negotiation
  inline constexpr unsigned char NEW_ENVIRON            = 39;  // Send environment variables (supersedes option 36)
  inline constexpr unsigned char X_DISPLAY_LOCATION     = 35;  // Send X11 DISPLAY value (e.g., ":0")
  inline constexpr unsigned char LOGOUT                 = 18;  // Server requests client logout
  inline constexpr unsigned char ENVIRONMENT_OPTION     = 36;  // Deprecated method to send environment variables
  inline constexpr unsigned char AUTHENTICATION         = 37;  // Authenticate user via a scheme (e.g., Kerberos)
  inline constexpr unsigned char ENCRYPTION             = 38;  // Encrypt the Telnet stream
  inline constexpr unsigned char RCP                    = 2;   // Remote Controlled Port (obsolete)
  inline constexpr unsigned char NAMS                   = 4;   // Negotiate Approximate Message Size (rare)
  inline constexpr unsigned char RCTE                   = 7;   // Remote Controlled Transmission and Echo
  inline constexpr unsigned char NAOL                   = 8;   // Output line width setting
  inline constexpr unsigned char NAOP                   = 9;   // Output page size
  inline constexpr unsigned char NAOCRD                 = 10;  // Carriage return disposition
  inline constexpr unsigned char NAOHTS                 = 11;  // Horizontal tab stops
  inline constexpr unsigned char NAOHTD                 = 12;  // Horizontal tab disposition
  inline constexpr unsigned char NAOFFD                 = 13;  // Formfeed disposition
  inline constexpr unsigned char NAOVTS                 = 14;  // Vertical tab stops
  inline constexpr unsigned char NAOVTD                 = 15;  // Vertical tab disposition
  inline constexpr unsigned char NAOLFD                 = 16;  // Linefeed disposition
  inline constexpr unsigned char EXTEND_ASCII           = 17;  // Extended ASCII character set support
  inline constexpr unsigned char BM                     = 19;  // Byte macro (macros for command sequences)
  inline constexpr unsigned char DET                    = 20;  // Data Entry Terminal mode
  inline constexpr unsigned char SUPDUP                 = 21;  // MIT SUPDUP protocol support
  inline constexpr unsigned char SUPDUP_OUTPUT          = 22;  // SUPDUP output extension
  inline constexpr unsigned char SEND_LOCATION          = 23;  // Send geographic location
  inline constexpr unsigned char END_OF_RECORD          = 25;  // Logical end-of-record marker
  inline constexpr unsigned char TACACS_UID             = 26;  // User identification via TACACS
  inline constexpr unsigned char OUTPUT_MARKING         = 27;  // Marks screen output boundaries
  inline constexpr unsigned char TTYLOC                 = 28;  // Send terminal location (TTYLOC)
  inline constexpr unsigned char REMOTE_FLOW_CONTROL    = 29;  // Enable/disable flow control remotely
  inline constexpr unsigned char XAUTH                  = 41;  // X Window System authentication
  inline constexpr unsigned char CHARSET                = 42;  // Negotiate character set
  inline constexpr unsigned char RSP                    = 43;  // Remote serial port control
  inline constexpr unsigned char COM_PORT_CONTROL       = 44;  // Advanced serial port control
  inline constexpr unsigned char SUPPRESS_LOCAL_ECHO    = 45;  // Don't locally echo what we type
  inline constexpr unsigned char START_TLS              = 46;  // Upgrade connection to TLS (STARTTLS)
  inline constexpr unsigned char KERMIT                 = 47;  // File transfer via Kermit protocol
  inline constexpr unsigned char SEND_URL               = 48;  // Send URL string to client
  inline constexpr unsigned char FORWARD_X              = 49;  // Forward X11 connections
  inline constexpr unsigned char TERMINAL_SPEED         = 32;  // Set terminal baud rate
  inline constexpr unsigned char TOGGLE_FLOW_CONTROL    = 33;  // Obsolete; similar to REMOTE_FLOW_CONTROL
  inline constexpr unsigned char X3_PAD                 = 30;  // Transmit X.3 PAD parameters
  inline constexpr unsigned char MSDP                   = 69;  // Mud Server Data Protocol (used in MUDs)
  inline constexpr unsigned char MSSP                   = 70;  // Mud Server Status Protocol
  inline constexpr unsigned char ZMP                    = 93;  // Zenith Mud Protocol
  inline constexpr unsigned char MUX                    = 95;  // Legacy multi-session support
  inline constexpr unsigned char MCCP1                  = 85;  // MUD Client Compression Protocol v1
  inline constexpr unsigned char MCCP2                  = 86;  // MUD Client Compression Protocol v2
  inline constexpr unsigned char GMCP                   = 201; // Generic Mud Communication Protocol
  inline constexpr unsigned char PRAGMA_LOGON           = 138; // Used in Microsoft Telnet (may be in private range)
  inline constexpr unsigned char SSPI_LOGON             = 139; // SSPI-based login (Microsoft)
  inline constexpr unsigned char PRAGMA_HEARTBEAT       = 140; // Keep-alive negotiation

  std::string readError(int rtntErrno) {
    if (rtntErrno < 200) { return strerror(errno); }
    switch (rtntErrno) {
      case RTELNET_SUCCESS: return                            "No error.";
      case RTELNET_TCP_ERROR_ADDRESS_NOT_VALID: return        "address is not valid.";
      case RTELNET_TCP_ERROR_CANNOT_ALOCATE_FD: return        "cannot alocate a file descriptor.";
      case RTELNET_TCP_ERROR_CONNECTION_CLOSED_R: return      "Connection closed by remote.";
      case RTELNET_TCP_ERROR_NOT_CONNECTED: return            "connection failed, tcp session was not established.";
      case RTELNET_TCP_ERROR_FAILED_SEND: return              "could not send message.";
      case RTELNET_TCP_ERROR_PARTIAL_SEND: return             "message was sent partially.";

      // Telnet logic errors
      case RTELNET_ERROR_NOT_A_NEGOTIATION: return            "a negotiation was called, yet server did not negotiate.";
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

  class session {
  public:
    int _port    = RTELNET_PORT;
    const char* _address;
    int _ipv     = RTELNET_IP_VERSION;
    std::string _username;
    std::string _password;
    bool _connected = false;
    int _fd;

    class tcp {
    public:
      tcp(session* owner) : _owner(owner) {}

      Result<sockaddr_in, int> getSocketAddr(const char* address, int port = RTELNET_PORT, int ipVersion = RTELNET_IP_VERSION) const {
        struct sockaddr_in server_address;
        server_address.sin_family = (ipVersion == 4) ? AF_INET : AF_INET6;
        server_address.sin_port = htons(port);

        if (inet_pton(AF_INET, address, &server_address.sin_addr) <= 0) {
          return RTELNET_TCP_ERROR_ADDRESS_NOT_VALID;
        }

        return server_address;
      }

      unsigned int Connect(sockaddr_in& address, int ipVersion = RTELNET_IP_VERSION) {
        int sockfd = socket((ipVersion == 4) ? AF_INET : AF_INET6, SOCK_STREAM, 0);
        if (sockfd < 0) return RTELNET_TCP_ERROR_CANNOT_ALOCATE_FD;

        errno = 0;
        if (connect(sockfd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) { return errno; }

        _owner->_connected = true;
        return sockfd;
      }

      void Close(int sockfd) {
        close(sockfd);
        _owner->_connected = false;
      }

      unsigned int Send(const std::vector<unsigned char>& message, int sockfd, int sendFlag = 0) const {
        if (!_owner->_connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;

        errno = 0;
        ssize_t bytesSent = send(sockfd, message.data(), message.size(), sendFlag);

        if (bytesSent == 0) return RTELNET_TCP_ERROR_FAILED_SEND;
        if (static_cast<size_t>(bytesSent) != message.size()) return RTELNET_TCP_ERROR_PARTIAL_SEND;
        if (bytesSent < 0) return errno;

        return RTELNET_SUCCESS;
      }

      Result<std::vector<unsigned char>, int> Read(int socketfd, int bufferSize = RTELNET_BUFFER_SIZE, int recvFlag = 0) const {
        if (!_owner->_connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;

        std::vector<unsigned char> buffer(bufferSize);
        errno = 0;
        ssize_t bytesRead = recv(socketfd, reinterpret_cast<char*>(buffer.data()), buffer.size(), recvFlag);

        if (bytesRead < 0) return errno;
        if (bytesRead == 0) return RTELNET_TCP_ERROR_CONNECTION_CLOSED_R;

        buffer.resize(bytesRead);
        return buffer;
      }

    private:
      session* _owner;
    };

    // Constructor that initializes tcp and passes this pointer
    session() : _tcp(this) {}

    tcp _tcp;
  };

  unsigned int Negotiate(session& session) {
    if (!session._connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;

    bool negotiatedSomething = false;

    while (true) {

      // Peek in the buffer
      auto bufferPeekResult = session._tcp.Read(session._fd, 3, MSG_PEEK);
      if (bufferPeekResult.is_err()) { return bufferPeekResult.error(); }
      const auto& bufferPeek = bufferPeekResult.value();

      printTelnet(bufferPeek, 1);

      ssize_t n = static_cast<ssize_t>(bufferPeek.size());
      if (n < 3) return errno;

      // If not a negotiation packet, exit
      if (bufferPeek[0] != IAC) {
        if (!negotiatedSomething) {
          return RTELNET_ERROR_NOT_A_NEGOTIATION;
        } else {
          break;
        }
      }

      // Read the full 3-byte sequence
      auto bufferResult = session._tcp.Read(session._fd, 3);
      if (bufferResult.is_err()) { return bufferResult.error(); }
      const auto& buffer = bufferResult.value();

      unsigned char command = buffer[1];
      unsigned char option  = buffer[2];
      std::vector<unsigned char> response = {
        static_cast<unsigned char>(IAC),
        static_cast<unsigned char>(0),
        static_cast<unsigned char>(option)
      };

      switch (command) {
        case DO:
          switch (option) {
            case BINARY:               response[1] = WONT; break;
            case ECHO:                 response[1] = WONT; break;
            case SGA:                  response[1] = WONT; break;
            case STATUS:               response[1] = WONT; break;
            case TIMING_MARK:          response[1] = WONT; break;
            case TERMINAL_TYPE:        response[1] = WONT; break;
            case NAWS:                 response[1] = WONT; break;
            case LINEMODE:             response[1] = WONT; break;
            case NEW_ENVIRON:          response[1] = WONT; break;
            case X_DISPLAY_LOCATION:   response[1] = WONT; break;
            case LOGOUT:               response[1] = WONT; break;
            case ENVIRONMENT_OPTION:   response[1] = WONT; break;
            case AUTHENTICATION:       response[1] = WONT; break;
            case ENCRYPTION:           response[1] = WONT; break;
            case RCP:                  response[1] = WONT; break;
            case NAMS:                 response[1] = WONT; break;
            case RCTE:                 response[1] = WONT; break;
            case NAOL:                 response[1] = WONT; break;
            case NAOP:                 response[1] = WONT; break;
            case NAOCRD:               response[1] = WONT; break;
            case NAOHTS:               response[1] = WONT; break;
            case NAOHTD:               response[1] = WONT; break;
            case NAOFFD:               response[1] = WONT; break;
            case NAOVTS:               response[1] = WONT; break;
            case NAOVTD:               response[1] = WONT; break;
            case NAOLFD:               response[1] = WONT; break;
            case EXTEND_ASCII:         response[1] = WONT; break;
            case BM:                   response[1] = WONT; break;
            case DET:                  response[1] = WONT; break;
            case SUPDUP:               response[1] = WONT; break;
            case SUPDUP_OUTPUT:        response[1] = WONT; break;
            case SEND_LOCATION:        response[1] = WONT; break;
            case END_OF_RECORD:        response[1] = WONT; break;
            case TACACS_UID:           response[1] = WONT; break;
            case OUTPUT_MARKING:       response[1] = WONT; break;
            case TTYLOC:               response[1] = WONT; break;
            case REMOTE_FLOW_CONTROL:  response[1] = WONT; break;
            case TOGGLE_FLOW_CONTROL:  response[1] = WONT; break;
            case X3_PAD:               response[1] = WONT; break;
            case MSDP:                 response[1] = WONT; break;
            case MSSP:                 response[1] = WONT; break;
            case ZMP:                  response[1] = WONT; break;
            case MUX:                  response[1] = WONT; break;
            case MCCP1:                response[1] = WONT; break;
            case MCCP2:                response[1] = WONT; break;
            case GMCP:                 response[1] = WONT; break;
            case PRAGMA_LOGON:         response[1] = WONT; break;
            case SSPI_LOGON:           response[1] = WONT; break;
            case PRAGMA_HEARTBEAT:     response[1] = WONT; break;
            default:                   response[1] = WONT; break;
          }
          break;

        case WILL:
          switch (option) {
            case BINARY:               response[1] = DONT; break;
            case ECHO:                 response[1] = DONT; break;
            case SGA:                  response[1] = DONT; break;
            case STATUS:               response[1] = DONT; break;
            case TIMING_MARK:          response[1] = DONT; break;
            case TERMINAL_TYPE:        response[1] = DONT; break;
            case NAWS:                 response[1] = DONT; break;
            case LINEMODE:             response[1] = DONT; break;
            case NEW_ENVIRON:          response[1] = DONT; break;
            case X_DISPLAY_LOCATION:   response[1] = DONT; break;
            case LOGOUT:               response[1] = DONT; break;
            case ENVIRONMENT_OPTION:   response[1] = DONT; break;
            case AUTHENTICATION:       response[1] = DONT; break;
            case ENCRYPTION:           response[1] = DONT; break;
            case RCP:                  response[1] = DONT; break;
            case NAMS:                 response[1] = DONT; break;
            case RCTE:                 response[1] = DONT; break;
            case NAOL:                 response[1] = DONT; break;
            case NAOP:                 response[1] = DONT; break;
            case NAOCRD:               response[1] = DONT; break;
            case NAOHTS:               response[1] = DONT; break;
            case NAOHTD:               response[1] = DONT; break;
            case NAOFFD:               response[1] = DONT; break;
            case NAOVTS:               response[1] = DONT; break;
            case NAOVTD:               response[1] = DONT; break;
            case NAOLFD:               response[1] = DONT; break;
            case EXTEND_ASCII:         response[1] = DONT; break;
            case BM:                   response[1] = DONT; break;
            case DET:                  response[1] = DONT; break;
            case SUPDUP:               response[1] = DONT; break;
            case SUPDUP_OUTPUT:        response[1] = DONT; break;
            case SEND_LOCATION:        response[1] = DONT; break;
            case END_OF_RECORD:        response[1] = DONT; break;
            case TACACS_UID:           response[1] = DONT; break;
            case OUTPUT_MARKING:       response[1] = DONT; break;
            case TTYLOC:               response[1] = DONT; break;
            case REMOTE_FLOW_CONTROL:  response[1] = DONT; break;
            case TOGGLE_FLOW_CONTROL:  response[1] = DONT; break;
            case X3_PAD:               response[1] = DONT; break;
            case MSDP:                 response[1] = DONT; break;
            case MSSP:                 response[1] = DONT; break;
            case ZMP:                  response[1] = DONT; break;
            case MUX:                  response[1] = DONT; break;
            case MCCP1:                response[1] = DONT; break;
            case MCCP2:                response[1] = DONT; break;
            case GMCP:                 response[1] = DONT; break;
            case PRAGMA_LOGON:         response[1] = DONT; break;
            case SSPI_LOGON:           response[1] = DONT; break;
            case PRAGMA_HEARTBEAT:     response[1] = DONT; break;
            default:                   response[1] = DONT; break;
          }
          break;

        case WONT:
        case DONT:
          // Add supprt for these later.
          break;
      }

      session._tcp.Send(response, session._fd);
      negotiatedSomething = true;
      printTelnet(response, 0);
    }

    return RTELNET_SUCCESS;
  }

  unsigned int Connect(session& session) {    
    // Get address
    auto addressResult = session._tcp.getSocketAddr(session._address, session._port,session._ipv);
    if (addressResult.is_err()) { return addressResult.error(); }
    sockaddr_in address = addressResult.value();

    int fd = session._tcp.Connect(address);
    if (fd < 0) { return fd; }
    session._fd = fd;
    
    int negotiateStatus = Negotiate(session);
    if (negotiateStatus != 0) return negotiateStatus;

    std::cout << "Done.\n";

    return RTELNET_SUCCESS;
  }
}
#endif // RTELNET_H
