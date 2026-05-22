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

Compile the source code:

Bash
   gcc -Wall -Wextra -O2 shell.c -o osh
Launch the shell:

Bash
   ./osh
💻 Usage Examples
1. Basic Commands & Built-ins

Bash
osh> pwd
osh> cd /tmp
osh> ls -la
2. Standard I/O Redirection

Bash
osh> echo "Hello, Systems Programming!" > output.txt
osh> cat < output.txt
3. Process Pipelining

Bash
osh> cat shell.c | grep "fork" | wc -l
4. Background Jobs

Bash
osh> sleep 10 &
[Process spawned with PID: 54321]
osh> echo "I can keep typing!"
🗺️ Roadmap & Future Enhancements
[ ] Multi-Pipelining: Support unlimited chaining (cmd1 | cmd2 | cmd3).

[ ] Job Control System: Implement jobs, fg, and bg commands tracking PIDs in a struct array.

[ ] Environment Variables: Expansion for strings like $HOME and $PATH.

[ ] Command History: Implement up/down arrow memory (via readline or custom buffer).

📜 License
Distributed under the MIT License. See LICENSE for more information.
