#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <mutex>
#include <ctime>
#include <cstring>

using namespace std;

// ===================== DATA STRUCT =====================
struct Client {
    int socket;
    string username;
};

vector<Client> clients;
mutex clients_mutex;

// ===================== UTILITIES =====================
string getTime() {
    time_t now = time(0);
    string t = ctime(&now);
    return t.substr(11, 8); // HH:MM:SS
}

string clean(string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
        s.pop_back();
    }
    return s;
}

// ===================== BROADCAST =====================
void broadcast(string msg, int senderSocket) {
    lock_guard<mutex> lock(clients_mutex);

    for (auto &c : clients) {
        if (c.socket != senderSocket) {
            send(c.socket, msg.c_str(), msg.size(), 0);
        }
    }
}

// ===================== USER LIST =====================
string getUsers() {
    lock_guard<mutex> lock(clients_mutex);

    string list = "Online users:\n";
    for (auto &c : clients) {
        list += "- " + c.username + "\n";
    }
    return list;
}

// ===================== CLIENT HANDLER =====================
void handle_client(int client_socket) {

    char name_buffer[1024] = {0};
    int n = read(client_socket, name_buffer, 1024);

    if (n <= 0) {
        close(client_socket);
        return;
    }

    string username = clean(string(name_buffer));

    {
        lock_guard<mutex> lock(clients_mutex);
        clients.push_back({client_socket, username});
    }

    string join_msg = "[" + getTime() + "] " + username + " joined the chat\n";
    cout << join_msg;
    broadcast(join_msg, client_socket);

    while (true) {

        char buffer[1024] = {0};
        int bytes = read(client_socket, buffer, 1024);

        if (bytes <= 0) {
            string leave_msg = "[" + getTime() + "] " + username + " left the chat\n";
            cout << leave_msg;
            broadcast(leave_msg, client_socket);
            break;
        }

        string msg = clean(string(buffer));

        // ================= COMMANDS =================
        if (msg == "/help") {
            string help = "Commands: /help /users /quit\n";
            send(client_socket, help.c_str(), help.size(), 0);
            continue;
        }

        if (msg == "/users") {
            string list = getUsers();
            send(client_socket, list.c_str(), list.size(), 0);
            continue;
        }

        if (msg == "/quit") {
            string leave_msg = "[" + getTime() + "] " + username + " left the chat\n";
            cout << leave_msg;
            broadcast(leave_msg, client_socket);
            break;
        }

        // ================= NORMAL MESSAGE =================
        string final_msg =
            "[" + getTime() + "] [" + username + "]: " + msg + "\n";

        cout << final_msg;
        broadcast(final_msg, client_socket);
    }

    close(client_socket);
}

// ===================== MAIN =====================
int main() {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    cout << "Chat server running on port 8080...\n";

    while (true) {

        int client_socket = accept(server_fd, NULL, NULL);

        if (client_socket < 0) continue;

        cout << "New client connected!\n";

        thread t(handle_client, client_socket);
        t.detach();
    }

    close(server_fd);
    return 0;
}