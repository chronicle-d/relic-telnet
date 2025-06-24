#include <iostream>
#include <vector>

inline constexpr unsigned char IAC                    = 255; // Interpret As Command
inline constexpr unsigned char DO                     =	253; //	Please use this option
inline constexpr unsigned char DONT                   =	254; //	Please don’t use this option
inline constexpr unsigned char WILL                   = 251; //	I will use this option
inline constexpr unsigned char WONT                   = 252; // I won’t use this option
inline constexpr unsigned char SB                     = 250; //	Begin subnegotiation
inline constexpr unsigned char SE                     =	240; //	End subnegotiation
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

inline void printTelnet(const std::vector<unsigned char>& buffer, int who) {
  if (buffer.size() != 3 || buffer[0] != IAC) {
    return;
  }

  auto cmdName = [](unsigned char code) -> const char* {
    switch (code) {
      case DO: return "DO";
      case DONT: return "DONT";
      case WILL: return "WILL";
      case WONT: return "WONT";
      case SB: return "SB";
      case SE: return "SE";
      default: return "UNKNOWN_CMD";
    }
  };

  auto optName = [](unsigned char code) -> const char* {
    switch (code) {
      case BINARY: return "BINARY";
      case ECHO: return "ECHO";
      case RCP: return "RCP";
      case SGA: return "SUPPRESS_GO_AHEAD";
      case NAMS: return "NAMS";
      case STATUS: return "STATUS";
      case TIMING_MARK: return "TIMING_MARK";
      case RCTE: return "RCTE";
      case NAOL: return "NAOL";
      case NAOP: return "NAOP";
      case NAOCRD: return "NAOCRD";
      case NAOHTS: return "NAOHTS";
      case NAOHTD: return "NAOHTD";
      case NAOFFD: return "NAOFFD";
      case NAOVTS: return "NAOVTS";
      case NAOVTD: return "NAOVTD";
      case NAOLFD: return "NAOLFD";
      case EXTEND_ASCII: return "EXTEND_ASCII";
      case LOGOUT: return "LOGOUT";
      case BM: return "BYTE_MACRO";
      case DET: return "DET";
      case SUPDUP: return "SUPDUP";
      case SUPDUP_OUTPUT: return "SUPDUP_OUTPUT";
      case SEND_LOCATION: return "SEND_LOCATION";
      case TERMINAL_TYPE: return "TERMINAL_TYPE";
      case END_OF_RECORD: return "END_OF_RECORD";
      case TACACS_UID: return "TACACS_UID";
      case OUTPUT_MARKING: return "OUTPUT_MARKING";
      case TTYLOC: return "TTYLOC";
      case REMOTE_FLOW_CONTROL: return "REMOTE_FLOW_CONTROL";
      case X3_PAD: return "X3_PAD";
      case TERMINAL_SPEED: return "TERMINAL_SPEED";
      case TOGGLE_FLOW_CONTROL: return "TOGGLE_FLOW_CONTROL";
      case X_DISPLAY_LOCATION: return "X_DISPLAY_LOCATION";
      case ENVIRONMENT_OPTION: return "ENVIRONMENT_OPTION";
      case AUTHENTICATION: return "AUTHENTICATION";
      case ENCRYPTION: return "ENCRYPTION";
      case NEW_ENVIRON: return "NEW_ENVIRON";
      case NAWS: return "NAWS";
      case LINEMODE: return "LINEMODE";
      case XAUTH: return "XAUTH";
      case CHARSET: return "CHARSET";
      case RSP: return "RSP";
      case COM_PORT_CONTROL: return "COM_PORT_CONTROL";
      case SUPPRESS_LOCAL_ECHO: return "SUPPRESS_LOCAL_ECHO";
      case START_TLS: return "START_TLS";
      case KERMIT: return "KERMIT";
      case SEND_URL: return "SEND_URL";
      case FORWARD_X: return "FORWARD_X";
      case MSDP: return "MSDP";
      case MSSP: return "MSSP";
      case MCCP1: return "MCCP1";
      case MCCP2: return "MCCP2";
      case ZMP: return "ZMP";
      case MUX: return "MUX";
      case GMCP: return "GMCP";
      case PRAGMA_LOGON: return "PRAGMA_LOGON";
      case SSPI_LOGON: return "SSPI_LOGON";
      case PRAGMA_HEARTBEAT: return "PRAGMA_HEARTBEAT";
      default: return "UNKNOWN_OPT";
    }
  };

  std::cout << "[TELNET " << ((who == 0) ? "->" : "<-") << " ] " << cmdName(buffer[1])
            << " " << optName(buffer[2])
            << " (IAC " << (int)buffer[1] << " " << (int)buffer[2] << ")\n";
}
