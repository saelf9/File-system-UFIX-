#include "disqueVirtuel.h"
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <stdio.h>
#include <sstream>
#include <cstddef>
#include <vector>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <algorithm>

using namespace std;

//vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3 {
    constexpr size_t
            DIR_ENTRY_SIZE = 28;

    int DisqueVirtuel::bd_FormatDisk() {       // Initialize the disk with 128 blocks
        m_blockDisque = std::vector<Block>(N_BLOCK_ON_DISK);

        // Creating Block objects from Block class
        for (int i = 0; i < N_BLOCK_ON_DISK; i++) {
            m_blockDisque[i] = Block();
        }

        // Initialize the bitmap of free blocks in block 2
        m_blockDisque[FREE_BLOCK_BITMAP].m_type_donnees = S_IFBL;
        m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap = std::vector<bool>(N_BLOCK_ON_DISK, true);


        // Mark all blocks from 0 to 23 as not free
        for (int i = 0; i <= 23; i++) {
            m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i] = false;
        }

        // Initialize the bitmap of free inodes in block 3

        m_blockDisque[FREE_INODE_BITMAP].m_type_donnees = S_IFIL;
        m_blockDisque[FREE_INODE_BITMAP].m_bitmap = std::vector<bool>(N_BLOCK_ON_DISK, true);

        // Block 0 is not free
        m_blockDisque[FREE_INODE_BITMAP].m_bitmap[0] = false;

        // Create one inode per block in blocks 4 to 23 
        for (int i = BASE_BLOCK_INODE; i <= 23; i++) {
            m_blockDisque[i].m_type_donnees = S_IFIN;
            m_blockDisque[i].m_inode = new iNode(i - BASE_BLOCK_INODE, 0, 0, 0, 0);
        }

        // Create inode 1 for the parent directory in block 5

        m_blockDisque[FREE_INODE_BITMAP].m_bitmap[1] = false; // mark inode 1 as used in the free inode bitmap
        std::cout << "UFS: saisir i-node " << 1 << std::endl;
        m_blockDisque[5].m_inode->st_mode = S_IFDIR;  // set file mode as a directory
        m_blockDisque[5].m_inode->st_nlink = 2;
        m_blockDisque[5].m_inode->st_size = 56; // directory size

        m_blockDisque[5].m_dirEntry.push_back(new dirEntry(1, ".")); // Add directory entry for current directory
        m_blockDisque[5].m_dirEntry.push_back(new dirEntry(1, "..")); // Add directory entry for parent directory
        std::cout << "UFS: saisir bloc " << 24 << std::endl;
        m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap.at(24) = false;  // mark block 24 as used in the free block bitmap
        log("FormatDisk", "Disk formatted successfully.");
        return 1;


    }


    Block *DisqueVirtuel::blockExist(const std::string path) {
        // Split string based on "/"
        std::vector <std::string> directories;
        char *elem = strtok(const_cast<char *>(path.c_str()), "/");
        while (elem != NULL) {
            directories.push_back(std::string(elem));
            elem = strtok(NULL, "/");
        }
        std::vector < dirEntry * > entries = m_blockDisque[5].m_dirEntry;
        // Traverse through the directory list in the given path
        int block = 5;
        for (int i = 0; i < directories.size(); i++) {

            bool found = false;
            // Traverse through the list of entries in the current directory
            for (int j = 0; j < entries.size(); j++) {
                if (entries[j]->m_filename == directories.at(i)) {
                    block = entries[j]->m_iNode + 4;
                    entries = m_blockDisque[block].m_dirEntry;
                    found = true;
                    break;
                }
            }
            if (!found)
                return 0; // If directory not found, return 0
        }

        return &m_blockDisque[block]; // Directory exists, return the block corresponding to the given path
    }


    int DisqueVirtuel::bd_mkdir(const std::string &p_DirName) {
        int lastpos = p_DirName.find_last_of("/");
        std::string path = p_DirName.substr(0, lastpos);
        std::string dirName = p_DirName.substr(lastpos + 1);

        Block *parentBlock = blockExist(path);
        if (!parentBlock) {
            cout << "Failed to create directory: Parent directory does not exist." << endl;
            return 0;
        }

        Block *existingBlock = blockExist(p_DirName);
        if (existingBlock) {
            cout << "Directory already exists." << endl;
            return 0;
        }

        size_t freeInodeIndex = findFreeInode();
        size_t freeBlockIndex = findFreeBlock();
        if (freeInodeIndex == 0 || freeBlockIndex == 0) {
            cout << "No free inode or block available." << endl;
            return 0;
        }

        cout << "UFS: saisir i-node " << freeInodeIndex << endl;
        cout << "UFS: saisir bloc " << freeBlockIndex << endl;

        Block *newBlock = &m_blockDisque[freeInodeIndex + 4];
        m_blockDisque[FREE_INODE_BITMAP].m_bitmap[freeInodeIndex] = false;
        m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[freeBlockIndex] = false;

        newBlock->m_inode = new iNode(freeInodeIndex, S_IFDIR, 2, 56, freeBlockIndex);
        newBlock->m_dirEntry.push_back(new dirEntry(newBlock->m_inode->st_ino, "."));
        newBlock->m_dirEntry.push_back(new dirEntry(parentBlock->m_inode->st_ino, ".."));

        parentBlock->m_dirEntry.push_back(new dirEntry(newBlock->m_inode->st_ino, dirName));
        parentBlock->m_inode->st_size += DIR_ENTRY_SIZE;  // Correctly increment the parent directory size
        parentBlock->m_inode->st_nlink++;  // Increment link count for the parent directory

        log("CreateDirectory", "Directory created: " + p_DirName);

        return 1;
    }


    int DisqueVirtuel::bd_create(const std::string &p_FileName) {
        int lastpos = p_FileName.find_last_of("/");
        std::string path = p_FileName.substr(0, lastpos);
        std::string filename = p_FileName.substr(lastpos + 1);

        Block *parentBlock = blockExist(path);
        if (parentBlock == nullptr) {
            std::cout << "Directory does not exist" << std::endl;
            log("CreateFile", "Failed to create file because the directory does not exist: " + path);
            return 0;
        }

        if (blockExist(p_FileName) != nullptr) {
            std::cout << "File already exists" << std::endl;
            log("CreateFile", "Failed to create file because it already exists: " + p_FileName);
            return 0;
        }

        size_t freeInodeIndex = findFreeInode();
        if (freeInodeIndex == 0) {
            std::cout << "No free inode available." << std::endl;
            return 0;
        }

        // Optional: Allocate a block immediately for the new file
        size_t freeBlockIndex = findFreeBlock();
        if (freeBlockIndex == 0) {
            std::cout << "No free block available." << std::endl;
            return 0;
        }

        std::cout << "UFS: saisir i-node " << freeInodeIndex << std::endl;
        Block *myBlock = &m_blockDisque[freeInodeIndex + 4];
        m_blockDisque[FREE_INODE_BITMAP].m_bitmap[freeInodeIndex] = false;

        // Assigning the block right at the creation if needed
        m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[freeBlockIndex] = false;

        iNode *myInode = new iNode(freeInodeIndex, S_IFREG, 1, 0, freeBlockIndex); // Regular file
        myBlock->m_inode = myInode;
        parentBlock->m_dirEntry.push_back(new dirEntry(myInode->st_ino, filename));
        parentBlock->m_inode->st_size += DIR_ENTRY_SIZE; // Increment the parent directory size

        log("CreateFile", "File created successfully: " + p_FileName);
        return 1;
    }

    std::string DisqueVirtuel::bd_ls(const std::string &p_DirLocation) {
        Block *block = blockExist(p_DirLocation);
        if (!block) {
            log("ListDirectory", "Failed to list directory contents because it does not exist: " + p_DirLocation);
            return "Directory does not exist.";
        }

        std::ostringstream val;
        val << p_DirLocation << std::endl;

        for (auto &entry: block->m_dirEntry) {
            iNode *inode = m_blockDisque[BASE_BLOCK_INODE + entry->m_iNode].m_inode;
            char dirOrFile = (inode->st_mode == S_IFDIR ? 'd' : '-');
            val << dirOrFile << " "
                << entry->m_filename << " "
                << "Size: " << inode->st_size << " "
                << "inode: " << entry->m_iNode << " "
                << "nlink: " << inode->st_nlink << std::endl;
        }

        log("ListDirectory", "Listed directory contents for: " + p_DirLocation);
        return val.str();
    }

