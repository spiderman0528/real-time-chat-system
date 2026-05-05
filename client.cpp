#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <cstring>

using namespace std;

// ===================== RECEIVE THREAD =====================
void receiveMessages(int socket) {
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int bytes = read(socket, buffer, sizeof(buffer));

        if (bytes <= 0) {
            cout << "Disconnected from server\n";
            break;
        }

        cout << buffer << endl;
    }
}

// ===================== MAIN =====================
int main() {

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        cout << "Connection failed\n";
        return -1;
    }

    cout << "Connected to chat server\n";

    // Send username first
    string username;
    cout << "Enter username: ";
    getline(cin, username);

    send(sock, username.c_str(), username.size(), 0);

    // Start receive thread
    thread t(receiveMessages, sock);
    t.detach();

    // Send messages
    string message;

    while (true) {
        getline(cin, message);

        send(sock, message.c_str(), message.size(), 0);

        if (message == "/quit") {
            break;
        }
    }

    close(sock);
    return 0;
}