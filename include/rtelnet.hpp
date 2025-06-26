/*
* Relic Telnet is a header only telnet client implementation.
*
* Debug levels:
* 1. Only log each major step. (.i.e Login, Negotiate)
* 2. Add paramets to each major step.
* 3. Login telnet commands & tcp session related major steps.
* 4. Log steps in each major step. (.i.e inside Login, "Waiting for login prompt")
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
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <sys/select.h>
#include <map>

#include "debug_helpers.hpp"

#define LV(x) #x, x

inline constexpr int RTELNET_PORT                           = 23;
inline constexpr int RTELNET_BUFFER_SIZE                    = 1024;
inline constexpr int RTELNET_IP_VERSION                     = 4;
inline constexpr int RTELNET_IDLE_TIMEOUT                   = 1000;
inline constexpr int RTELNET_TOTAL_TIMEOUT                  = 10000;
inline constexpr int RTELNET_DEBUG                          = 0;

// Log titles
inline constexpr const char* RTELNET_LOG_TCP_SET_ADDR = "TCP => SETTING SOCKET ADDRESS";
inline constexpr const char* RTELNET_LOG_TCP_CONNECT = "TCP => CONNECTING TO SOCKET";
inline constexpr const char* RTELNET_LOG_TCP_CLOSE = "TCP => CLOSING SOCKET";
inline constexpr const char* RTELNET_LOG_TCP_SEND_BIN = "TCP => SEND BIN";
inline constexpr const char* RTELNET_LOG_TCP_SEND = "TCP => SEND";
inline constexpr const char* RTELNET_LOG_TCP_READ = "TCP => READ";
inline constexpr const char* RTELNET_LOG_CONNECT = "CONNECT";
inline constexpr const char* RTELNET_LOG_EXECUTE = "EXECITE";
inline constexpr const char* RTELNET_LOG_NEGOTIATE = "NEGOTIATE";
inline constexpr const char* RTELNET_LOG_LOGIN = "LOGIN";













// 0 | 200 > 210 : Relic telnet
inline constexpr int RTELNET_SUCCESS                        = 0;
inline constexpr int RTELNET_ERROR_CANT_FIND_EXPECTED       = 201;

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
inline constexpr int RTELNET_ERROR_NOT_NEGOTIATED           = 301;
inline constexpr int RTELNET_ERROR_USERNAME_NOT_SET         = 302;
inline constexpr int RTELNET_ERROR_PASSWORD_NOT_SET         = 303;
inline constexpr int RTELNET_ERROR_NOT_LOGGED               = 304;
inline constexpr int RTELNET_ERROR_FAILED_LOGIN             = 305;


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

  inline std::string readError(int rtntErrno) {
    if (rtntErrno < 200) { return strerror(errno); }
    switch (rtntErrno) {
      case RTELNET_SUCCESS: return                            "No error.";
      case RTELNET_TCP_ERROR_ADDRESS_NOT_VALID: return        "address is not valid.";
      case RTELNET_TCP_ERROR_CANNOT_ALOCATE_FD: return        "cannot alocate a file descriptor.";
      case RTELNET_TCP_ERROR_CONNECTION_CLOSED_R: return      "Connection closed by remote.";
      case RTELNET_TCP_ERROR_NOT_CONNECTED: return            "connection failed, tcp session was not established.";
      case RTELNET_TCP_ERROR_FAILED_SEND: return              "could not send message. (No errno just 0 bytes sent)";
      case RTELNET_TCP_ERROR_PARTIAL_SEND: return             "message was sent partially.";

      // Telnet logic errors
      case RTELNET_ERROR_NOT_A_NEGOTIATION: return            "a negotiation was called, yet server did not negotiate.";
      case RTELNET_ERROR_NOT_NEGOTIATED: return               "negotiation is required, please negotiate first.";
      case RTELNET_ERROR_NOT_LOGGED: return                   "login is required, please login and try again.";
      case RTELNET_ERROR_FAILED_LOGIN: return                 "login failed, username or password are wrong.";

      // rtelnet specific
      case RTELNET_ERROR_CANT_FIND_EXPECTED: return           "cannot find expected substring in buffer.";
      case RTELNET_ERROR_USERNAME_NOT_SET: return             "username was not set in object.";
      case RTELNET_ERROR_PASSWORD_NOT_SET: return             "password was not set in object.";

      default: return                                         "Unknonw error.";
    }
  }

  class session {
  public:
    int _port = RTELNET_PORT;
    const char* _address;
    int _ipv = RTELNET_IP_VERSION;
    int _debug = RTELNET_DEBUG;
    std::string _username;
    std::string _password;
    int _idle = RTELNET_IDLE_TIMEOUT;
    int _timeout = RTELNET_TOTAL_TIMEOUT;

    session(
      const char* address,
      const std::string username,
      const std::string password,
      int port = RTELNET_PORT,
      int ipv = RTELNET_IP_VERSION,
      int debug = RTELNET_DEBUG,
      int idle = RTELNET_IDLE_TIMEOUT,
      int timeout = RTELNET_TOTAL_TIMEOUT
    ) : 
      _address(address),
      _username(username),
      _password(password),
      _port(port),
      _ipv(ipv),
      _debug(debug),
      _idle(idle),
      _timeout(timeout),
      _tcp(this),
      _logger(this) {}

    ~session() {
      close(_fd);
    }

    class Logger {
    public:
      Logger(session* owner) : _owner(owner) {}

      template <typename T, typename... Rest>
      void log(const std::string& title, const std::string& message, int level,
              const std::string& name1, const T& val1, const Rest&... rest) {
        if (_owner->_debug < level) return;

        std::cout << "\033[1;97m[\033[0m\033[1;96m"
                  << title
                  << "\033[0m\033[1;97m]:\033[0m "
                  << message
                  << " (";
        
        print_named_args(true, name1, val1, rest...);

        std::cout << ")" << std::endl;
      }

      // --- Simple version for just message
      inline void log(const std::string& title, const std::string& message, int level) {
        if (_owner->_debug < level) return;

        std::cout << "\033[1;97m[\033[0m\033[1;96m"
                  << title
                  << "\033[0m\033[1;97m]:\033[0m "
                  << message
                  << std::endl;
      }

    private:
      session* _owner;

      template <typename T>
      void print_named_arg(const std::string& name, const T& value, bool isFirst) {
        if (!isFirst) std::cout << ", ";
        std::cout << name << ": " << value;
      }

      template <typename T>
      void print_named_arg(const std::string& name, const std::vector<T>& value, bool isFirst) {
        if (!isFirst) std::cout << ", ";
        std::cout << name << ": <";
        for (size_t i = 0; i < value.size(); ++i) {
          std::cout << value[i];
          if (i + 1 < value.size()) std::cout << ", ";
        }
        std::cout << ">";
      }

      inline void print_named_arg(const std::string& name, const std::vector<unsigned char>& value, bool isFirst) {
        if (!isFirst) std::cout << ", ";
        std::cout << name << ": <";
        for (size_t i = 0; i < value.size(); ++i) {
          std::cout << static_cast<int>(value[i]);
          if (i + 1 < value.size()) std::cout << ", ";
        }
        std::cout << ">";
      }

      inline void print_named_arg(const std::string& name, const std::string& value, bool isFirst) {
        if (!isFirst) std::cout << ", ";
        std::cout << name << ": \"";
        for (char c : value) {
          switch (c) {
            case '\\': std::cout << "\\\\"; break;
            case '\n': std::cout << "\\n"; break;
            case '\t': std::cout << "\\t"; break;
            case '\r': std::cout << "\\r"; break;
            default: std::cout << c; break;
          }
        }
        std::cout << "\"";
      }



      template <typename K, typename V>
      void print_named_arg(const std::string& name, const std::map<K, V>& value, bool isFirst) {
        if (!isFirst) std::cout << ", ";
        std::cout << name << ": <";
        size_t count = 0;
        for (const auto& [k, v] : value) {
          std::cout << k << ": " << v;
          if (++count < value.size()) std::cout << ", ";
        }
        std::cout << ">";
      }

      inline void print_named_args(bool) {}

      template <typename T, typename... Rest>
      void print_named_args(bool isFirst, const std::string& name, const T& value, const Rest&... rest) {
        print_named_arg(name, value, isFirst);
        print_named_args(false, rest...);
      }
    };


    class tcp {
    public:
      tcp(session* owner) : _owner(owner) {}

      inline unsigned int setSocketAddr(sockaddr_in& server_address) const {
        server_address.sin_family = (_owner->_ipv == 4) ? AF_INET : AF_INET6;
        server_address.sin_port = htons(_owner->_port);

        if (inet_pton(AF_INET, _owner->_address, &server_address.sin_addr) <= 0) {
          return RTELNET_TCP_ERROR_ADDRESS_NOT_VALID;
        }
        
        _owner->_logger.log(RTELNET_LOG_TCP_SET_ADDR, "Successfully set socket address.", 4, LV(_owner->_address), LV(_owner->_port));

        return RTELNET_SUCCESS;
      }

      inline unsigned int Connect(sockaddr_in& address) {
        int sockfd = socket((_owner->_ipv == 4) ? AF_INET : AF_INET6, SOCK_STREAM, 0);
        if (sockfd < 0) return RTELNET_TCP_ERROR_CANNOT_ALOCATE_FD;

        errno = 0;
        if (connect(sockfd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) { return errno; }

        _owner->_logger.log(RTELNET_LOG_TCP_CONNECT, "Successfully connected.", 4, LV(_owner->_address), LV(_owner->_port));
        
        _owner->_connected = true;
        return sockfd;
      }

      void Close() {
        close(_owner->_fd); 
        _owner->_logger.log(RTELNET_LOG_TCP_CLOSE, "Closed socket.", 4);
        _owner->_connected = false;
      }

      inline unsigned int SendBin(const std::vector<unsigned char>& message, int sendFlag = 0) const {
        if (!_owner->_connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;

        errno = 0;
        ssize_t bytesSent = send(_owner->_fd, message.data(), message.size(), sendFlag);

        if (bytesSent == 0) return RTELNET_TCP_ERROR_FAILED_SEND;
        if (static_cast<size_t>(bytesSent) != message.size()) return RTELNET_TCP_ERROR_PARTIAL_SEND;
        if (bytesSent < 0) return errno;

        _owner->_logger.log(RTELNET_LOG_TCP_SEND_BIN, "Successfully sent message.", 4, LV(message), LV(sendFlag));

        return RTELNET_SUCCESS;
      }

      inline unsigned int Send(const std::string& message, int sendFlag = 0) const {
        if (!_owner->_connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;

        std::vector<unsigned char> buffer;
        buffer.reserve(message.size());

        // Escape 255/0xFF
        for (unsigned char c : message) {
          buffer.push_back(c);
          if (c == 255) buffer.push_back(255);
        }

        errno = 0;
        ssize_t bytesSent = send(_owner->_fd, buffer.data(), buffer.size(), sendFlag);

        if (bytesSent == 0) return RTELNET_TCP_ERROR_FAILED_SEND;
        if (bytesSent < 0) return errno;
        if (static_cast<size_t>(bytesSent) != buffer.size()) return RTELNET_TCP_ERROR_PARTIAL_SEND;

        _owner->_logger.log(RTELNET_LOG_TCP_SEND, "Successfully sent message.", 4, LV(message), LV(sendFlag));

        return RTELNET_SUCCESS;
      }

      inline unsigned int Read(std::vector<unsigned char>& buffer, int readSize = RTELNET_BUFFER_SIZE, int recvFlag = 0) const {
        if (!_owner->_connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;

        buffer.resize(readSize);

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(_owner->_fd, &readfds);

        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ready = select(_owner->_fd + 1, &readfds, nullptr, nullptr, &timeout);
        if (ready < 0) return errno;
        if (ready == 0) {
          buffer.clear();
          return RTELNET_SUCCESS;
        }

        errno = 0;
        ssize_t bytesRead = recv(_owner->_fd, reinterpret_cast<char*>(buffer.data()), readSize, recvFlag);

        if (bytesRead < 0) return errno;
        if (bytesRead == 0) return RTELNET_TCP_ERROR_CONNECTION_CLOSED_R;

        buffer.resize(bytesRead);

        _owner->_logger.log(RTELNET_LOG_TCP_READ, "Successfully read.", 4, LV(buffer), LV(readSize), LV(recvFlag));

        return RTELNET_SUCCESS;
      }

    private:
      session* _owner;
    };

    // Read-only accessors
    inline bool isConnected() const { return _connected; }
    inline bool isNegotiated() const { return _negotiated; }
    inline bool isLoggedIn() const { return _logged_in; }

    tcp _tcp;
    Logger _logger;

    inline unsigned int Connect() {

      _logger.log(RTELNET_LOG_CONNECT, "Trying to connnected to telnet server.", 2, LV(_address), LV(_port));

      // Get address
      sockaddr_in address;
      unsigned int addressResult = _tcp.setSocketAddr(address);
      if (addressResult != 0 ) { return addressResult; }

      int fd = _tcp.Connect(address);
      if (fd < 0) { return fd; }
      _fd = fd;
      
      int negotiateStatus = Negotiate();
      if (negotiateStatus != RTELNET_SUCCESS) return negotiateStatus;

      int loginStatus = Login();
      if (loginStatus != RTELNET_SUCCESS) return loginStatus;

      _logger.log(RTELNET_LOG_CONNECT, "Connected to telnet server.", 2, _address, _port);

      return RTELNET_SUCCESS;
    }

    unsigned int Execute(const std::string& command, std::string& buffer) {
      if (!_connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;
      if (!_negotiated) return RTELNET_ERROR_NOT_NEGOTIATED;
      if (!_logged_in) return RTELNET_ERROR_NOT_LOGGED;

      _logger.log(RTELNET_LOG_EXECUTE, "Trying to execute a command.", 2, LV(command));

      unsigned int sendStatus = _tcp.Send(command + "\n");
      if (sendStatus != RTELNET_SUCCESS) return sendStatus;

      std::vector<unsigned char> output;
      buffer.clear();

      auto startTime = std::chrono::steady_clock::now();
      auto lastRead = startTime;

      while (true) {
        unsigned int readStatus = _tcp.Read(output);
        if (readStatus != RTELNET_SUCCESS) return readStatus;

        if (!output.empty()) {
          buffer.append(reinterpret_cast<const char*>(output.data()), output.size());
          lastRead = std::chrono::steady_clock::now();
        }

        auto now = std::chrono::steady_clock::now();
        auto idle = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRead).count();
        auto total = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

        if (idle > _idle || total > _timeout) break;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }

      _logger.log(RTELNET_LOG_EXECUTE, "Executed command successfully.", 2, LV(command));

      return RTELNET_SUCCESS;
    }

    inline unsigned int FlushBanner() {
      if (!_connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;
      if (!_negotiated) return RTELNET_ERROR_NOT_NEGOTIATED;
      if (!_logged_in) return RTELNET_ERROR_NOT_LOGGED;
      
      std::string buffer;
      unsigned int execStatus = Execute("\n", buffer);
      if (execStatus != RTELNET_SUCCESS) return execStatus;

      return RTELNET_SUCCESS;
    }

  private:
    bool _connected = false;
    bool _negotiated = false;
    bool _logged_in = false;
    int _fd;

    unsigned int Negotiate() {
      if (!_connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;

      _logger.log(RTELNET_LOG_NEGOTIATE, "Trying to negotiate.", 2);

      std::vector<unsigned char> buffer;

      while (true) {

        // Peek in the buffer
        unsigned int bufferPeek = _tcp.Read(buffer, 3, MSG_PEEK);
        if (bufferPeek != RTELNET_SUCCESS) { return bufferPeek; }

        // printTelnet(buffer, 1);

        ssize_t n = static_cast<ssize_t>(buffer.size());
        if (n < 3) return errno;

        // If not a negotiation packet, exit
        if (buffer[0] != IAC) {
          if (!_negotiated) {
            return RTELNET_ERROR_NOT_A_NEGOTIATION;
          } else {
            break;
          }
        }

        // Read the full 3-byte sequence
        unsigned int bufferResult = _tcp.Read(buffer, 3);
        if (bufferPeek != RTELNET_SUCCESS) { return bufferPeek; }

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

        _tcp.SendBin(response);
        _negotiated = true;
        // printTelnet(response, 0);
      }

      _logger.log(RTELNET_LOG_NEGOTIATE, "Finished negotiation sequence.", 2);

      return RTELNET_SUCCESS;
    }


    unsigned int expectOutput(const std::string& expect, std::vector<unsigned char>& buffer) {
        if (!_connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;
        if (!_negotiated) return RTELNET_ERROR_NOT_NEGOTIATED;

        for (int i = 0; i < 300; ++i) {
          unsigned int readStatus = _tcp.Read(buffer);
          if (readStatus != RTELNET_SUCCESS) return readStatus;

          std::string cleanedBuffer(reinterpret_cast<const char*>(buffer.data()), buffer.size());

          cleanedBuffer.erase(std::remove(cleanedBuffer.begin(), cleanedBuffer.end(), '\r'), cleanedBuffer.end());
          cleanedBuffer.erase(std::remove(cleanedBuffer.begin(), cleanedBuffer.end(), '\n'), cleanedBuffer.end());

          if (cleanedBuffer.find(expect) != std::string::npos) {
              return RTELNET_SUCCESS;
          }

          std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        return RTELNET_ERROR_CANT_FIND_EXPECTED;
    }

    inline unsigned int Login() {
      if (!_connected) return RTELNET_TCP_ERROR_NOT_CONNECTED;
      if (!_negotiated) return RTELNET_ERROR_NOT_NEGOTIATED;
      if (_username.empty()) return RTELNET_ERROR_USERNAME_NOT_SET;
      if (_password.empty()) return RTELNET_ERROR_PASSWORD_NOT_SET;

      _logger.log(RTELNET_LOG_LOGIN, "Trying to login.", 2, LV(_username), LV(_password));

      std::vector<unsigned char> buffer;

      // Enter login
      buffer.clear();
      unsigned int loginStatus = expectOutput("login:", buffer);
      if (loginStatus != RTELNET_SUCCESS) return loginStatus;
      unsigned int loginResponse = _tcp.Send(_username + "\n");
      if (loginResponse != RTELNET_SUCCESS) return loginResponse;

      // Enter password
      buffer.clear();
      unsigned int passwordStatus = expectOutput("Password:", buffer);
      if (passwordStatus != RTELNET_SUCCESS) return passwordStatus;
      unsigned int passwordResponse = _tcp.Send(_password + "\n");
      if (passwordResponse != RTELNET_SUCCESS) return passwordResponse;

      // Search for "Login incorrect"
      buffer.clear();
      unsigned int bufferPeek = _tcp.Read(buffer);
      if (bufferPeek != RTELNET_SUCCESS) { return bufferPeek; }

      std::string cleanedBuffer(reinterpret_cast<const char*>(buffer.data()), buffer.size());

      cleanedBuffer.erase(std::remove(cleanedBuffer.begin(), cleanedBuffer.end(), '\r'), cleanedBuffer.end());
      cleanedBuffer.erase(std::remove(cleanedBuffer.begin(), cleanedBuffer.end(), '\n'), cleanedBuffer.end());

      if (cleanedBuffer.find("Login incorrect") != std::string::npos) {
        return RTELNET_ERROR_FAILED_LOGIN;
      }

      _logged_in = true;

      _logger.log(RTELNET_LOG_LOGIN, "Logged in successfully.", 2, LV(_username), LV(_password));

      return RTELNET_SUCCESS;
    }

    friend class tcp;
    friend class Logger;
  };

}
#endif // RTELNET_H
