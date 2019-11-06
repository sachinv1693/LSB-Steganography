#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include <libgen.h>

/* Global variables. These will be externed into other files */
//Store raster data index, file index, length of seret filename
//Flags indicating need of passcode and need of default file extension
uint raster_data, secret_filename_len, default_ext_name = 0, passcode_flag = 0, passcode_len = 0;
//string for storing magic string signature: * or # with '\0' character
uchar magic_string_signature[CHAR_SIZE + CHAR_SIZE];
//string storing temporary user provided decode filename
uchar temp_decode_name[MAX_FILENAME_SIZE];

int main(int argc, char* argv[]) {
		EncodeInfo encInfo;//Structure variable
		printf("\nINFO: Verifying inputs..\n");
		if (argc < 3) {
				printf("ERROR: Invalid number of command line arguments.\n\n");
				exit(e_success);
		}
		if (check_operation_type(argv + 1) == e_encode){
				printf("INFO: Encoding operation requested.\n");
				if (argc < 4 || argc > 7) {
						printf("ERROR: Invalid number of command line arguments.\n\n");
						exit(e_success);
				}
				//Read and validate src_image filename
				//Extract only filename from given path if any
				argv[2] = basename(argv[2]);
				printf("INFO: Verifying source image filename..\n");
				if (read_and_validate_bmp_format(argv + 2) == e_failure) {
						printf("ERROR: Invalid filename provided in 2nd command line argument. It must be a '.bmp' file\n\n");
						exit(e_success);
				}
				printf("INFO: Valid source image filename.\n");
				strcpy((char*)encInfo.src_image_fname, argv[2]);
				//Validate and assign 3rd command line argument as secret filename
				printf("INFO: Verifying secret filename..\n");
				//Extract only filename from given path if any
				argv[3] = basename(argv[3]);
				if (read_and_validate_extn((uchar_ptr)argv[3], &encInfo))
				{
						printf("INFO: Valid secret filename.\n");
				}
				else
				{
						printf("ERROR: Invalid secret filename.\n\n");
						exit(e_success);
				}
				strcpy((char*)encInfo.secret_fname, argv[3]);
				switch (argc) {
						case 4:
								//Giving default output filename since no 4th command line argument is given
								strcpy((char*)encInfo.stego_image_fname, "stego_img.bmp");
								printf("INFO: No output filename given. Creating default output image file %s in current project directory\n", encInfo.stego_image_fname);
								break;
						case 5:
								//Read and validate given output filename
								//Extract only filename from given path if any
								argv[4] = basename(argv[4]);
								printf("INFO: Verifying output image filename..\n");
								if (read_and_validate_bmp_format(argv + 4) == e_failure) {
										printf("ERROR: Invalid filename provided in 4th command line argument. It must be user provided '.bmp' file.\n\n");
										exit(e_success);
								}
								printf("INFO: Valid output image filename.\n");
								strcpy((char*)encInfo.stego_image_fname, argv[4]);
								printf("INFO: Assigning Output filename as %s\n", encInfo.stego_image_fname);
								break;
						case 6:
								if (strcmp(argv[4], "-p")) {
										printf("ERROR: Invalid 4th argument. Please use '-p' and a passcode in the next argument.\n\n");
										exit(e_success);
								}
								printf("INFO: Verifying passcode..\n");
								if (strlen(argv[5]) > MAX_PASSCODE_LEN || no_digits(argv[5])) {
										printf("ERROR: Passcode must include maximum %d 'digits'.\n\n", MAX_PASSCODE_LEN);
										exit(e_success);
								}
								printf("INFO: Passcode accepted!!\n");
								strcpy((char*)encInfo.passcode, argv[5]);
								passcode_flag = 1;
								passcode_len = strlen((const char*)encInfo.passcode);
								//Giving default output filename since no fifth argument is passed
								strcpy((char*)encInfo.stego_image_fname, "stego_img.bmp");
								printf("INFO: No output filename given. Creating default output image file %s in current project directory\n", encInfo.stego_image_fname);
								break;
						default://case 7
								//Read and validate given output filename
								//Extract only filename from given path if any
								argv[4] = basename(argv[4]);
								if (read_and_validate_bmp_format(argv + 4) == e_failure) {
										printf("ERROR: Invalid filename provided in 4th command line argument. It must be user provided output '.bmp' file.\n\n");
										exit(e_success);
								}
								strcpy((char*)encInfo.stego_image_fname, argv[4]);
								printf("INFO: Output filename is provided as %s\n", encInfo.stego_image_fname);
								if (strcmp(argv[5], "-p")) {
										printf("ERROR: Invalid 5th argument. Please use '-p' and a passcode in the next argument.\n\n");
										exit(e_success);
								}
								printf("INFO: Verifying passcode..\n");
								if (strlen(argv[6]) > MAX_PASSCODE_LEN || no_digits(argv[6])) {
										printf("ERROR: Passcode must include maximum %d 'digits'.\n\n", MAX_PASSCODE_LEN);
										exit(e_success);
								}
								printf("INFO: Passcode accepted!!\n");
								strcpy((char*)encInfo.passcode, argv[6]);
								passcode_flag = 1;
								passcode_len = strlen((const char*)encInfo.passcode);
				}
				// Test open_files
				printf("INFO: Opening all the necessary files..\n");
				if (open_files(&encInfo) == e_success) {
						printf("INFO: All files successfully opened.\n\n");
				} else
						exit(e_success);
				printf("INFO: Obtaining offset to image raster data..\n");
				//Collect raster data offset: seek to 10th index of bmp file
				fseek(encInfo.fptr_src_image, 10L, SEEK_SET);
				fread(&raster_data, sizeof(raster_data), 1, encInfo.fptr_src_image);
				if (ferror(encInfo.fptr_src_image)) {//Error handling
						printf("ERROR: Error while reading from file %s\n\n", encInfo.src_image_fname);
						exit(e_success);
				}
				printf("INFO: Offset to image raster data found at %u.\n", raster_data);
				rewind(encInfo.fptr_src_image);
				printf("INFO: Copying image header to output file..\n");
				//Copy image header
				if (copy_bmp_header((FILE*) encInfo.fptr_src_image,(FILE*) encInfo.fptr_stego_image)) {
						printf("INFO: Image header copied to output file successfully.\n");
				} else {
						printf("ERROR: Failed to copy image header\n\n");
						exit(e_success);
				}
				//Image data size should be larger than Magic String Size
				printf("INFO: Verifying source image size..\n");
				encInfo.image_capacity = get_image_size_for_bmp(encInfo.fptr_src_image); 
				if (!encInfo.image_capacity)
				{
						printf("ERROR: Source image file is empty.\n\n");
						exit(e_success);
				}
				printf("INFO: Source image file is not empty.\n");
				printf("INFO: Verifying secret file size..\n");
				encInfo.size_secret_file = get_file_size(encInfo.fptr_secret);
				if (!encInfo.size_secret_file)
				{
						printf("ERROR: Secret file to be encoded is empty.\n\n");
						exit(e_success);
				}
				printf("INFO: Secret file is not empty.\n");
				printf("INFO: Secret data size = %u bytes\n", encInfo.size_secret_file - CHAR_SIZE);//Last byte i.e. EOF is not considered in actual secret data size
				if (passcode_flag)
				{
						//Magic string size would be having additional passcode length + passcode
						//Magic string = MSS + passcode length + passcode + secret file extn size + dot + file extn + secret data size + secret data 
						encInfo.magic_string_size = (CHAR_SIZE + INT_SIZE + passcode_len + INT_SIZE + CHAR_SIZE + encInfo.secret_extn_len + INT_SIZE + encInfo.size_secret_file - CHAR_SIZE) * 8;
						strcpy((char*)magic_string_signature, MAGIC_STRING_WITH_PASSCODE);
				}
				else{//Magic string size without passcode
						//Magic string = MSS + secret file extn size + dot + file extn + secret data size + secret data 
						encInfo.magic_string_size = (CHAR_SIZE + INT_SIZE + CHAR_SIZE + encInfo.secret_extn_len + INT_SIZE + encInfo.size_secret_file - CHAR_SIZE) * 8;
						strcpy((char*)magic_string_signature, MAGIC_STRING);
				}
				//Check encoding capacity
				printf("INFO: Verifying encoding capacity..\n");
				check_capacity(&encInfo) ? printf("INFO: Image data size is sufficient enough to encode the secret data\n"): printf("ERROR: Image data size is insufficient to encode the secret data\n\n");
				printf("\nINFO: ##--------Encoding procedure started---------##\n");
				if (do_encoding(&encInfo))
				{
						printf("INFO: ##------Encoding operation successful!!------##\n\n");
				}
				else
				{
						printf("ERROR: ##------Encoding operation failed!!------##\n\n");
						exit(e_success);
				}
				fclose(encInfo.fptr_src_image);//close source image file
				fclose(encInfo.fptr_secret);//Close secret file
		}
		else if (check_operation_type(argv + 1) == e_decode){//Decoding
				printf("INFO: Decoding operation requested.\n");
				if (argc > 6) {
						printf("Error: Invalid number of command line arguments.\n\n");
						exit(e_success);
				}
				//Read and validate given output filename
				printf("INFO: Verifying image filename..\n");
				//Extract only filename from given path if any
				argv[2] = basename(argv[2]);
				if (read_and_validate_bmp_format(argv + 2) == e_failure) {
						printf("ERROR: Invalid filename provided in 4th command line argument. It must be a '.bmp' file\n\n");
						exit(e_success);
				}
				printf("INFO: Valid image filename.\n");
				strcpy((char*)encInfo.stego_image_fname, argv[2]);
				switch (argc) {
						//For argc = 3, wait for file extension decoding
						case 3: default_ext_name = 1;
								break;
						case 4: //User defined decoded filename
								//Extract only filename from given path if any
								argv[3] = basename(argv[3]);
								strcpy((char*)temp_decode_name, argv[3]);
								printf("INFO: Decode filename is provided as %s\n", temp_decode_name);
								break;
						case 5: //default decoded filename, -p and pass code
								default_ext_name = 1;
								if (strcmp(argv[3], "-p")) {
										printf("ERROR: Invalid 3rd argument. Please use '-p' & a passcode as next argument.\n\n");
										exit(e_success);
								}
								if (strlen(argv[4]) > MAX_PASSCODE_LEN || no_digits(argv[4])) {
										printf("ERROR: Invalid pass code. Passcode must include maximum %d 'digits'\n\n", MAX_PASSCODE_LEN);
										exit(e_success);
								}
								strcpy((char*)encInfo.passcode, argv[4]);
								passcode_flag = 1;
								passcode_len = strlen((const char*)encInfo.passcode);
								break;
						default: // argc = 6, User defined decoded filename, -p and pass code
								//Extract only filename from given path if any
								argv[3] = basename(argv[3]);
								strcpy((char*)temp_decode_name, argv[3]);
								printf("INFO: Decode filename is provided as %s\n", temp_decode_name);
								if (strcmp(argv[4], "-p")) {
										printf("ERROR: Invalid 4th argument. Please use '-p' for pass code & 4-character pass code as next argument.\n\n");
										exit(e_success);
								}
								if (strlen(argv[5]) > MAX_PASSCODE_LEN || no_digits(argv[5])) {
										printf("ERROR: Invalid pass code. Passcode must include maximum %d 'digits'\n\n", MAX_PASSCODE_LEN);
										exit(e_success);
								}
								strcpy((char*)encInfo.passcode, argv[5]);
								passcode_flag = 1;
								passcode_len = strlen((const char*)encInfo.passcode);
				}
				//Open stegged image file
				printf("INFO: Opening the image file\n");
				if((encInfo.fptr_stego_image = fopen((const char*)encInfo.stego_image_fname, "rb")) == NULL)
				{
						printf("ERROR: Unable to open file %s. This file may not be present in the current project directory.\n\n", encInfo.stego_image_fname);
						exit(e_success);
				}
				printf("INFO: Image file successfully opened.\n\n");
				//Collect raster data offset
				printf("INFO: Obtaining offset to image raster data\n");
				fseek(encInfo.fptr_stego_image, 10L, SEEK_SET);
				fread(&raster_data, sizeof(raster_data), 1, encInfo.fptr_stego_image);
				printf("INFO: Offset to image raster data found at %u.\n\n", raster_data);
				fseek(encInfo.fptr_stego_image, raster_data, SEEK_SET);
				if (ferror(encInfo.fptr_stego_image)) {
						printf("ERROR: Error while reading file %s\n\n", encInfo.stego_image_fname);
						exit(e_success);
				}
				//Stego file index is now pointing at the end of raster data
				//Decode Magic String Signature
				printf("INFO: ##--------Decoding procedure started--------##\n");
				if (do_decoding(&encInfo))
				{
						printf("INFO: ##------Decoding operation successful!!------##\n\n");
				}
				else
				{
						printf("ERROR: ##------Decoding operation failed!!------##\n\n");
						exit(e_success);
				}
				//close decoded output file
				fclose(encInfo.fptr_decoded_file);
		} else {//e_unsupported - Neither encoding nor decoding option
				printf("ERROR: 1st command line argument must be either '-e' for encoding or '-d' for decoding\n\n");
				exit(e_success);
		}
		//Close the output file
		fclose(encInfo.fptr_stego_image);//common file both in encoding & decoding part
		return 0;
}
