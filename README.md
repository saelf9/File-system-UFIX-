# Mini File System (UFS Implementation)

## Overview

This repository contains an implementation of a mini file system inspired by the **Unix File System (UFS)**. The objective of this project is to simulate the structure and functionality of a basic file system, similar to UFS, by implementing file management operations such as file creation, directory creation, deletion, and listing, along with metadata management.

The implementation includes:
- A **virtual disk** simulated by a vector of blocks.
- **File metadata** stored in inodes.
- Support for **files** and **directories**.
- Operations such as `ls`, `mkdir`, `rm`, and file creation (`create`).
- An **inode management system** for file metadata, and **bitmap structures** for free blocks and inodes.

The project is developed in **C++**, based on the UFS principles, and provides functionality for managing files and directories, simulating disk operations, and maintaining the integrity of a file system.

## Objectives

- Implement a **mini file system** with operations similar to UFS (Unix File System).
- Understand and implement disk operations, file system metadata management, and inode handling.
- Implement the ability to simulate basic file operations like **create**, **delete**, **list**, and **mkdir**.
- Simulate the virtual disk with **bitmaps** to track free blocks and inodes.
- Develop a basic **logging system** for the file system to track operations.

## Features

- **Virtual Disk Simulation**: Simulates a virtual disk with a block-based structure.
- **Inodes and File Metadata**: Uses inodes to store file metadata, including file name, size, and type.
- **Directory Management**: Supports creation and management of directories, including handling of `.` and `..` entries.
- **File Operations**:
  - `create`: Create new files.
  - `mkdir`: Create new directories.
  - `rm`: Remove files and directories.
  - `ls`: List the contents of a directory.
- **Bitmap Management**: Tracks free blocks and inodes using bitmaps.
- **Logging**: Logs key operations such as file creation, deletion, and inode usage.
- **Resilience**: Ensures consistency in case of crashes or unexpected failures.

## File Structure

The project is structured as follows:

```
/src
    /block.cpp          # Contains the implementation of the Block class.
    /disqueVirtuel.cpp   # Contains the implementation of the DisqueVirtuel class.
    /main.cpp            # Main entry point for running the program.
    /block.h             # Header for the Block class.
    /disqueVirtuel.h      # Header for the DisqueVirtuel class.
    /utils.cpp           # Helper functions used in file operations.
    /utils.h             # Header file for utility functions.
```

### Classes

- **Block**: Represents a block in the virtual disk, which can store metadata, bitmaps, or directory entries.
- **DisqueVirtuel**: Simulates the entire virtual disk, handling disk operations, file system management, and inode management.

### Methods

- **bd_FormatDisk**: Formats the virtual disk and initializes the file system (free block and inode bitmaps).
- **bd_create**: Creates a file in the specified directory.
- **bd_mkdir**: Creates a new directory.
- **bd_ls**: Lists the contents of a directory.
- **bd_rm**: Removes a file or directory.

## Requirements

- **C++11** or higher.
- C++ compiler (e.g., GCC, Clang).
- **CMake** for building the project.

## Building the Project

To build the project, follow these steps:

1. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/mini-filesystem.git
   cd mini-filesystem
   ```

2. Create the build directory and compile the project:

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. After building, you can run the program with:

   ```bash
   ./ufs
   ```

## Running the Project

The program will prompt you to execute commands or process command files. Here are the key commands supported:

- **format**: Format the virtual disk.
- **create <path>**: Create a new file at the specified path.
- **mkdir <path>**: Create a new directory at the specified path.
- **ls <path>**: List the contents of the directory at the specified path.
- **rm <path>**: Remove a file or directory at the specified path.

## Example Usage

### Format the Disk

```
format
```

### Create a New Directory

```
mkdir /doc
```

### List Files in Root Directory

```
ls /
```

### Remove a File

```
rm /doc/file.txt
```

## Logging

The file system tracks operations using logging. Each operation, such as formatting the disk, creating a file, or freeing blocks and inodes, is logged to the terminal for debugging purposes.

### Example Log Output

```
UFS: Saisie i-node 1
UFS: Saisie bloc 24
...
```

## Testing the Project

To test the project, the provided test scripts (Test1.txt and Test2.txt) simulate file operations that you can use to validate the functionality of your file system.

1. Run the program with the test file:

   ```bash
   ./ufs < Test1.txt
   ```

2. Check the log output to verify correctness.

## Contributions

Feel free to fork this repository and submit pull requests. If you encounter any issues or have suggestions, please create an issue in the repository.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---
