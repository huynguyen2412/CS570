p2 interpret the first word as name of an executable file
- the shell must be able to handle redirection of I/O using the words "<", ">", "|"
Eg: cat somefile > myexistingfile will NOT fork if the file existed
stderr: myexistingfile: File exists
- If the terminator is "&", p2 will start new process to execute the file, print out PID of that process
continue WITHOUT waiting for the completion of the child.
- MUST use the command killpq(getpgrp(), SIGTERM) when it is time for p2 to close up
Use getpgrp() at the start to give p2 its own
The getpgrp() function shall return the process group ID of the calling process, no return value is reserved to indicate an error.
~cs570/sighandler.c to 'catch' the signal, so that p2 survives the SIGTERM

Only have to handle 0 and 1 additional arguments for "cd" and "ls-F"
Handle "cd" command (no child is forked)
- p2 should handle cd(1) correctly without forking a child
- cd with no parameter consider cd $HOME (HOME can obtained with getenv())
- generate an error message if presented with more than one argument
- NOT conduct meaning of "~" character

Handle 'ls-F' command as a built-in (no child is forked)
- see ~cs570/dir.c for an illustration of the relevant system calls
- only handle 0 or 1 additional argument, if more than 1 should ignore all but the first one
- if no other argument (argc = 1) except for the "ls-F", do command "real" ls does: treat as "ls ."
If "ls-F -l" error message -l: No such file or directory
if want to execute -l , child should exec() "/bin/ls" 
- print out the names, one presented line, same order while traversing

Handle '|' pipe line (only handle or 1 pipeline) (***See pipe.c page 6***)
- if two '|' characters appear on the command line, flagged as a syntax error
- command 1 | process 2
will require 2 children to be forked, one child to exec command1 and another child to exec process2.

Only use execvp()

To print error use
- fprintf(stderr, "error \n", );

Use fflush before fork() a child to remove the input for next input