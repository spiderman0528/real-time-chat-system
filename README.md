# 💬 Real-Time Multi-Client Chat System (C++)

## 📌 Overview
A real-time multi-client chat system built in C++ using socket programming and multithreading. The system enables multiple users to communicate concurrently through a custom client-server architecture with command-based interaction.

---

## ⚙️ Features
- 👥 Multi-client support (concurrent users)
- ⚡ Real-time message broadcasting
- 👤 Username-based messaging system
- 📢 Join/leave notifications
- 💬 Built-in commands:
  - `/help` → show available commands
  - `/users` → list online users
  - `/quit` → disconnect cleanly
- 🔒 Thread-safe server using mutex synchronization
- ⏱️ Timestamped messages (if implemented)

---

## 🛠️ Tech Stack
- C++
- TCP Sockets (POSIX API)
- Multithreading (`std::thread`)
- Mutex synchronization (`std::mutex`)
- Linux (Ubuntu / WSL compatible)

---

## 🏗️ Architecture

Client-Server model:
client 1/2/3 ---> C++ Server ---- broadcasts ----> All Clients