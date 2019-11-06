#ifndef ENCODE_H
#define ENCODE_H

#include "types.h" // Contains user defined types
#include "common.h"

/* 
 * Structure to store information required for
 * encoding & decoding secret file to/from image file
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_FILENAME_SIZE   50
#define MAX_FILE_SUFFIX     4
#define MAX_PASSCODE_LEN    4
#define INT_SIZE            sizeof(int)
#define CHAR_SIZE           sizeof(char)

typedef struct _EncodeInfo
{
    uchar passcode[MAX_PASSCODE_LEN];
    /* Source Image info */
    FILE *fptr_src_image;
    uchar src_image_fname[MAX_FILENAME_SIZE];
    uint image_capacity;
    /* Secret File Info */
    FILE *fptr_secret;
    uchar secret_fname[MAX_FILENAME_SIZE];
    uchar extn_secret_file[MAX_FILE_SUFFIX + CHAR_SIZE];
    uint size_secret_file;
    uint magic_string_size;
    uint secret_extn_len;
    /* Stego Image Info */
    FILE *fptr_stego_image;
    uchar stego_image_fname[MAX_FILENAME_SIZE];
    /*Decoded File Info */
    FILE* fptr_decoded_file;
    uchar decoded_fname[MAX_FILENAME_SIZE];
} EncodeInfo;

/* Function prototypes */

/* Check operation type */
OperationType check_operation_type(char *argv[]);

/* Read and validate Encode args from argv */
Status read_and_validate_bmp_format(char *argv[]);

/* Digit check in passcode */
Status no_digits(const char* str);

/* Perform encoding */
Status do_encoding(EncodeInfo *encInfo);

/* Perform decoding */
Status do_decoding(EncodeInfo *encInfo);

/* Get File pointers for i/p and o/p files */
Status open_files(EncodeInfo *encInfo);

/* check capacity */
Status check_capacity(EncodeInfo *encInfo);

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
uint get_file_size(FILE *fptr);

/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* Encode Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* Encode secret file extenstion */
Status encode_int_size_expression(uint len, EncodeInfo *encInfo);

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_image_data(FILE *fptr_src, FILE *fptr_dest, uint f_size);

/* Decode magic string */
uchar_ptr decode_magic_string(uint size, EncodeInfo *encInfo);

/* Decode int size expression */
uint decode_int_size_expression(EncodeInfo *encInfo);

/* Read, validate and extract secret file extension */
Status read_and_validate_extn(uchar_ptr sec_file_name_holder, EncodeInfo *encInfo);

/* Decode secret file data */
Status decode_file_data(uint f_size, EncodeInfo *encInfo);

#endif
