# Task 2: Multithreaded File Compression Tool

This C++ application compresses and decompresses files using the zlib library, with multithreading to improve performance on large files.

## Code Description

The program `task2.cpp` implements a command-line tool that can compress or decompress a given file. It processes the file in chunks, distributing the work across multiple threads to speed up the operation.

- **`compress_chunk()` / `decompress_chunk()`**: These functions handle the compression/decompression of individual data chunks using zlib.
- **`compress_file_multithreaded()`**: Reads the input file, splits it into chunks, and creates a separate thread to compress each chunk in parallel. It then writes the compressed chunks (with size metadata) to the output file.
- **`decompress_file_multithreaded()`**: Reads the compressed file, extracts the chunks, and creates a separate thread to decompress each one in parallel. It then reconstructs the original file from the decompressed chunks.
- **`main()`**: Parses the command-line arguments to determine the mode (compress or decompress) and the input/output filenames.

## Dependencies

This program requires the **zlib** development library.

-   **On Debian/Ubuntu:**
    ```bash
    sudo apt-get install zlib1g-dev
    ```
-   **On Fedora/CentOS:**
    ```bash
    sudo yum install zlib-devel
    ```
-   **On macOS (with Homebrew):**
    ```bash
    brew install zlib
    ```

## How to Compile and Run

1.  **Compile the code:**
    Open a terminal and use g++ to compile the program. You need to link the zlib (`-lz`) and pthread (`-pthread`) libraries.

    ```bash
    g++ task2.cpp -o compressor -std=c++11 -pthread -lz
    ```

2.  **Run the application:**

    -   **To compress a file:**
        ```bash
        ./compressor compress <input_file> <output_file>
        # Example:
        ./compressor compress large_log.txt large_log.z
        ```

    -   **To decompress a file:**
        ```bash
        ./compressor decompress <input_file> <output_file>
        # Example:
        ./compressor decompress large_log.z large_log_restored.txt
        ```
