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

#include <iostream>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <ostream>
#include <string_view>
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
#include <atomic>
#include <mutex>

#define LV(x) #x, x
#define PUSH_ERROR(code) pushError(code, __LINE__, __func__)

// DEFUALTS
inline constexpr int RTELNET_SUCCESS             = 0;
inline constexpr int RTELNET_PORT                = 23;
inline constexpr int RTELNET_BUFFER_SIZE         = 1024;
inline constexpr int RTELNET_IP_VERSION          = 4;
inline constexpr int RTELNET_IDLE_TIMEOUT        = 1000;
inline constexpr int RTELNET_TOTAL_TIMEOUT       = 10000;
inline constexpr int RTELNET_DEBUG               = 0;
inline constexpr int RTELNET_LOGIN_TIMEOUT       = 3000; // ms
inline constexpr int RTELNET_NEGOTIATION_TIMEOUT = 3; // s

// Log titles
inline constexpr std::string_view RTELNET_LOG_TCP_SET_ADDR = "TCP => SETTING SOCKET ADDRESS";
inline constexpr std::string_view RTELNET_LOG_TCP_CONNECT = "TCP => CONNECTING TO SOCKET";
inline constexpr std::string_view RTELNET_LOG_TCP_CLOSE = "TCP => CLOSING SOCKET";
inline constexpr std::string_view RTELNET_LOG_TCP_SEND_BIN = "TCP => SEND BIN";
inline constexpr std::string_view RTELNET_LOG_TCP_SEND = "TCP => SEND";
inline constexpr std::string_view RTELNET_LOG_TCP_READ = "TCP => READ";
inline constexpr std::string_view RTELNET_LOG_CONNECT = "CONNECT";
inline constexpr std::string_view RTELNET_LOG_EXECUTE = "EXECITE";
inline constexpr std::string_view RTELNET_LOG_NEGOTIATE = "NEGOTIATE";
inline constexpr std::string_view RTELNET_LOG_LOGIN = "LOGIN";
inline constexpr std::string_view RTELNET_LOG_IAC_READER = "IAC READER";
inline constexpr std::string_view RTELNET_LOG_PTELNET = "TELNET";

namespace rtnt {

  enum Errors {
    // 0 | 200 > 210 : Relic telnet
    CANT_FIND_EXPECTED     = 201,
  
    // From 1 to 199 - errno errors
  
    // 210 > : TCP connection errors.
    ADDRESS_NOT_VALID      = 210,
    CANNOT_ALLOCATE_FD      = 211,
    CONNECTION_CLOSED_R    = 212,
    NOT_CONNECTED          = 213,
    FAILED_SEND            = 214,
    PARTIAL_SEND           = 215,
  
    // 300 > : Telnet logic errors.
    NOT_A_NEGOTIATION      = 300,
    NOT_NEGOTIATED         = 301,
    USERNAME_NOT_SET       = 302,
    PASSWORD_NOT_SET       = 303,
    NOT_LOGGED             = 304,
    FAILED_LOGIN           = 305,
    IAC_READER_FAILED_NEGO = 306,
    SHARED_BUFFER_EMPTY    = 307,
    NEGOTIATION_TIMEOUT    = 308
  };

  enum TelnetCommands : unsigned char {
    IAC  = 255, // Interpret As Command
    DO   = 253, // Please use this option
    DONT = 254, // Please don’t use this option
    WILL = 251, // I will use this option
    WONT = 252, // I won’t use this option
    SB   = 250, // Begin subnegotiation
    SE   = 240  // End subnegotiation
  };

