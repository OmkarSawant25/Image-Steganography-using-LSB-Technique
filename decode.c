#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/*
 * Checks if the given file name has a valid extension (like .bmp)
 */
Status validate_file_extension_decode(const char *filename, char *valid_extns[], int extn_count)
{
    // Make sure filename doesn’t start with a dot
    if (filename[0] == '.')
    {
        fprintf(stderr, "Error: Invalid file name '%s' — missing name before '.'\n", filename);
        return e_failure;
    }

    // Find the file extension
    char *dot = strrchr(filename, '.');
    if (dot == NULL)
    {
        fprintf(stderr, "Error: File '%s' has no extension.\n", filename);
        return e_failure;
    }

    // Compare with valid extensions
    for (int i = 0; i < extn_count; i++)
    {
        if (strcmp(dot, valid_extns[i]) == 0)
            return e_success;
    }

    // fprintf(stderr, "Error: '%s' has invalid extension.\n", filename);
    return e_failure;
}

/*
 * Reads and validates command-line arguments for decoding.
 * Makes sure the stego (encoded) image is a .bmp
 * and prepares the output file name.
 */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *bmp_ext[] = {".bmp"};

    // Validate the input image file
    if (validate_file_extension_decode(argv[2], bmp_ext, 1) == e_success)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf("Error: '%s' has invalid extension. Must be a .bmp file.\n\n", argv[2]);
        return e_failure;
    }

    // If user didn’t give an output file name, use “Decoded” by default
    if (argv[3] == NULL)
    {
        decInfo->secret_fname = "Decoded";
    }
    else
    {
        // Remove any extension from the provided output name
        char *str;
        str = strtok(argv[3], ".");
        decInfo->secret_fname = str;
    }
    return e_success;
}

/*
 * Opens the encoded (stego) BMP image file for reading.
 */
Status open_decoded_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }
    return e_success;
}

/*
 * Skips the first 54 bytes of the BMP file (header section)
 * since we only want the pixel data.
 */
Status skip_bmp_header(FILE *fptr_dest_image)
{
    fseek(fptr_dest_image, 54, SEEK_SET);
    return e_success;
}

/*
 * Checks for the special magic string to verify that
 * the image actually contains hidden data.
 */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char string[20];
    char ch;
    char buffer[8];
    int i;

    // Decode the first few bytes to reconstruct the magic string
    for (i = 0; i < 2; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&ch, buffer);
        string[i] = ch;
    }
    string[i] = '\0';

    // Compare the decoded string with our known MAGIC_STRING
    if (strcmp(MAGIC_STRING, string) == 0)
        return e_success;

    return e_failure;
}

/*
 * Extracts a single byte from 8 pixels (LSB method).
 */
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    char new = 0;
    for (size_t i = 0; i < 8; i++)
    {
        new = new | (image_buffer[i] & 1) << i;
    }
    *data = new;
    return e_success;
}

/*
 * Extracts an integer value (like size) from 32 pixels.
 */
Status decode_size_from_lsb(int *size, char *imageBuffer)
{
    int new_size = 0;
    for (size_t i = 0; i < 32; i++)
    {
        new_size = new_size | (imageBuffer[i] & 1) << i;
    }
    *size = new_size;
    return e_success;
}

/*
 * Reads and decodes the length of the secret file extension.
 */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    int size;

    fread(buffer, 32, 1, decInfo->fptr_stego_image);
    decode_size_from_lsb(&size, buffer);

    decInfo->ext_size = (long)size;
    return e_success;
}

/*
 * Reads and decodes the actual extension (e.g., .txt, .c)
 * and rebuilds the decoded output filename.
 */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];
    char extn[5];
    char ch;
    int i;

    // Decode the extension character by character
    for (i = 0; i < decInfo->ext_size; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&ch, buffer);
        extn[i] = ch;
    }
    extn[i] = '\0';

    static char new_fname[100];
    sprintf(new_fname, "%s%s", decInfo->secret_fname, extn);
    decInfo->secret_fname = new_fname;   

    return e_success;
}

/*
 * Reads and decodes the total size of the hidden secret file.
 */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    int size;

    fread(buffer, 32, 1, decInfo->fptr_stego_image);
    decode_size_from_lsb(&size, buffer);
    decInfo->size_secret_file = size;

    return e_success;
}

/*
 * Decodes the actual secret data and writes it to a new file.
 */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    // Open the output file to save the decoded content
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");

    // Decode byte by byte and write it into the output file
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&ch, buffer);
        fwrite(&ch, 1, 1, decInfo->fptr_secret);
    }
    return e_success;
}

/*
 * The main decoding process that performs all steps one by one.
 */
Status do_decoding(DecodeInfo *decInfo)
{
    printf("\n========================================\n");
    printf(" 🔓 Starting Decoding Process\n");
    printf("========================================\n\n");

    // Step 1: Decode the size of the file extension
    if (decode_secret_file_extn_size(decInfo) == e_success)
    {
        printf("-> Step 1: Secret file extension size decoded successfully.\n");

        // Step 2: Decode the actual file extension
        if (decode_secret_file_extn(decInfo) == e_success)
        {
            printf("-> Step 2: Secret file extension decoded successfully.\n");

            // Step 3: Decode the secret file’s total size
            if (decode_secret_file_size(decInfo) == e_success)
            {
                printf("-> Step 3: Secret file size decoded successfully.\n");

                // Step 4: Decode the secret file content
                if (decode_secret_file_data(decInfo) == e_success)
                {
                    printf("-> Step 4: Secret file data decoded successfully.\n");
                    return e_success;
                }
                else
                {
                    printf("❌ ERROR: Decoding secret file data failed!\n");
                }
            }
            else
            {
                printf("❌ ERROR: Decoding secret file size failed!\n");
            }
        }
        else
        {
            printf("❌ ERROR: Decoding secret file extension failed!\n");
        }
    }
    else
    {
        printf("❌ ERROR: Decoding secret file extension size failed!\n");
    }

    printf("========================================\n");
    printf(" ❌ Decoding process terminated with errors.\n");
    printf("========================================\n\n");
    return e_failure;
}
