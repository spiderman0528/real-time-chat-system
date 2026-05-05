#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <cstring>

using namespace std;

// ================= GLOBAL STATE =================
unordered_map<string, int> clients;
mutex clients_mutex;

vector<string> message_history;
const int MAX_HISTORY = 50;

// ================= UTIL =================
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

// ================= BROADCAST =================
void broadcast(const string &msg, int sender_socket) {
    lock_guard<mutex> lock(clients_mutex);

    for (auto &c : clients) {
        if (c.second != sender_socket) {
            send(c.second, msg.c_str(), msg.size(), 0);
        }
    }
}

// ================= USER LIST =================
string getUsers() {
    lock_guard<mutex> lock(clients_mutex);

    string list = "Online users:\n";
    for (auto &c : clients) {
        list += "- " + c.first + "\n";
    }
    return list;
}

// ================= HANDLE CLIENT =================
void handle_client(int client_socket) {

    char buffer[1024] = {0};
    int n = read(client_socket, buffer, 1024);

    if (n <= 0) {
        close(client_socket);
        return;
    }

    string username = clean(string(buffer));

    {
        lock_guard<mutex> lock(clients_mutex);
        clients[username] = client_socket;
    }

    // ================= SEND HISTORY =================
    for (auto &m : message_history) {
        send(client_socket, m.c_str(), m.size(), 0);
    }

    string join_msg =
        "🔵 [" + getTime() + "] " + username + " joined the chat\n";

    cout << join_msg;
    broadcast(join_msg, client_socket);

    // ================= MAIN LOOP =================
    while (true) {

        char msg_buffer[1024] = {0};
        int bytes = read(client_socket, msg_buffer, 1024);

        string msg = clean(string(msg_buffer));

        // handle disconnect safely
        if (bytes <= 0 || msg == "/quit") {
            break;
        }

        // ================= COMMANDS =================
        if (msg == "/help") {
            string help = "Commands: /help /users /quit /msg <user> <msg>\n";
            send(client_socket, help.c_str(), help.size(), 0);
            continue;
        }

        if (msg == "/users") {
            string list = getUsers();
            send(client_socket, list.c_str(), list.size(), 0);
            continue;
        }

        // ================= PRIVATE MESSAGE =================
        if (msg.rfind("/msg ", 0) == 0) {

            size_t first = msg.find(" ");
            size_t second = msg.find(" ", first + 1);

            if (second == string::npos) {
                string err = "Usage: /msg <user> <message>\n";
                send(client_socket, err.c_str(), err.size(), 0);
                continue;
            }

            string target = msg.substr(first + 1, second - first - 1);
            string text = msg.substr(second + 1);

            lock_guard<mutex> lock(clients_mutex);

            if (clients.find(target) != clients.end()) {

                string pm = "[PRIVATE] " + username + ": " + text + "\n";
                send(clients[target], pm.c_str(), pm.size(), 0);

                string confirm = "[You -> " + target + "]: " + text + "\n";
                send(client_socket, confirm.c_str(), confirm.size(), 0);

            } else {
                string err = "User not found\n";
                send(client_socket, err.c_str(), err.size(), 0);
            }

            continue;
        }

        // ================= NORMAL MESSAGE =================
        string final_msg =
            "💬 [" + getTime() + "] " + username + ": " + msg + "\n";

        cout << final_msg;

        {
            lock_guard<mutex> lock(clients_mutex);
            message_history.push_back(final_msg);

            if (message_history.size() > MAX_HISTORY) {
                message_history.erase(message_history.begin());
            }
        }

        broadcast(final_msg, client_socket);
    }

    // ================= CLEANUP =================
    {
        lock_guard<mutex> lock(clients_mutex);
        clients.erase(username);
    }

    string leave_msg =
        "🔴 [" + getTime() + "] " + username + " disconnected\n";

    cout << leave_msg;
    broadcast(leave_msg, client_socket);

    close(client_socket);
}

// ================= MAIN =================
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

    cout << "\n=================================\n";
    cout << "   CHAT SERVER RUNNING (PORT 8080)\n";
    cout << "   Multi-client system active\n";
    cout << "=================================\n\n";

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