# Simple Shell

A Linux command-line shell written in C, built for CS 446 (Operating Systems) at the University of Nevada, Reno.

It works like a simplified version of the Terminal: it shows a prompt, reads what the user types, and runs it.

## Features
- **Runs commands** like `ls` or `date`, by creating a child process with `fork()` and replacing it with the requested program using `execvp()`
- **Input and output redirection:** `sort < input.txt` reads from a file, and `ls > output.txt` writes to one
- **Pipes:** `ls | grep txt` sends the output of one command into another
- **Built-in commands:** `cd` to change directories and `exit` to quit
- **Error handling:** reports failed commands, bad paths, and non-zero exit statuses

## How to Run
```bash
gcc simpleshell.c -o simpleshell
./simpleshell
```

## Example
```
bpicassokennedy:/home/user$ ls | grep .c
simpleshell.c
bpicassokennedy:/home/user$ cd ..
bpicassokennedy:/home$ exit
```

## System Calls Used
`fork()`, `execvp()`, `pipe()`, `dup2()`, `open()`, `waitpid()`, `chdir()`, `getcwd()`
