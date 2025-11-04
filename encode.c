#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

int extn_size;

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Move to offset 18 where width is stored
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (integer - 4 bytes)
    fread(&width, sizeof(int), 1, fptr_image);
    // printf("           Width = %u\n", width);

    // Read the height (integer - 4 bytes)
    fread(&height, sizeof(int), 1, fptr_image);
    // printf("           Height = %u\n", height);

    // Return image capacity = width * height * 3 bytes
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Move to end of file to determine file size
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    return size;
}

/*
 * Validate file extensions for supported formats
 * Inputs: file name, list of valid extensions, count
 * Output: e_success or e_failure
 */
Status validate_file_extension(const char *filename, char *valid_extns[], int extn_count)
{
    // Check if filename starts with '.'
    if (filename[0] == '.')
    {
        fprintf(stderr, "Error: Invalid file name '%s' — missing name before '.'\n", filename);
        return e_failure;
    }

    // Find last '.' in the filename
    char *dot = strrchr(filename, '.');
    if (dot == NULL)
    {
        fprintf(stderr, "Error: File '%s' has no extension.\n", filename);
        return e_failure;
    }

    // Compare with allowed extensions
    for (int i = 0; i < extn_count; i++)
    {
        if (strcmp(dot, valid_extns[i]) == 0)
            return e_success;
    }

    fprintf(stderr, "Error: '%s' has invalid extension.\n", filename);
    return e_failure;
}

/*
 * Read and validate input arguments for encoding
 * Ensures source, secret, and output files are correct
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Validate source image (must be .bmp)
    char *bmp_ext[] = {".bmp"};
    if (validate_file_extension(argv[2], bmp_ext, 1) == e_success)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        fprintf(stderr, "Error: Invalid source file '%s'. Must be a .bmp file.\n\n", argv[2]);
        return e_failure;
    }

    // Validate secret file (allowed: .txt, .c, .h, .sh)
    char *secret_ext[] = {".txt", ".c", ".h", ".sh"};
    if (validate_file_extension(argv[3], secret_ext, 4) == e_success)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        fprintf(stderr, "Error: Invalid secret file '%s'. Must be .txt, .c, .h, or .sh.\n\n", argv[3]);
        return e_failure;
    }

    // Check for optional output filename
    if (argv[4] == NULL)
    {
        encInfo->stego_image_fname = "destination.bmp"; // Default output name
    }
    else
    {
        if (validate_file_extension(argv[4], bmp_ext, 1) == e_success)
        {
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            fprintf(stderr, "Error: Invalid output file '%s'. Must be a .bmp file.\n\n", argv[4]);
            return e_failure;
        }
    }
    return e_success;
}

/*
 * Opens source image, secret, and output stego files
 * Returns e_success if all opened correctly
 */
Status open_files(EncodeInfo *encInfo)
{
    // Open source image in read-binary mode
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    // Open secret file in read-binary mode
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    // Open destination stego image in write-binary mode
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

/*
 * Check if source image has enough capacity to hold secret data
 */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // Identify and store file extension of secret file
    char *extn;
    if (strstr(encInfo->secret_fname, ".txt") != NULL)
        extn = strstr(encInfo->secret_fname, ".txt");
    else if (strstr(encInfo->secret_fname, ".c") != NULL)
        extn = strstr(encInfo->secret_fname, ".c");
    else if (strstr(encInfo->secret_fname, ".h") != NULL)
        extn = strstr(encInfo->secret_fname, ".h");
    else if (strstr(encInfo->secret_fname, ".sh") != NULL)
        extn = strstr(encInfo->secret_fname, ".sh");

    strcpy(encInfo->extn_secret_file, extn); // Store extension
    extn_size = strlen(extn);

    // Calculate total bytes needed for encoding
    int total_bytes = 54 + (strlen(MAGIC_STRING) * 8) + 32 +
                      (extn_size * 8) + 32 + (encInfo->size_secret_file * 8);

    // Compare available vs required capacity
    if (encInfo->image_capacity > total_bytes)
        return e_success;
    else
        return e_failure;
}

