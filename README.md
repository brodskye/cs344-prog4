# cs344-prog4

Implementation of a <a href="http://en.wikipedia.org/wiki/Onetime_pad"> One Time Pad (OTP) </a> for CS344.
It encrypts and decrypts plaintext into ciphertext, using a randomized key. Compile script 'compileall' provided.

It consists of 5 small programs:

    *) otp_enc_d: Performs the actual encoding. Runs in the background as a daemon. Listens on a particular port for the                       plaintext and the key. When a connection is made to otp_enc, it forks off a seperate process immediately so
                  that it can handle concurrent connections. 
                      *) Syntax: otp_enc_d <listening_port> & 
                  
    *) otp_enc: Connects to otp_enc_d and asks it to perform a onetime pad style encryption as above. When it receives the 
                  encrypted text, it outputs to stdout.
                      *) Syntax: otp_enc <plaintext_file_name> <key_file_name> <port> 
                  
    *) otp_dec_d: Performs exactly like otp_enc_d except that it decrpyts the ciphertext given to it.
                      *) Syntax: otp_dec_d <listening_port> & 
    
    *) otp_dec: Performs excatly like otp_enc except that it asks otp_dec_d to decrypt ciphertext using a passed in ciphertet
                  and key.
                      *) Syntax: otp_dec <plaintext_file_name> <key_file_name> <port>
                      
    *) keygen: Generates a randomized key of specified length using the 26 capital letters and the space character.
                      *) Syntax: keygen <character_length>
                      
                      
Plaintext files provided for testing.
