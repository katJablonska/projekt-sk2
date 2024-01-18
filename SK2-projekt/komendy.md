# Server
## Kompilacja
g++ -Wall -Wextra -std=c++20 -o server main.cpp Server.cpp Client.cpp Channel.cpp
clang++ -Wall -Wextra -std=c++20 -o server main.cpp Server.cpp Client.cpp Channel.cpp

## Uruchomienie
ip address
./server 127.0.0.1 8080


# Klient
## Kompilacja
g++ -Wall -Wextra -std=c++20 -o client main.cpp
clang++ -Wall -Wextra -std=c++20 -o client main.cpp

## Uruchomienie
./client [ADRES SERWERA] 8080