/*
 * Copy first 54 bytes (BMP header) from source to destination
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];
    rewind(fptr_src_image);
    fread(buffer, 54, 1, fptr_src_image);
    fwrite(buffer, 54, 1, fptr_dest_image);
    if (ftell(fptr_src_image) == ftell(fptr_dest_image))
        return e_success;
    else
        return e_failure;
}

/*
 * Encode the magic string into the LSBs of image data
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];
    for (size_t i = 0; i < strlen(magic_string); i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

/*
 * Encode secret file extension size (32 bits)
 */
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

/*
 * Encode secret file extension characters into LSBs
 */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    for (size_t i = 0; i < strlen(file_extn); i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

/*
 * Encode secret file size (4 bytes -> 32 bits)
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

/*
 * Encode the actual secret file data into LSBs
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    fread(encInfo->secret_data, encInfo->size_secret_file, 1, encInfo->fptr_secret);
    char buffer[8];
    for (size_t i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(encInfo->secret_data[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}

/*
 * Copy any remaining image data to complete the stego file
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

/*
 * Encode a single byte into the LSBs of 8 image bytes
 */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (size_t i = 0; i < 8; i++)
    {
        image_buffer[i] = image_buffer[i] & (~1) | (data >> i) & 1;
    }
    return e_success;
}

/*
 * Encode an integer (size or length) into 32 image bytes
 */
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (size_t i = 0; i < 32; i++)
    {
        imageBuffer[i] = (imageBuffer[i] & (~1)) | ((size >> i) & 1);
    }
    return e_success;
}

/******************************************************************************
 * Function: do_encoding
 * Description:
 *   Performs the overall encoding process by hiding secret data inside
 *   a BMP image using the Least Significant Bit (LSB) method.
 ******************************************************************************/
Status do_encoding(EncodeInfo *encInfo)
{
    printf("\n========================================\n");
    printf(" 🔐 Starting Encoding Process\n");
    printf("========================================\n\n");

    // Step 1: Open files
    if (open_files(encInfo) == e_success)
    {
        printf("-> Step 1: Opened required files successfully.\n");

        // Step 2: Check capacity
        if (check_capacity(encInfo) == e_success)
        {
            printf("-> Step 2: Source image has sufficient capacity.\n");

            // Step 3: Copy BMP header
            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("-> Step 3: BMP header copied successfully.\n");

                // Step 4: Encode magic string
                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("-> Step 4: Magic string encoded successfully.\n");

                    // Step 5: Encode secret file extension size
                    if (encode_secret_file_extn_size(extn_size, encInfo) == e_success)
                    {
                        printf("-> Step 5: Secret file extension size encoded successfully.\n");

                        // Step 6: Encode secret file extension
                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("-> Step 6: Secret file extension encoded successfully.\n");

                            // Step 7: Encode secret file size
                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("-> Step 7: Secret file size encoded successfully.\n");

                                // Step 8: Encode secret file data
                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("-> Step 8: Secret file data encoded successfully.\n");

                                    // Step 9: Copy remaining image data
                                    if (copy_remaining_img_data(encInfo->fptr_src_image,
                                                                encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("-> Step 9: Remaining image data copied successfully.\n");
                                        return e_success;
                                    }
                                    else
                                    {
                                        printf("❌ ERROR: Copying remaining image data failed!\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("❌ ERROR: Encoding secret file data failed!\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("❌ ERROR: Encoding secret file size failed!\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("❌ ERROR: Encoding secret file extension failed!\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("❌ ERROR: Encoding secret file extension size failed!\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("❌ ERROR: Encoding magic string failed!\n");
                    return e_failure;
                }
            }
            else
            {
                printf("❌ ERROR: Copying BMP header failed!\n");
                return e_failure;
            }
        }
        else
        {
            printf("❌ ERROR: Source image does not have enough capacity to encode data.\n");
            return e_failure;
        }
    }
    else
    {
        printf("❌ ERROR: Opening files failed!\n");
        return e_failure;
    }

    return e_failure;
}