  enum TelnetOptions : unsigned char {
    BINARY              = 0,   // Binary transmission (8-bit clean communication)
    ECHO                = 1,   // Echo input back to sender (commonly server-side)
    SGA                 = 3,   // Suppress Go Ahead (stream mode instead of line mode)
    STATUS              = 5,   // Query or send current option status
    TIMING_MARK         = 6,   // Timing mark for synchronization
    TERMINAL_TYPE       = 24,  // Exchange terminal type (e.g., "ANSI", "VT100")
    NAWS                = 31,  // Negotiate About Window Size (send terminal size)
    LINEMODE            = 34,  // Line-by-line input mode negotiation
    NEW_ENVIRON         = 39,  // Send environment variables (supersedes option 36)
    X_DISPLAY_LOCATION  = 35,  // Send X11 DISPLAY value (e.g., ":0")
    LOGOUT              = 18,  // Server requests client logout
    ENVIRONMENT_OPTION  = 36,  // Deprecated method to send environment variables
    AUTHENTICATION      = 37,  // Authenticate user via a scheme (e.g., Kerberos)
    ENCRYPTION          = 38,  // Encrypt the Telnet stream
    RCP                 = 2,   // Remote Controlled Port (obsolete)
    NAMS                = 4,   // Negotiate Approximate Message Size (rare)
    RCTE                = 7,   // Remote Controlled Transmission and Echo
    NAOL                = 8,   // Output line width setting
    NAOP                = 9,   // Output page size
    NAOCRD              = 10,  // Carriage return disposition
    NAOHTS              = 11,  // Horizontal tab stops
    NAOHTD              = 12,  // Horizontal tab disposition
    NAOFFD              = 13,  // Formfeed disposition
    NAOVTS              = 14,  // Vertical tab stops
    NAOVTD              = 15,  // Vertical tab disposition
    NAOLFD              = 16,  // Linefeed disposition
    EXTEND_ASCII        = 17,  // Extended ASCII character set support
    BM                  = 19,  // Byte macro (macros for command sequences)
    DET                 = 20,  // Data Entry Terminal mode
    SUPDUP              = 21,  // MIT SUPDUP protocol support
    SUPDUP_OUTPUT       = 22,  // SUPDUP output extension
    SEND_LOCATION       = 23,  // Send geographic location
    END_OF_RECORD       = 25,  // Logical end-of-record marker
    TACACS_UID          = 26,  // User identification via TACACS
    OUTPUT_MARKING      = 27,  // Marks screen output boundaries
    TTYLOC              = 28,  // Send terminal location (TTYLOC)
    REMOTE_FLOW_CONTROL = 29,  // Enable/disable flow control remotely
    XAUTH               = 41,  // X Window System authentication
    CHARSET             = 42,  // Negotiate character set
    RSP                 = 43,  // Remote serial port control
    COM_PORT_CONTROL    = 44,  // Advanced serial port control
    SUPPRESS_LOCAL_ECHO = 45,  // Don't locally echo what we typeRTELNET_LOG_PTELNET
    MCCP1               = 85,  // MUD Client Compression Protocol v1
    MCCP2               = 86,  // MUD Client Compression Protocol v2
    GMCP                = 201, // Generic Mud Communication Protocol
    PRAGMA_LOGON        = 138, // Used in Microsoft Telnet (may be in private range)
    SSPI_LOGON          = 139, // SSPI-based login (Microsoft)
    PRAGMA_HEARTBEAT    = 140, // Keep-alive negotiation
    TOGGLE_FLOW_CONTROL = 33,  // Obsolete; similar to REMOTE_FLOW_CONTROL
    X3_PAD              = 30,  // Transmit X.3 PAD parameters
    MSDP                = 69,  // Mud Server Data Protocol (used in MUDs)
    MSSP                = 70,  // Mud Server Status Protocol
    ZMP                 = 93,  // Zenith Mud Protocol
    MUX                 = 95,  // Legacy multi-session support
    TERMINAL_SPEED      = 32   // Set terminal baud rate
  };

