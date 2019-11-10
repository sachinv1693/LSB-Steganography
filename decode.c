#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

//Extern required variables from main file
extern uint passcode_flag, passcode_len, default_ext_name;
extern uchar temp_decode_name[MAX_FILENAME_SIZE]; 

Status do_decoding(EncodeInfo* encInfo)
{
		uchar_ptr receive_str = NULL;//Pointer to decoded string	
		printf("INFO: Decoding Magic String Signature\n");
		//If first decoded byte is * then no pass code. If 1st byte is # then pass code expected
		receive_str = decode_magic_string(CHAR_SIZE, encInfo);
		if (!strcmp((const char*) receive_str, (const char*) MAGIC_STRING))
		{
				printf("INFO: Magic string signature successfully decoded!!\n");
				if (passcode_flag)
				{
						printf("INFO: No passcode expected!! Neglecting last 2 arguments..\n");
				}
		}
		else if (!strcmp((const char*)receive_str, (const char*)MAGIC_STRING_WITH_PASSCODE))
		{
				printf("INFO: Magic string signature successfully decoded!!\nINFO: Expecting passcode entry..\n");
				if (passcode_flag)
				{
				printf("INFO: A passocde has been provided!! Verifying passcode..\n");
				//Decode the passcode length
				printf("INFO: Decoding passcode length..\n");
			    uint decoded_passcode_len = decode_int_size_expression(encInfo);	
				if (decoded_passcode_len)  
				{
						printf("INFO: Successfully decoded the passcode length.\n");	
				if (passcode_len == decoded_passcode_len)
				{
						printf("INFO: Length of user provided passcode matched with decoded passcode length!!\n");
				}
				else
				{
						printf("ERROR: Incorrect passcode length given!! Expecting %d digit passcode.\n\n", decoded_passcode_len);
						return e_failure;
				}
				}
				else
				{
						printf("ERROR: Failed to decode the passcode length!!\n\n");
						return e_failure;
				}
				//Decode the passcode
				printf("INFO: Decoding the passcode..\n");
				receive_str = decode_magic_string(passcode_len, encInfo);
				if (receive_str)
				{
						printf("INFO: Successfully decoded the passcode.\n");
				}
				else
				{
						printf("INFO: Failed to decode the passcode.\n\n");
						return e_failure;
				}
						if (!strcmp((const char*)receive_str, (const char*)encInfo->passcode))
								printf("INFO: Passcode matched!!\n");
						else
						{
								printf("ERROR: Incorrect password provided!!\n\n");
								return e_failure;
						}
				}
				else
				{
						printf("ERROR: No passcode given. Please provide -p followed by passcode from the command line.\n\n");
						return e_failure;
				}
		}
		else
		{
				printf("ERROR: Failed to decode magic string signature!!\n\n");
				return e_failure;
		}
		//Decode File Extension size
		printf("INFO: Decoding secret file extension size\n");
		uint dec_extn_size = decode_int_size_expression(encInfo);
		if (dec_extn_size)
		{
				printf("INFO: Successfully decoded secret file extension size. It has %u characters.\n", dec_extn_size);
		}
		else
		{
				printf("ERROR: Failed to decode the secret file extension size!!\n\n");
				return e_failure;
		}
		//Decode the dot
		printf("INFO: Decoding the dot in secret filename\n");
		receive_str = decode_magic_string(CHAR_SIZE, encInfo);
		if (!strcmp((const char*)receive_str, (const char*)"."))
		{
				printf("INFO: Successfully decoded the dot in the secret filename!!\n");
		}
		else
		{
				printf("ERROR: Failed to decode the dot in the secret filename.\n\n");
				return e_failure;
		}
		//Decode File extension
		printf("INFO: Decoding secret file extension\n");
		receive_str = decode_magic_string(dec_extn_size, encInfo);
		if (receive_str)
		{
				printf("INFO: Successfully decoded the secret file extension!!\n");
				if (default_ext_name)
				{
						strcpy((char*) encInfo->decoded_fname, (const char*) "decoded.");
						strcat((char*)encInfo->decoded_fname, (const char*)receive_str);
						printf("INFO: No decode filename provided, creating default decode file %s\n", encInfo->decoded_fname);
				}
				else
				{
						//Validate and assign 3rd command line argument as secret filename
						printf("INFO: Verifying user provdided decode file extension..\n");
						if (read_and_validate_extn(temp_decode_name, encInfo))
						{
								printf("INFO: Valid decode file extension.\n");
						}
						else
						{
								printf("ERROR: Invalid decode file extension.\n\n");
								return e_failure;
						}
						strcpy((char*)encInfo->decoded_fname, (const char*)temp_decode_name);
						printf("INFO: Verifying user given decode file extension with that of the secret file..\n");
						if (!strcmp((const char*)encInfo->extn_secret_file, (const char*)receive_str))
						{
								printf("INFO: Decode file extension matched!!\n");
						}
						else
						{
								printf("ERROR: Incorrect decode file extension. Decode file must be a '.%s' file!!\n", receive_str);
								return e_failure;
						}
				}
		}
	else
		{
				printf("ERROR: Failed to decode the secret file extension.\n\n");
				return e_failure;
		}
		//Decode secret data size
		printf("INFO: Decoding secret data size\n");
		encInfo->size_secret_file = decode_int_size_expression(encInfo);
		if (encInfo->size_secret_file)
		{
				printf("INFO: Successfully decoded the secret data size. It is %u bytes!!\n", encInfo->size_secret_file);
		}
		else
		{
				printf("ERROR: Failed to decode the secret data size!!\n\n");
				return e_failure;
		}
		//Let's open the decode file for writing the secret data
		printf("INFO: Opening file %s for writing the secret data\n", encInfo->decoded_fname);
		if ((encInfo->fptr_decoded_file = fopen((const char*)encInfo->decoded_fname, "wb")) != NULL)
		{
				printf("INFO: Successfully opened the file.\n");
		}
		else
		{
				printf("ERROR: Unable to open the file %s. This file may not be present in the current project directory.\n\n", encInfo->decoded_fname);
				return e_failure;
		}
		//Decode file data
		printf("INFO: Decoding and storing the secret file data\n");
		if (decode_file_data(encInfo->size_secret_file, encInfo))
		{
				printf("INFO: Successfully decoded the secret file data.\nINFO: Decoded data has been successfully written onto file %s\n", encInfo->decoded_fname);
		}
		else
		{
				printf("ERROR: Failed to decode the secret file data!!\n\n");
		}
		free(receive_str);
		return e_success;//No error found
}

