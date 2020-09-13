In this assignment, you will be creating five small programs that encrypt and decrypt information using a one-time pad-like system. These programs will combine the multi-processing code you have been learning with socket-based inter-process communication. Your programs will also be accessible from the command line using standard Unix features like input/output redirection, and job control. Finally, you will write a short compilation script.

bash compile all then run with the following syntax:
$ enc_server listening_port
$ enc_client text_file key port