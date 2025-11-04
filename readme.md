
# 🔐 Image Steganography using LSB Technique

## 📘 Overview
This project implements **Image Steganography** using the **Least Significant Bit (LSB)** method in **C programming**.  
It enables users to **embed a secret file** (like `.txt`, `.c`, `.h`, `.sh`) inside a **24-bit `.bmp` image**, and later **decode** it safely — all while keeping the image visually unchanged.

The system performs complete validation of files, image capacity, and uses a **magic string** to ensure accurate decoding.

---

## ⚙️ Features
✅ Secure data hiding using LSB manipulation  
🖼️ Supports 24-bit BMP images  
📄 Handles multiple secret file types (.txt, .c, .h, .sh)  
✅ Validates file names, extensions, and storage capacity  
🧠 Modular C design for clarity and maintainability  
🔍 Uses “magic string” to verify encoded images  
💬 Step-by-step console output for transparency  

---

## 🧩 Project Structure
```
├── main.c          # Entry point (handles CLI, user operations)
├── encode.c        # Implements encoding process
├── encode.h        # Structures & function prototypes for encoding
├── decode.c        # Implements decoding process
├── decode.h        # Structures & function prototypes for decoding
├── types.h         # Common enums (Status, OperationType)
├── common.h        # Shared macros (MAGIC_STRING, etc.)
```
---

## 🧠 Working Principle: LSB Encoding
Each pixel in a 24-bit BMP image has 3 bytes (R, G, B).  
By altering the **least significant bit** of each color byte, we can store secret data without changing the visible image.

| Original Byte | Binary | After Encoding ‘1’ |
|----------------|---------|-------------------|
| 11001000       | 11001000 | 11001001 |

This makes the image **visually identical** while carrying hidden data.

---

## 🧮 Encoding Process
1. **Validate Input Files**
2. **Open Required Files**
3. **Check Image Capacity**
4. **Copy BMP Header**
5. **Embed Data Sequentially**
   - Magic string  
   - Secret file extension size  
   - Secret file extension (e.g. `.txt`)  
   - Secret file size  
   - Secret file data
6. **Copy Remaining Image Data**
7. **Save Output**

---

## 🧰 Data Structures

### `EncodeInfo` (from `encode.h`)

typedef struct _EncodeInfo {
    char *src_image_fname;
    FILE *fptr_src_image;
    uint image_capacity;

    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[5];
    char secret_data[100000];
    long size_secret_file;

    char *stego_image_fname;
    FILE *fptr_stego_image;
} EncodeInfo;

### `DecodeInfo` (from `decode.h`)

typedef struct _DecodeInfo {
    char *stego_image_fname;
    FILE *fptr_stego_image;

    char *secret_fname;
    FILE *fptr_secret;
    long ext_size;
    char extn_secret_file[5];
    long size_secret_file;
} DecodeInfo;


---

## 🧾 Decoding Process
1. **Validate & Open Encoded Image**
2. **Skip BMP Header (54 bytes)**
3. **Verify Magic String**
4. **Decode Extension Size**
5. **Decode Extension Name**
6. **Decode File Size**
7. **Decode File Data**
8. **Reconstruct Secret File**

---

## 🧭 Command-Line Usage

### 🧱 Encoding
```bash
./a.out -e <source.bmp> <secret.txt> [output_image.bmp]
```

Example:
```bash
./a.out -e sample.bmp secret.txt encoded.bmp
```

### 🔍 Decoding
```bash
./a.out -d <encoded_image.bmp> [output_name]
```

Example:
```bash
./a.out -d encoded.bmp Decoded
```

---

## 💻 Sample Console Output

### ✅ Encoding
```
========================================
 🔐  Steganography using LSB Technique
========================================

🔒 Selected Encoding Operation
-> Encode arguments validated successfully.

========================================
 🔐 Starting Encoding Process
========================================
-> Step 1: Opened required files successfully.
-> Step 2: Source image has sufficient capacity.
-> Step 3: BMP header copied successfully.
-> Step 4: Magic string encoded successfully.
-> Step 5: Secret file extension size encoded successfully.
-> Step 6: Secret file extension encoded successfully.
-> Step 7: Secret file size encoded successfully.
-> Step 8: Secret file data encoded successfully.
-> Step 9: Remaining image data copied successfully.

✅ Encoding completed successfully!
📁 Output file generated: destination.bmp
========================================
```

### ✅ Decoding
```
========================================
 🔐  Steganography using LSB Technique
========================================

🔓 Selected decoding operation.
-> Decode arguments validated successfully.

========================================
 🔓 Starting Decoding Process
========================================
-> Step 1: Secret file extension size decoded successfully.
-> Step 2: Secret file extension decoded successfully.
-> Step 3: Secret file size decoded successfully.
-> Step 4: Secret file data decoded successfully.

✅ Decoding completed successfully!
📁 Output file generated: Decoded.txt
========================================
```

---

## 🧱 Compilation
```bash
gcc main.c encode.c decode.c -o stego
```

Run examples:
```bash
./stego -e sample.bmp secret.txt encoded.bmp
./stego -d encoded.bmp output
```

---

## 🚀 Future Enhancements
🔹 Add password or encryption for secure encoding  
🔹 Support PNG or JPG image formats  
🔹 Create GUI (Qt/GTK)  
🔹 Enable batch encoding of multiple files  
🔹 Add checksum for data integrity  

---

## 👨‍💻 Author
**Omkar Ashok Sawant**  
📍 *Bengaluru, Karnataka, India*  
💡 *Passionate about Embedded System.*

'''
"Code. Hide. Reveal."
'''