uchar_ptr decode_magic_string(uint size, EncodeInfo* encInfo)
{
		//Pointer to hold the heap memory of given size
		uchar_ptr decoded_str = (uchar_ptr) malloc(size* sizeof(uchar));
		if (decoded_str == NULL)
		{
				printf("ERROR: Unable to allocate dynamic memory.\n\n");
				exit(e_success);
		}
		uchar scan_char;//Read and store each character
		uint j;//Outer iterator
		for (j = 0; j < size; j++)//Iterate till given string size
		{
				uchar ch = 0;//To store every obtained byte
				for (uint i = 0; i < 8; i++)//8 times inner iteration
				{
						//Read and store each byte
						fread(&scan_char, sizeof(scan_char), 1, encInfo->fptr_stego_image);
						scan_char &= 01;//Obtain the least significant bit
						ch <<= 1;//Left shift by 1 bit to store obtained least significant bit
						ch |= scan_char;//Store the obtained least significant bit
				}
				decoded_str[j] = ch;//Store obtained character into heap
		}
		decoded_str[j] = '\0';//Append NUL character in the end.
		return decoded_str;//Return obtained string
		//Free the heap memory at caller side after executing this function
}

uint decode_int_size_expression(EncodeInfo* encInfo)
{
		uint decoded_int = 0;//To store decoded integer value
		for (uint j = 0; j < INT_SIZE; j++)//Integer size outer iterations
		{
				uchar scan_char = 0;//Read and store each character
				for (uint i = 0; i < 8; i++)//8 times inner iteration
				{
						//Read and store each byte
						fread(&scan_char, sizeof(scan_char), 1, encInfo->fptr_stego_image);
						if (ferror(encInfo->fptr_stego_image))//Error handling
						{
								printf("ERROR: Error in reading from image file.\n\n");
								exit(e_success);
						}
						scan_char &= 01;//Obtain the least significant bit
						decoded_int <<= 1;//Left shift by 1 bit to store obtained least significant bit
						decoded_int |= (uint) scan_char;//Store the obtained least significant bit
				}
		}
		return decoded_int;//Return obtained integer
}

Status decode_file_data(uint f_size, EncodeInfo* encInfo)
{
		//Pointer to hold the heap memory of file size
		uchar_ptr file_data = (uchar_ptr) malloc(f_size * sizeof(uchar));
		if (file_data == NULL)
		{
				printf("ERROR: Unable to allocate dynamic memory.\n\n");
				return e_failure;
		}
		uchar scan_char;//Read and store each character
		for (uint j = 0; j < f_size; j++)//File size outer iterations
		{
				uchar ch = 0;//To store every obtained byte
				for (uint i = 0; i < 8; i++)//8 times inner iteration
				{
						//Read and store each byte from image file
						fread(&scan_char, sizeof(scan_char), 1, encInfo->fptr_stego_image);
						if (ferror(encInfo->fptr_stego_image))//Error handling
						{
								printf("ERROR: Error while reading from file %s\n\n", encInfo->stego_image_fname);
								return e_failure;
						}
						scan_char &= 01;//Obtain the least significant bit
						ch <<= 1;//Left shift by 1 bit to store obtained least significant bit
						ch |= scan_char;//Store the obtained least significant bit
				}
				file_data[j] = ch;//Store obtained character into heap
		}
		//Write obtained heap data onto decoded file
		fwrite(file_data, f_size, 1, encInfo->fptr_decoded_file);
		if (ferror(encInfo->fptr_decoded_file))//Error handling
		{
				printf("ERROR: Error while writing into file %s\n\n", encInfo->decoded_fname);
				return e_failure;
		}
		free(file_data);//Free allocated heap memory
		return e_success;
}