  inline std::string_view readError(int rtntErrno) {
    if (rtntErrno < 200) return strerror(errno);
    switch (rtntErrno) {
      case RTELNET_SUCCESS: return                  "No error.";
      case Errors::ADDRESS_NOT_VALID: return        "address is not valid.";
      case Errors::CANNOT_ALLOCATE_FD: return        "cannot allocate a file descriptor.";
      case Errors::CONNECTION_CLOSED_R: return      "Connection closed by remote.";
      case Errors::NOT_CONNECTED: return            "connection failed, tcp session was not established.";
      case Errors::FAILED_SEND: return              "could not send message. (No errno just 0 bytes sent)";
      case Errors::PARTIAL_SEND: return             "message was sent partially.";

      // Telnet logic errors
      case Errors::NOT_A_NEGOTIATION: return        "a negotiation was called, yet server did not negotiate.";
      case Errors::NOT_NEGOTIATED: return           "negotiation is required, please negotiate first.";
      case Errors::NOT_LOGGED: return               "login is required, please login and try again.";
      case Errors::FAILED_LOGIN: return             "login failed, username or password are wrong.";

      // rtelnet specific
      case Errors::CANT_FIND_EXPECTED: return       "cannot find expected substring in buffer.";
      case Errors::USERNAME_NOT_SET: return         "username was not set in object.";
      case Errors::PASSWORD_NOT_SET: return         "password was not set in object.";
      case Errors::IAC_READER_FAILED_NEGO: return   "IAC reader failed while re negotiating.";
      case Errors::SHARED_BUFFER_EMPTY: return      "Read failed, the shared buffer is empty.";
      case Errors::NEGOTIATION_TIMEOUT: return      "Timeout while waiting for negotiation.";

      default: return                               "Unknown error.";
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
      _stopBackground = true;

      if (_background.joinable()) {
        _background.join();
      }

      _tcp.Close();
    }

    class Logger {
    public:
      Logger(session* owner) : _owner(owner) {}

      template <typename T, typename... Rest>
      void log(const std::string_view& title, const std::string& message, int level,
              const std::string& name1, const T& val1, const Rest&... rest) {
        if (_owner->_debug < level) return;

        std::cerr << "\033[1;97m[\033[0m\033[1;96m"
                  << title
                  << "\033[0m\033[1;97m]:\033[0m "
                  << message
                  << " (";
        
        printNamedArgs(true, name1, val1, rest...);

        std::cerr << ")" << std::endl;
      }

      inline void log(const std::string_view& title, const std::string& message, int level) {
        if (_owner->_debug < level) return;

        std::cerr << "\033[1;97m[\033[0m\033[1;96m"
                  << title
                  << "\033[0m\033[1;97m]:\033[0m "
                  << message
                  << std::endl;
      }

      inline void printTelnet(const std::vector<unsigned char>& buffer, int who) {
        if (buffer.size() != 3 || buffer[0] != IAC || _owner->_debug < 3) {
          return;
        }
      
        auto cmdName = [](unsigned char code) -> const char* {
          switch (static_cast<TelnetOptions>(code)) {
            case TelnetCommands::DO: return "DO";
            case TelnetCommands::DONT: return "DONT";
            case TelnetCommands::WILL: return "WILL";
            case TelnetCommands::WONT: return "WONT";
            case TelnetCommands::SB: return "SB";
            case TelnetCommands::SE: return "SE";
            default: return "UNKNOWN_CMD";
          }
        };
      
        auto optName = [](unsigned char code) -> const char* {
          switch (static_cast<TelnetOptions>(code)) {
            case TelnetOptions::BINARY: return "BINARY";
            case TelnetOptions::ECHO: return "ECHO";
            case TelnetOptions::RCP: return "RCP";
            case TelnetOptions::SGA: return "SUPPRESS_GO_AHEAD";
            case TelnetOptions::NAMS: return "NAMS";
            case TelnetOptions::STATUS: return "STATUS";
            case TelnetOptions::TIMING_MARK: return "TIMING_MARK";
            case TelnetOptions::RCTE: return "RCTE";
            case TelnetOptions::NAOL: return "NAOL";
            case TelnetOptions::NAOP: return "NAOP";
            case TelnetOptions::NAOCRD: return "NAOCRD";
            case TelnetOptions::NAOHTS: return "NAOHTS";
            case TelnetOptions::NAOHTD: return "NAOHTD";
            case TelnetOptions::NAOFFD: return "NAOFFD";
            case TelnetOptions::NAOVTS: return "NAOVTS";
            case TelnetOptions::NAOVTD: return "NAOVTD";
            case TelnetOptions::NAOLFD: return "NAOLFD";
            case TelnetOptions::EXTEND_ASCII: return "EXTEND_ASCII";
            case TelnetOptions::LOGOUT: return "LOGOUT";
            case TelnetOptions::BM: return "BYTE_MACRO";
            case TelnetOptions::DET: return "DET";
            case TelnetOptions::SUPDUP: return "SUPDUP";
            case TelnetOptions::SUPDUP_OUTPUT: return "SUPDUP_OUTPUT";
            case TelnetOptions::SEND_LOCATION: return "SEND_LOCATION";
            case TelnetOptions::TERMINAL_TYPE: return "TERMINAL_TYPE";
            case TelnetOptions::END_OF_RECORD: return "END_OF_RECORD";
            case TelnetOptions::TACACS_UID: return "TACACS_UID";
            case TelnetOptions::OUTPUT_MARKING: return "OUTPUT_MARKING";
            case TelnetOptions::TTYLOC: return "TTYLOC";
            case TelnetOptions::REMOTE_FLOW_CONTROL: return "REMOTE_FLOW_CONTROL";
            case TelnetOptions::X_DISPLAY_LOCATION: return "X_DISPLAY_LOCATION";
            case TelnetOptions::ENVIRONMENT_OPTION: return "ENVIRONMENT_OPTION";
            case TelnetOptions::AUTHENTICATION: return "AUTHENTICATION";
            case TelnetOptions::ENCRYPTION: return "ENCRYPTION";
            case TelnetOptions::NEW_ENVIRON: return "NEW_ENVIRON";
            case TelnetOptions::NAWS: return "NAWS";
            case TelnetOptions::LINEMODE: return "LINEMODE";
            case TelnetOptions::XAUTH: return "XAUTH";
            case TelnetOptions::CHARSET: return "CHARSET";
            case TelnetOptions::RSP: return "RSP";
            case TelnetOptions::COM_PORT_CONTROL: return "COM_PORT_CONTROL";
            case TelnetOptions::SUPPRESS_LOCAL_ECHO: return "SUPPRESS_LOCAL_ECHO";
            case TelnetOptions::MCCP1: return "MCCP1";
            case TelnetOptions::MCCP2: return "MCCP2";
            case TelnetOptions::GMCP: return "GMCP";
            case TelnetOptions::PRAGMA_LOGON: return "PRAGMA_LOGON";
            case TelnetOptions::SSPI_LOGON: return "SSPI_LOGON";
            case TelnetOptions::PRAGMA_HEARTBEAT: return "PRAGMA_HEARTBEAT";
            case TelnetOptions::TOGGLE_FLOW_CONTROL: return "TOGGLE_FLOW_CONTROL";
            case TelnetOptions::X3_PAD: return "X3_PAD";
            case TelnetOptions::MSDP: return "MSDP";
            case TelnetOptions::MSSP: return "MSSP";
            case TelnetOptions::ZMP: return "ZMP";
            case TelnetOptions::MUX: return "MUX";
            case TelnetOptions::TERMINAL_SPEED: return "TERMINAL_SPEED";
            default: return "UNKNOWN_OPT";
          }
        };

        std::string results = 
          std::string((who == 0) ? "CLIENT => " : "SERVER <= ") +
          std::string(cmdName(static_cast<unsigned char>(buffer[1])))
          + " " +
          optName(static_cast<unsigned char>(buffer[2])) +
          " (IAC " + std::to_string(buffer[1]) + " " + std::to_string(buffer[2]) + ")";
        

        log(RTELNET_LOG_PTELNET, results, 3);
      }

    private:
      session* _owner;

      template <typename T>
      void printNamedArg(const std::string& name, const T& value, bool isFirst) {
        if (!isFirst) std::cerr << ", ";
        std::cerr << name << ": " << value;
      }

      template <typename T>
      void printNamedArg(const std::string& name, const std::vector<T>& value, bool isFirst) {
        if (!isFirst) std::cerr << ", ";
        std::cerr << name << ": <";
        for (size_t i = 0; i < value.size(); ++i) {
          std::cerr << value[i];
          if (i + 1 < value.size()) std::cerr << ", ";
        }
        std::cerr << ">";
      }

      inline void printNamedArg(const std::string& name, const std::vector<unsigned char>& value, bool isFirst) {
        if (!isFirst) std::cerr << ", ";
        std::cerr << name << ": <";
        for (size_t i = 0; i < value.size(); ++i) {
          std::cerr << static_cast<int>(value[i]);
          if (i + 1 < value.size()) std::cerr << ", ";
        }
        std::cerr << ">";
      }

      inline void printNamedArg(const std::string& name, const std::string& value, bool isFirst) {
        if (!isFirst) std::cerr << ", ";
        std::cerr << name << ": \"";
        for (char c : value) {
          switch (c) {
            case '\\': std::cerr << "\\\\"; break;
            case '\n': std::cerr << "\\n"; break;
            case '\t': std::cerr << "\\t"; break;
            case '\r': std::cerr << "\\r"; break;
            default: std::cerr << c; break;
          }
        }
        std::cerr << "\"";
      }

      template <typename K, typename V>
      void printNamedArg(const std::string& name, const std::map<K, V>& value, bool isFirst) {
        if (!isFirst) std::cerr << ", ";
        std::cerr << name << ": <";
        size_t count = 0;
        for (const auto& [k, v] : value) {
          std::cerr << k << ": " << v;
          if (++count < value.size()) std::cerr << ", ";
        }
        std::cerr << ">";
      }

      inline void printNamedArgs(bool) {}

      template <typename T, typename... Rest>
      void printNamedArgs(bool isFirst, const std::string& name, const T& value, const Rest&... rest) {
        printNamedArg(name, value, isFirst);
        printNamedArgs(false, rest...);
      }
    };

    inline void throwErrorStack() {
        std::cerr << "[Error Stack Trace]" << std::endl;

        for (size_t i = 0; i < errorStack.size(); ++i) {
            const auto& err = errorStack[i];

            std::cerr << (i + 1 < errorStack.size() ? " ├─ " : " └─ ")
                      << "#" << i
                      << " [Code " << err.code << "] "
                      << err.function << ":" << err.line
                      << " → " << readError(err.code)
                      << std::endl;
        }
    }

    class tcp {
    public:
      tcp(session* owner) : _owner(owner) {}

      inline unsigned int setSocketAddr(sockaddr_in& server_address) const {
        server_address.sin_family = (_owner->_ipv == 4) ? AF_INET : AF_INET6;
        server_address.sin_port = htons(_owner->_port);

        if (inet_pton(AF_INET, _owner->_address, &server_address.sin_addr) <= 0) {
          return _owner->PUSH_ERROR(Errors::ADDRESS_NOT_VALID);
        }
        
        _owner->_logger.log(RTELNET_LOG_TCP_SET_ADDR, "Successfully set socket address.", 4, LV(_owner->_address), LV(_owner->_port));

        return RTELNET_SUCCESS;
      }

      inline unsigned int Connect(sockaddr_in& address) {
        int sockfd = socket((_owner->_ipv == 4) ? AF_INET : AF_INET6, SOCK_STREAM, 0);
        if (sockfd < 0) return _owner->PUSH_ERROR(Errors::CANNOT_ALLOCATE_FD);

        errno = 0;
        if (connect(sockfd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) { return _owner->PUSH_ERROR(errno); }

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
        if (!_owner->_connected) return _owner->PUSH_ERROR(Errors::NOT_CONNECTED);

        errno = 0;
        ssize_t bytesSent = send(_owner->_fd, message.data(), message.size(), sendFlag);

        if (bytesSent == 0) return _owner->PUSH_ERROR(Errors::FAILED_SEND);
        if (static_cast<size_t>(bytesSent) != message.size()) return _owner->PUSH_ERROR(Errors::PARTIAL_SEND);
        if (bytesSent < 0) return _owner->PUSH_ERROR(errno);

        _owner->_logger.log(RTELNET_LOG_TCP_SEND_BIN, "Successfully sent message.", 4, LV(message), LV(sendFlag));

        return RTELNET_SUCCESS;
      }

      inline unsigned int Send(const std::string& message, int sendFlag = 0) const {
        if (!_owner->_connected) return _owner->PUSH_ERROR(Errors::NOT_CONNECTED);

        std::vector<unsigned char> buffer;
        buffer.reserve(message.size());

        // Escape 255/0xFF unless full duplex binary communication.
        for (unsigned char c : message) {
          buffer.push_back(c);
          if (!(_owner->_binarySendEnabled && _owner->_binaryReceiveEnabled) && c == 255) buffer.push_back(255);
        }

        errno = 0;
        ssize_t bytesSent = send(_owner->_fd, buffer.data(), buffer.size(), sendFlag);

        if (bytesSent == 0) return _owner->PUSH_ERROR(Errors::FAILED_SEND);
        if (bytesSent < 0) return _owner->PUSH_ERROR(errno);
        if (static_cast<size_t>(bytesSent) != buffer.size()) return _owner->PUSH_ERROR(Errors::PARTIAL_SEND);

        _owner->_logger.log(RTELNET_LOG_TCP_SEND, "Successfully sent message.", 4, LV(message), LV(sendFlag));

        return RTELNET_SUCCESS;
      }

      inline unsigned int Read(std::vector<unsigned char>& buffer, int readSize = RTELNET_BUFFER_SIZE, int recvFlag = 0) const {
        if (!_owner->_connected) return _owner->PUSH_ERROR(Errors::NOT_CONNECTED);

        buffer.resize(readSize);

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(_owner->_fd, &readfds);

        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ready = select(_owner->_fd + 1, &readfds, nullptr, nullptr, &timeout);
        if (ready < 0) return _owner->PUSH_ERROR(errno);
        if (ready == 0) {
          buffer.clear();
          return RTELNET_SUCCESS;
        }

        errno = 0;
        ssize_t bytesRead = recv(_owner->_fd, reinterpret_cast<char*>(buffer.data()), readSize, recvFlag);

        if (bytesRead < 0) return _owner->PUSH_ERROR(errno);
        if (bytesRead == 0) return _owner->PUSH_ERROR(Errors::CONNECTION_CLOSED_R);

        buffer.resize(bytesRead);
 
        return RTELNET_SUCCESS;
      }

    private:
      session* _owner;
    };

    // Read-only accessors
    inline bool isConnected() const { return _connected; }
    inline bool isNegotiated() const { return _negotiated; }
    inline bool isLoggedIn() const { return _logged_in; }
    inline int getBackgroundError() const { return _backgroundError; }
    inline bool isBackgroundError() const { return _stopBackground; }

    tcp _tcp;
    Logger _logger;

    inline unsigned int Read(std::vector<unsigned char>& buffer, size_t n = RTELNET_BUFFER_SIZE, unsigned int flag = 0, unsigned int timeoutMs = 1000) {
      auto start = std::chrono::steady_clock::now();

      while (true) {
        {
          std::lock_guard<std::mutex> lock(_bufferMutex);

          size_t toRead = std::min(n, _sharedBuffer.size());

          if (toRead > 0) {
            buffer.insert(buffer.end(), _sharedBuffer.begin(), _sharedBuffer.begin() + toRead);
            if (flag != MSG_PEEK) {
                _sharedBuffer.erase(_sharedBuffer.begin(), _sharedBuffer.begin() + toRead);
            }
            return RTELNET_SUCCESS;
          }
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > timeoutMs) {
          buffer.clear();
          return RTELNET_SUCCESS;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // avoid CPU spinning
      }
    }

    inline unsigned int Connect() {

      _logger.log(RTELNET_LOG_CONNECT, "Trying to connnected to telnet server.", 2, LV(_address), LV(_port));

      // Get address
      sockaddr_in address;
      unsigned int addressResult = _tcp.setSocketAddr(address);
      if (addressResult != 0 ) return PUSH_ERROR(addressResult);

      int fd = _tcp.Connect(address);
      if (fd < 0) return PUSH_ERROR(fd);
      _fd = fd;
      
      _background = std::thread([this]() {
        while (!_stopBackground) {
          int readFlag = 0;
          if (!_negotiated) readFlag = MSG_PEEK;

          std::vector<unsigned char> buffer;
          unsigned int status = _tcp.Read(buffer, RTELNET_BUFFER_SIZE, readFlag);

          if (status != RTELNET_SUCCESS) {
            _stopBackground = true; _backgroundError = status; break;
          }

          if (buffer[0] == TelnetCommands::IAC) {
            int negotiateStatus = Negotiate();
            if (negotiateStatus != RTELNET_SUCCESS) {
              _stopBackground = true;
              _backgroundError = negotiateStatus;
              break;
            }
            continue;
          } else {
            std::lock_guard<std::mutex> lock(_bufferMutex);
            _sharedBuffer.insert(_sharedBuffer.end(), buffer.begin(), buffer.end());
          }

          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
      });


      auto start = std::chrono::steady_clock::now();
      while (!_negotiated) {
        if ((std::chrono::steady_clock::now() - start) > std::chrono::seconds(RTELNET_NEGOTIATION_TIMEOUT)) {
          return PUSH_ERROR(Errors::NEGOTIATION_TIMEOUT);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }

      int loginStatus = Login();
      if (loginStatus != RTELNET_SUCCESS) return PUSH_ERROR(loginStatus);

      _logger.log(RTELNET_LOG_CONNECT, "Connected to telnet server.", 2, _address, _port);
 
      return RTELNET_SUCCESS;
    }

    unsigned int Execute(const std::string& command, std::string& buffer) {
      if (!_connected) return PUSH_ERROR(Errors::NOT_CONNECTED);
      if (!_negotiated) return PUSH_ERROR(Errors::NOT_NEGOTIATED);
      if (!_logged_in) return PUSH_ERROR(Errors::NOT_LOGGED);

      _logger.log(RTELNET_LOG_EXECUTE, "Trying to execute a command.", 2, LV(command));

      unsigned int sendStatus = _tcp.Send(command + "\n");
      if (sendStatus != RTELNET_SUCCESS) return PUSH_ERROR(sendStatus);

      std::vector<unsigned char> output;
      buffer.clear();

      auto startTime = std::chrono::steady_clock::now();
      auto lastRead = startTime;

      while (true) {
        output.clear();

        unsigned int readStatus = Read(output);
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
      if (!_connected) return PUSH_ERROR(Errors::NOT_CONNECTED);
      if (!_negotiated) return PUSH_ERROR(Errors::NOT_NEGOTIATED);
      if (!_logged_in) return PUSH_ERROR(Errors::NOT_LOGGED);
      
      std::string buffer;
      unsigned int execStatus = Execute("\n", buffer);
      if (execStatus != RTELNET_SUCCESS) return PUSH_ERROR(execStatus);

      return RTELNET_SUCCESS;
    }

  private:
    bool _connected = false;
    bool _negotiated = false;
    bool _logged_in = false;
    int _fd;

    /*        ---           IAC Listener         ---         */
    std::thread _background;
    std::atomic<bool> _stopBackground{false};
    std::mutex _bufferMutex;
    std::vector<unsigned char> _sharedBuffer;
    unsigned int _backgroundError;

    /*        ---           IAC Listener         ---         */

    /*        ---         Telnet commands        ---         */
    bool _binarySendEnabled = false;
    bool _binaryReceiveEnabled = false; 
    /*        ---         Telnet commands        ---         */

    struct errorEntry {
      unsigned int code;
      unsigned int line;
      std::string function;
    };

    std::vector<errorEntry> errorStack;

    unsigned int pushError(unsigned int code, unsigned int line, const std::string& function) {
      errorEntry ee;
      ee.code = code;
      ee.line = line;
      ee.function = function;

      errorStack.push_back(ee); 

      return code;
    }

    unsigned int Negotiate() {
      if (!_connected) return PUSH_ERROR(Errors::NOT_CONNECTED);

      _logger.log(RTELNET_LOG_NEGOTIATE, "Trying to negotiate.", 2);
    
      std::vector<unsigned char> buffer;

      while (true) {
        // Peek in the buffer
        buffer.clear();
        unsigned int bufferPeek = _tcp.Read(buffer, 3, MSG_PEEK);
        if (bufferPeek != RTELNET_SUCCESS) return PUSH_ERROR(bufferPeek);

        _logger.printTelnet(buffer, 1);

        ssize_t n = static_cast<ssize_t>(buffer.size());
        if (n < 3) {
          _negotiated = false;
          return PUSH_ERROR(Errors::SHARED_BUFFER_EMPTY);
        }

        // If not a negotiation packet, exit
        if (buffer[0] != TelnetCommands::IAC) {
          if (!_negotiated) {
            return PUSH_ERROR(Errors::NOT_A_NEGOTIATION);
          } else {
            break;
          }
        }

        // Read the full 3-byte sequence
        buffer.clear();
        unsigned int bufferResult = _tcp.Read(buffer, 3);
        if (bufferPeek != RTELNET_SUCCESS) return PUSH_ERROR(bufferResult);

        unsigned char command = buffer[1];
        unsigned char option  = buffer[2];
        std::vector<unsigned char> response = {
          static_cast<unsigned char>(TelnetCommands::IAC),
          static_cast<unsigned char>(0),
          static_cast<unsigned char>(option)
        };

        switch (command) {
          case TelnetCommands::DO:
            switch (option) {
              case TelnetOptions::BINARY:               response[1] = TelnetCommands::WILL; _binarySendEnabled = true; break;
              case TelnetOptions::ECHO:                 response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::SGA:                  response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::STATUS:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::TIMING_MARK:          response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::TERMINAL_TYPE:        response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAWS:                 response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::LINEMODE:             response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NEW_ENVIRON:          response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::X_DISPLAY_LOCATION:   response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::LOGOUT:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::ENVIRONMENT_OPTION:   response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::AUTHENTICATION:       response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::ENCRYPTION:           response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::RCP:                  response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAMS:                 response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::RCTE:                 response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAOL:                 response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAOP:                 response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAOCRD:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAOHTS:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAOHTD:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAOFFD:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAOVTS:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAOVTD:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::NAOLFD:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::EXTEND_ASCII:         response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::BM:                   response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::DET:                  response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::SUPDUP:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::SUPDUP_OUTPUT:        response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::SEND_LOCATION:        response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::END_OF_RECORD:        response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::TACACS_UID:           response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::OUTPUT_MARKING:       response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::TTYLOC:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::REMOTE_FLOW_CONTROL:  response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::TOGGLE_FLOW_CONTROL:  response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::X3_PAD:               response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::MSDP:                 response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::MSSP:                 response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::ZMP:                  response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::MUX:                  response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::MCCP1:                response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::MCCP2:                response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::GMCP:                 response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::PRAGMA_LOGON:         response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::SSPI_LOGON:           response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::PRAGMA_HEARTBEAT:     response[1] = TelnetCommands::WONT; break;
              case TelnetOptions::TERMINAL_SPEED:       response[1] = TelnetCommands::WONT; break;
              default:                                  response[1] = TelnetCommands::WONT; break;
            }
            break;

          case TelnetCommands::WILL:
            switch (option) {
              case TelnetOptions::BINARY:               response[1] = TelnetCommands::DO; _binaryReceiveEnabled = true; break;
              case TelnetOptions::ECHO:                 response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::SGA:                  response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::STATUS:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::TIMING_MARK:          response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::TERMINAL_TYPE:        response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAWS:                 response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::LINEMODE:             response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NEW_ENVIRON:          response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::X_DISPLAY_LOCATION:   response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::LOGOUT:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::ENVIRONMENT_OPTION:   response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::AUTHENTICATION:       response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::ENCRYPTION:           response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::RCP:                  response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAMS:                 response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::RCTE:                 response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAOL:                 response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAOP:                 response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAOCRD:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAOHTS:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAOHTD:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAOFFD:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAOVTS:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAOVTD:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::NAOLFD:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::EXTEND_ASCII:         response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::BM:                   response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::DET:                  response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::SUPDUP:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::SUPDUP_OUTPUT:        response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::SEND_LOCATION:        response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::END_OF_RECORD:        response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::TACACS_UID:           response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::OUTPUT_MARKING:       response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::TTYLOC:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::REMOTE_FLOW_CONTROL:  response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::TOGGLE_FLOW_CONTROL:  response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::X3_PAD:               response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::MSDP:                 response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::MSSP:                 response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::ZMP:                  response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::MUX:                  response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::MCCP1:                response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::MCCP2:                response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::GMCP:                 response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::PRAGMA_LOGON:         response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::SSPI_LOGON:           response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::PRAGMA_HEARTBEAT:     response[1] = TelnetCommands::DONT; break;
              case TelnetOptions::TERMINAL_SPEED:       response[1] = TelnetCommands::DONT; break;
              default:                                  response[1] = TelnetCommands::DONT; break;
            }
            break;

          case TelnetCommands::WONT:
          case TelnetCommands::DONT:
            // Add supprt for these later.
            break;
        }

        
        _tcp.SendBin(response);
        _negotiated = true;
        _logger.printTelnet(response, 0);
      }

      _logger.log(RTELNET_LOG_NEGOTIATE, "Finished negotiation sequence.", 2);
      return RTELNET_SUCCESS;
    }

    unsigned int expectOutput(const std::string& expect, std::vector<unsigned char>& buffer) {
        if (!_connected) return PUSH_ERROR(Errors::NOT_CONNECTED);
        if (!_negotiated) return PUSH_ERROR(Errors::NOT_NEGOTIATED);

        for (int i = 0; i < 300; ++i) {
          unsigned int readStatus = Read(buffer);
          if (readStatus != RTELNET_SUCCESS) return PUSH_ERROR(readStatus);

          _logger.log("EXPECT", "Expecting.", 2, LV(expect) , LV(buffer));

          std::string cleanedBuffer(reinterpret_cast<const char*>(buffer.data()), buffer.size());

          cleanedBuffer.erase(std::remove(cleanedBuffer.begin(), cleanedBuffer.end(), '\r'), cleanedBuffer.end());
          cleanedBuffer.erase(std::remove(cleanedBuffer.begin(), cleanedBuffer.end(), '\n'), cleanedBuffer.end());

          if (cleanedBuffer.find(expect) != std::string::npos) {
              return RTELNET_SUCCESS;
          }

          std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        return PUSH_ERROR(Errors::CANT_FIND_EXPECTED);
    }

    inline unsigned int Login() {
      if (!_connected) return PUSH_ERROR(Errors::NOT_CONNECTED);
      if (!_negotiated) return PUSH_ERROR(Errors::NOT_NEGOTIATED);
      if (_username.empty()) return PUSH_ERROR(Errors::USERNAME_NOT_SET);
      if (_password.empty()) return PUSH_ERROR(Errors::PASSWORD_NOT_SET);

      _logger.log(RTELNET_LOG_LOGIN, "Trying to login.", 2, LV(_username), LV(_password));

      std::vector<unsigned char> buffer;

      // Enter login
      buffer.clear();
      unsigned int loginStatus = expectOutput("login:", buffer);
      if (loginStatus != RTELNET_SUCCESS) return PUSH_ERROR(loginStatus);
      unsigned int loginResponse = _tcp.Send(_username + "\n");
      if (loginResponse != RTELNET_SUCCESS) return PUSH_ERROR(loginResponse);

      // Enter password
      buffer.clear();
      unsigned int passwordStatus = expectOutput("Password:", buffer);
      if (passwordStatus != RTELNET_SUCCESS) return PUSH_ERROR(passwordStatus);
      unsigned int passwordResponse = _tcp.Send(_password + "\n");
      if (passwordResponse != RTELNET_SUCCESS) return PUSH_ERROR(passwordResponse);

      // Search for "Login incorrect"
      std::string accumulated;
      buffer.clear();
      auto start = std::chrono::steady_clock::now();

      _logger.log(RTELNET_LOG_LOGIN, "Searching for Login incorrect.", 2);

      while (true) {

          unsigned int readStatus = Read(buffer, RTELNET_BUFFER_SIZE, MSG_PEEK);
          if (readStatus != RTELNET_SUCCESS) return PUSH_ERROR(readStatus);

          if (!buffer.empty()) {
              std::string temp(reinterpret_cast<const char*>(buffer.data()), buffer.size());
              temp.erase(std::remove(temp.begin(), temp.end(), '\r'), temp.end());
              temp.erase(std::remove(temp.begin(), temp.end(), '\n'), temp.end());

              accumulated += temp;
          }

          _logger.log(RTELNET_LOG_LOGIN, "Still searching.", 2, LV(accumulated));


          if (accumulated.find("Login incorrect") != std::string::npos) {
              return PUSH_ERROR(Errors::FAILED_LOGIN);
          }

          // possible: detect prompt here, e.g., "$ " or "> "
          if (accumulated.find("$") != std::string::npos || accumulated.find(">") != std::string::npos || accumulated.find("#") != std::string::npos) {
              break;
          }

          auto now = std::chrono::steady_clock::now();
          if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > RTELNET_LOGIN_TIMEOUT) {
              break;
          }

          std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
