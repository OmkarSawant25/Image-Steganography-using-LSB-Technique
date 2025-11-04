#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h" // Contains custom user-defined types like Status, etc.

/*
 * Structure: DecodeInfo
 * ----------------------
 * This structure keeps all the information needed during decoding.
 * It stores file names, file pointers, file sizes, and buffers used
 * while reading the encoded (stego) image and reconstructing the
 * original secret file.
 */
typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;  // Name of the encoded BMP file (input)
    FILE *fptr_stego_image;   // File pointer to the stego image

    /* Secret File Info */
    char *secret_fname;        // Name of the decoded output file
    FILE *fptr_secret;         // File pointer to the output secret file
    long ext_size;             // Size of the secret file extension
    char extn_secret_file[5];  // Stores decoded extension (like .txt)
    char secret_data[100];     // Temporary buffer to store decoded data
    long size_secret_file;     // Total size of the secret file
} DecodeInfo;

/* Decoding function prototype */

/* Reads and validates command-line arguments for decoding */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Controls the full decoding process step by step */
Status do_decoding(DecodeInfo *decInfo);

/* Opens the encoded BMP file for reading */
Status open_decoded_files(DecodeInfo *decInfo);

/* Skips the first 54 bytes (BMP header) to reach pixel data */
Status skip_bmp_header(FILE *fptr_dest_image);

/* Decodes and verifies the magic string to confirm valid encoding */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Reads and decodes the size of the secret file extension */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decodes the actual extension (like .txt, .c, etc.) */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decodes the total size of the secret file */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decodes the hidden secret file content and writes it to a file */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decodes a single byte from 8 pixels (using LSB method) */
Status decode_byte_from_lsb(char *data, char *image_buffer);

/* Decodes an integer value (like size) from 32 pixels */
Status decode_size_from_lsb(int *size, char *imageBuffer);

/* Validates file extension during decoding */
Status validate_file_extension_decode(const char *filename, char *valid_extns[], int extn_count);

#endif
