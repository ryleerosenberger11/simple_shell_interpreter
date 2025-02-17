Rylee Rosenberger

Simple Shell Interpreter

Foreground execution:
usr@host: cwd printed to terminal.
User is able to execute commands in terminal.
Undefined commands will not be executed. Program will output that execvp failed.
ctrl+C will not terminate the ssi.
If the user input is empty (e.g. \n), the shell will continue to prompt the user.
SSI responds to control+C to stop long-running processes like ping

Changing directories:
Special arguments ~, .., ., cd all accepted.
cd takes only one argument and ignores all others.

Background execution:
bg process output is redirected as to not display it on terminal.
bglist lists the current bg processes as follows:
pid: process
Total Background jobs: num_bg_processes
Shell indicates when background processes have terminated.

To run the simple shell interpreter:
make
./ssi