/* Méthode utilitaire permettant de vérifier si un répertoire existe */
    int DisqueVirtuel::createEmptyDirectory(const std::string &p_DirName) {
        std::cout << "Creating empty directory: " << p_DirName << std::endl;

        // Check if the directory already exists
        if (directoryExists(p_DirName)) {
            std::cout << "Directory already exists!" << std::endl;
            return 0; // Return failure
        }

        // Find the first free inode
        size_t freeInode = findFreeInode();
        if (freeInode == -1) {
            std::cout << "No free i-node available!" << std::endl;
            return 0; // Return failure
        }

        // Find the first free data block for the directory contents
        size_t freeDataBlock = findFreeBlock();
        if (freeDataBlock == -1) {
            std::cout << "No free data block available!" << std::endl;
            return 0;
        }

        // Create an inode for the directory
        auto *dirInode = new iNode(freeInode, S_IFDIR, 2, 0, freeDataBlock);

        // Update the bitmap for the inode and data block
        m_blockDisque[FREE_INODE_BITMAP].m_bitmap[freeInode] = false;
        m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[freeDataBlock] = false;

        // Create "." and ".." entries for the directory
        m_blockDisque[freeDataBlock].m_dirEntry.push_back(new dirEntry(freeInode, "."));
        m_blockDisque[freeDataBlock].m_dirEntry.push_back(new dirEntry(ROOT_INODE, ".."));

        // Update the parent directory to include the new directory entry
        m_blockDisque[ROOT_INODE].m_dirEntry.push_back(new dirEntry(freeInode, p_DirName));

        std::cout << "Empty directory created successfully!" << std::endl;
        return 1;
    }

    int DisqueVirtuel::bd_rm(const std::string &p_FileName) {
        Block *block = blockExist(p_FileName);
        if (!block) {
            std::cout << "Path does not exist" << std::endl;
            log("RemoveItem", "Failed to remove because the path does not exist: " + p_FileName);
            return 0;
        }

        int lastPos = p_FileName.find_last_of('/');
        std::string path = (lastPos > 0) ? p_FileName.substr(0, lastPos) : "/";
        std::string itemName = p_FileName.substr(lastPos + 1);

        Block *parentBlock = blockExist(path);
        if (!parentBlock || parentBlock->m_dirEntry.empty()) {
            std::cout << "Parent directory does not exist or is empty." << std::endl;
            return 0;
        }

        auto it = std::find_if(parentBlock->m_dirEntry.begin(), parentBlock->m_dirEntry.end(),
                               [&](dirEntry *e) { return e && e->m_filename == itemName; });

        if (it == parentBlock->m_dirEntry.end()) {
            std::cout << "Item not found in directory." << std::endl;
            return 0;
        }

        // Correct handling of link count for directories
        if (block->m_inode->st_mode == S_IFDIR) {
            parentBlock->m_inode->st_nlink--; // Decrement parent directory's link count
        }

        // Removing directory entry
        delete *it; // Free memory
        parentBlock->m_dirEntry.erase(it); // Remove from vector
        parentBlock->m_inode->st_size -= DIR_ENTRY_SIZE; // Update parent size

        // Decrement the link count and release if zero
// Decrement the link count and release if zero
        if (--block->m_inode->st_nlink >= 0) {
            std::cout << "UFS: Relache i-node " << block->m_inode->st_ino << std::endl;
            m_blockDisque[FREE_INODE_BITMAP].m_bitmap[block->m_inode->st_ino] = true;

            // Only release the block if it was actually used for storing data
            if (block->m_inode->st_size > 0 && block->m_inode->st_block >= 24) {
                std::cout << "UFS: Relache bloc " << block->m_inode->st_block << std::endl;
                m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[block->m_inode->st_block] = true;
            }
        }


        log("RemoveItem", "Successfully removed: " + p_FileName);
        return 1;
    }


    size_t DisqueVirtuel::findFreeInode() {
        for (size_t idx = 1; idx < N_INODE_ON_DISK; idx++) {
            if (m_blockDisque[FREE_INODE_BITMAP].m_bitmap[idx]) {
                return idx;
            }
        }
        return 0;
    }


    DisqueVirtuel::~DisqueVirtuel() {

    }

    int DisqueVirtuel::findFreeBlock() {
        int block = 24;
        while (!m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[block]) {
            block++;
            if (block == N_BLOCK_ON_DISK) {
                return 0;
            }
        }
        return block;
    }

    DisqueVirtuel::DisqueVirtuel() {

        m_blockDisque = std::vector<Block>(N_BLOCK_ON_DISK);
    }

    void DisqueVirtuel::log(const std::string &operation, const std::string &details) {
        const std::string filename = "disqueVirtuel.log";
        std::ofstream logFile(filename, std::ios::app);
        if (!logFile) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
            return;
        }
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        std::string logMessage = ss.str() + " - [" + operation + "] " + details;
        logFile << logMessage << std::endl;
    }

    bool DisqueVirtuel::directoryExists(const std::string &path) {
        Block *block = blockExist(path);
        return (block != nullptr && block->m_inode->st_mode == S_IFDIR);
    }
}//End of namespace
