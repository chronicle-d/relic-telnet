# Relic Telnet

## Jun 23 | Starting out

My influence to write my own **telnet** client for **Chronicle** was [this](https://www.reddit.com/r/cpp_questions/comments/2wrlpv/comment/cotoari/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button) comment in a Reddit thread, and the fact that my current **CPP** knowledge is very limited and this project might prove as a _stepping stone_ in my **CPP** journey.

I found the following links that might help me get stated with this _sub_ project:

- [List of RFC](https://en.wikipedia.org/wiki/Telnet#Internet_Standards)s.
- [Simple implementation](https://listarchives.boost.org/boost-users/att-40895/telnet.cpp)
- [Full fledged implementation](https://github.com/KazDragon/telnetpp/)
- [Simple TCP session](https://medium.com/@naseefcse/ip-tcp-programming-for-beginners-using-c-5bafb3788001)

---

Its been a few hours since I sat down, I am currently working on a simple **TCP** class as oppose to what the **Reddit** thread recommended (using `boost`).

I decided this should be a _header only_ library, I also decided that since I prioritize efficiency over comfort, **exceptions** were not an option since any errors in this library would be _logic errors_ (and exceptions are slow).

My way of returning an error is via a numeric value:

- **0** - General success.
- **200 <** - `errno` error.
- **200 >** - Relic telnet errors.

Now users can determine the error cause with ease.

### TCP Handler

I only need to implement a way to let users **read** from the _file descriptor_. I am using the [man page of **read**](https://man7.org/linux/man-pages/man2/read.2.html) for that sake.

I now have a simple tester for the **TCP** connection:

```cpp
#include "rtelnet.hpp"
/*
* Relic Telnet is a header only library, this file tests the functionality of rtelnet.
*/

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc < 2) { std::cerr << "Usage: " << argv[0] << " ADDRESS PORT"; return 1; }

  rtnt::tcp TCP;

  try {
    sockaddr_in serverAddress = rtnt::unwrapOrThrow(TCP.getSocketAddr(argv[1], std::atoi(argv[2])));

    int fd = TCP.Connect(serverAddress);
    if (fd < 0) { throw std::runtime_error(rtnt::readError(fd));}

    std::cout << "Sending...\n";
    TCP.Send("Hello!\n", fd);
    std::cout << "Sent.\n";

    std::cout << "Reading.\n";
    std::string buffer = rtnt::unwrapOrThrow(TCP.Read(fd));
    std::cout << "Got: " << buffer;
    std::cout << "Done.\n";

    TCP.Close(fd);
  } catch (std::exception& e) {
    std::cerr << "[ERROR]: " << e.what() << "\n";
  }

  return 0;
}
```

And I can test it by using:

```bash
nc -l -p 8080
```

Yet sadly it seems like I cannot establish a connection...

```
noam ◈ noam ⊛ bin ⊛ ❯❯ ./relic-telnet-tester 127.0.0.1 8080
[ERROR]: faild connection to server.
```

I want to check if there is an `errors` to the `connect()` function as well.

Yep

```
RETURN VALUE
       If the connection or binding succeeds, zero is returned.  On error, -1 is returned, and errno is set to  in‐
       dicate the error.
```

Ok, I'll add a return the `errno` instead of the custom error.

```
noam ◈ noam ⊛ bin ⊛ ❯❯ ./relic-telnet-tester 127.0.0.1 8080
[ERROR]: Connection refused
```

AYO

This is cool.

---

Ok the issue was with the `_connected` boolean state, I fixed it and now everything works!

Sending hello:

```
noam ◈ noam ⊛ bin ⊛ ❯❯ ./relic-telnet-tester 127.0.0.1 8080
Sending...
Sent.
Reading.

```

> Server:
>
> ```
> noam ◈ noam ⊛ bin ⊛ ❯❯ nc -l -p 8080
> Hello!
> ```

Sending back something from server:

```
noam ◈ noam ⊛ bin ⊛ ❯❯ nc -l -p 8080
Hello!
Hey!
noam ◈ noam ⊛ bin ⊛ ❯❯
```

> Client:
>
> ```
> noam ◈ noam ⊛ bin ⊛ ❯❯ ./relic-telnet-tester 127.0.0.1 8080
> Sending... Sent.
> Reading.
> Got: Hey!
> Done.
> noam ◈ noam ⊛ bin ⊛ ❯❯
> ```

I have a working **TCP** handler!
