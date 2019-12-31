Compile all the C files using gcc -Wall *.c
You can create a makefile to build this project if you want

For encoding, you have to use -e followed by an input image filename followed by a secret filename
(Note that these files must be present in the current project directory, otherwise an error message appears. Also note that the input image filename must be having ".bmp" format to avoid error while the secret file can be of any format: .c, .cpp, .txt, .vhdl etc. But  it cannot exceed more than 4 characters because a macro MAX_FILE_SUFFIX has been set to 4. Also don't miss the dot in any filename. It will result in an error)

eg: ./a.out -e beautiful.bmp secret.txt

The above input will generate an additional image file in the project directory namely "stego_img.bmp" which is default output filename
You can provide your own output filename by giving 4th argument as shown below:
    (Note that this filename must be having ".bmp" format, otherwise an error message will be shown)
    
    ./a.out -e beautiful.bmp secret.txt my_stegged_img.bmp
    
If you want to incorporate more security, you can use a passcode for encoding operation using -p followed by the passcode

eg: ./a.out -e beautiful.bmp secret.txt my_stegged_img.bmp -p 123

(Note that passcode can contain maximum 4 "digits". You cannot use a passcode like: 1$3q)
The output image file looks exactly like the input image file. To decode the message inside it, use the decoding technique.

For decoding, you have to use -d followed by output image filename

eg: ./a.out -d stego_img.bmp

The above input will decode the secret message from the image and store in a newly created file "decoded.txt" which is default decoded filename. This file has the same format as that of the input secret file format.
If you want to provide your own decoded filename, you can do so using 3rd argument as shown below:

    ./a.out -d stego_img.bmp my_decoded_file.txt

    (Note that the file format must match with that of the input secret file's format. Otherwise an error message appears)
    
If a passcode is used while encoding, the above input results in an error.
Now user has to provide the correct passcode as shown below:

    ./a.out -d stego_img.bmp -p 123
    
*You can provide the filename by its path like: Projects/Steganography/beautiful.bmp

But the file has to be present in your current project directory.
