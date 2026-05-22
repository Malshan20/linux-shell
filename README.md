# 🐚 Custom POSIX C-Shell (osh)

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)

A lightweight, robust, and fully functional UNIX shell built entirely in C from scratch. This project demonstrates core systems programming concepts, operating system internals, and low-level process management.

---

## 🚀 Features

* **Command Execution:** Uses the standard `fork()` and `execvp()` lifecycle to run system binaries.
* **I/O Redirection:** Direct output streams to files (`>`) and read inputs from files (`<`) using `dup2()`.
* **Pipelining:** Connect multiple commands together (e.g., `cmd1 | cmd2`) using kernel-level unidirectional `pipe()` channels.
* **Background Processing:** Execute non-blocking background jobs using the `&` operator.
* **Zombie Process Reaping:** Asynchronous `SIGCHLD` signal handling to instantly clean up finished background processes.
* **Robust Signal Handling:** Safely captures `SIGINT` (Ctrl+C) so it interrupts running child processes without killing the shell itself.
* **Built-in Commands:** Native environment implementations of `cd` and `exit`.

---

## 🧠 System Architecture

The shell operates on an infinite **Read-Parse-Execute** loop:
1. **Read:** Pulls raw string input from `stdin`.
2. **Parse:** Tokenizes the string by whitespace, identifying special operators (`|`, `<`, `>`, `&`).
3. **Execute:** 
   * Modifies the environment for built-ins.
   * Clones itself using `fork()`.
   * Maps file descriptors if redirection/piping is present.
   * Replaces the child process memory space with the target program using `execvp()`.

---

## 🛠️ Getting Started

### Prerequisites
* A UNIX-like operating system (Linux, macOS, or WSL on Windows).
* `gcc` (GNU Compiler Collection) or `clang`.
* Standard C libraries (POSIX compliant).

### Installation & Compilation

1. **Clone the repository:**
   
```bash
   git clone [https://github.com/yourusername/c-shell.git](https://github.com/yourusername/c-shell.git)
   cd c-shell
