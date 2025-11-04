/*
===============================================================================
Project Title : LSB Image Stegnography
Name          : Omkar Ashok Sawant
Batch Id      : 25021C_309
Date          : 03/11/2025
Language      : C Programming
===============================================================================

📘 Project Overview

This project implements image steganography using the Least Significant Bit (LSB) technique in the C programming language.
It allows users to hide (encode) a secret text or code file inside a (.bmp) image and 
later extract (decode) the hidden information safely without visible distortion of the image.

The system supports multiple file types (e.g .txt, .c, .h, .sh) and performs full data integrity checks during encoding and decoding.

🧩 Features

* 🔒 Secure Data Hiding using LSB bit manipulation.
* 🖼️ Supports 24-bit BMP images.
* 📄 Handles multiple file types (.txt, .c, .h, .sh).
* ✅ Robust validation for file names, extensions, and image capacity.
* 🔍 Magic string verification to ensure correct decoding.
* 🧠 Modular C code with clear function separation for readability.
* 💡 Detailed console messages for easy debugging and understanding.

🧮 Encoding Steps

1. Validate Input Files
   * Source image must be .bmp
   * Secret file can be .txt, .c, .h, .sh
2. Open Required Files (src.bmp, secret.txt, stego.bmp)
3. Check Capacity — Ensure image can hold the secret data.
4. Copy BMP Header (first 54 bytes unchanged)
5. Encode the following sequentially:
   * Magic string (e.g., "#*")
   * Secret file extension size
   * Secret file extension (e.g., .txt)
   * Secret file size
   * Secret file data (actual contents)
6. Copy Remaining Image Data after encoding.
7. Output: Stego image ('destination.bmp') containing the hidden data.

🔍 Decoding Steps

1. Validate and Open Stego Image
2. Skip BMP Header (54 bytes)
3. Read and Verify Magic String
4. Decode Extension Size
5. Decode Extension Name
6. Decode Secret File Size
7. Extract Secret File Data and write to decoded file.

⚠️ Error Handling

* Invalid file names or extensions are reported.
* Missing arguments or corrupted BMP files trigger descriptive error messages.
* Magic string mismatch ensures you don’t decode unencoded files.

🧩 Future Enhancements

* Support for other image formats (PNG, JPEG).
* Password-based encryption before embedding.
* GUI-based front-end for user interaction.
* Batch encoding of multiple files.

🧭 Command Format

./a.out -e <source_image.bmp> <secret_file.txt> [output_image.bmp]
./a.out -d <stego_image.bmp> [output_file_name]

*/

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
#include "common.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    printf("\n========================================\n");
    printf(" 🔐  Steganography using LSB Technique\n");
    printf("========================================\n\n");

    // Step 1: Check for minimum argument count
    if (argc >= 4)
    {
        // Step 2: Check whether encode or decode
        OperationType op_type = check_operation_type(argv[1]);

        /*------- ENCODING SECTION -------*/
       
        if (op_type == e_encode)
        {
            printf("🔒 Selected Encoding Operation\n\n");

            // Step 3: Declare structure variable EncodeInfo
            EncodeInfo enc_info;

            // Step 4: Validate and read encode arguments
            if (read_and_validate_encode_args(argv, &enc_info) == e_success)
            {
                printf("-> Encode arguments validated successfully.\n");

                // Step 5: Call do_encoding
                if (do_encoding(&enc_info) == e_success)
                {
                    printf("\n✅ Encoding completed successfully!\n");
                    printf("📁 Output file generated: %s\n", enc_info.stego_image_fname);
                }
                else
                {
                    printf("\n❌ ERROR: Encoding failed.\n");
                }
            }
            else
            {
                printf("❌ ERROR: Invalid encode arguments.\n");
            }
        }

        /*------- DECODING SECTION -------*/

        else if (op_type == e_decode)
        {
            printf("🔓 Selected decoding operation.\n\n");

            // Step 3: Declare structure variable DecodeInfo
            DecodeInfo dec_info;

            // Step 4: Validate and read decode arguments
            if (read_and_validate_decode_args(argv, &dec_info) == e_success)
            {
                printf("-> Decode arguments validated successfully.\n");

                // Step 5: Open stego image file
                if (open_decoded_files(&dec_info) == e_success)
                {
                    // Step 6: Skip BMP header (first 54 bytes)
                    skip_bmp_header(dec_info.fptr_stego_image);

                    // Step 7: Verify magic string
                    if (decode_magic_string(MAGIC_STRING, &dec_info) == e_success)
                    {
                        // Step 8: Perform decoding process
                        if (do_decoding(&dec_info) == e_success)
                        {
                            printf("\n✅ Decoding completed successfully!\n");
                            printf("📁 Output file generated: %s\n", dec_info.secret_fname);

                        }
                        else
                        {
                            printf("\n❌ ERROR: Decoding failed.\n");
                        }
                    }
                    else
                    {
                        printf("❌ ERROR: Provided image is not an encoded file.\n");
                    }
                }
            }
            else
            {
                printf("❌ ERROR: Invalid decode arguments.\n");
            }
        }
        else
        {
            printf("❌ ERROR: Unsupported operation type.\n\n");
            printf("Use -e for encode or -d for decode.\n\n");
            printf("Usage:\n");
            printf(" 🔎 To Encode: %s -e <source_image.bmp> <secret_file.txt> [output_image.bmp]\n", argv[0]);
            printf(" 🔎 To Decode: %s -d <stego_image.bmp> [output_file_name]\n", argv[0]);
        }
    }

    // Step 9: Handle invalid or insufficient arguments
    else
    {
        printf("❌ ERROR: Invalid number of arguments.\n\n");
        printf("Usage:\n");
        printf(" 🔎 To Encode: %s -e <source_image.bmp> <secret_file.txt> [output_image.bmp]\n", argv[0]);
        printf(" 🔎 To Decode: %s -d <stego_image.bmp> [output_file_name]\n", argv[0]);
    }
    printf("========================================\n\n");


    return 0;
}

//  * Function: check_operation_type
//  * Description: Determines whether user selected encode or decode.

OperationType check_operation_type(char *symbol)
{
    // Step 1: Check whether the symbol is -e or not
    if (strcmp(symbol, "-e") == 0)
        return e_encode;

    // Step 2: Check whether the symbol is -d or not
    else if (strcmp(symbol, "-d") == 0)
        return e_decode;

    // Step 3: Otherwise, return unsupported
    else
        return e_unsupported;
}
