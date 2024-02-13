#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#define MAX_DEPTH 100

// Structure to hold command-line options
struct Options {
    int verbose;
    off_t fileSize;
    char *searchPattern;
    int searchDepth;
};

// Function pointer type for processing files
typedef void (*FileProcessor)(const char *, struct Options *);

// Function declarations
void processFiles(const char *dirname, FileProcessor processor, struct Options *options);
void listFiles(const char *filename, struct Options *options);
void listFilesWithDetails(const char *filename, struct Options *options);
void listLargeFiles(const char *filename, struct Options *options);
void searchFiles(const char *filename, struct Options *options);
void printFileDetails(const char *filename, struct stat *fileStat);

int main(int argc, char *argv[]) {
    struct Options options = {0, 0, NULL, 0}; // Initialize options with default values
    int opt;

    // Parse command-line options
    while ((opt = getopt(argc, argv, "vL:s:")) != -1) {
        switch (opt) {
            case 'v':
                options.verbose = 1;
                break;
            case 'L':
                options.fileSize = atoll(optarg);
                break;
            case 's':
                options.searchPattern = optarg;
                options.searchDepth = atoi(argv[optind++]);
                break;
            default:
                fprintf(stderr, "Usage: %s [-v] [-L fileSize] [-s searchPattern searchDepth] [dirname]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Set the starting directory based on command-line argument or use current directory
    const char *dirname = (optind < argc) ? argv[optind] : ".";

    // Process files based on command-line options
    if (options.searchPattern != NULL) {
        processFiles(dirname, searchFiles, &options);
    } else if (options.fileSize > 0) {
        processFiles(dirname, listLargeFiles, &options);
    } else if (options.verbose) {
        processFiles(dirname, listFilesWithDetails, &options);
    } else {
        processFiles(dirname, listFiles, &options);
    }

    return 0;
}

// Function to process files recursively in a directory
void processFiles(const char *dirname, FileProcessor processor, struct Options *options) {
    DIR *dir;
    struct dirent *entry;
    char path[1024];

    // Open directory
    if (!(dir = opendir(dirname))) {
        perror("opendir");
        return;
    }

    // Process each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

        // Ignore "." and ".." directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        processor(path, options); // Process file or directory
    }

    closedir(dir);
}

// Function to list files in a directory
void listFiles(const char *filename, struct Options *options) {
    printf("%s\n", filename);
}

// Function to list files with details (verbose mode)
void listFilesWithDetails(const char *filename, struct Options *options) {
    struct stat fileStat;

    if (lstat(filename, &fileStat) == -1) {
        perror("lstat");
        return;
    }

    printFileDetails(filename, &fileStat);
}

// Function to list large files based on file size
void listLargeFiles(const char *filename, struct Options *options) {
    struct stat fileStat;

    if (lstat(filename, &fileStat) == -1) {
        perror("lstat");
        return;
    }

    if (S_ISREG(fileStat.st_mode) && fileStat.st_size >= options->fileSize) {
        printf("%s\n", filename);
    }
}

// Function to search for files based on search pattern and depth
void searchFiles(const char *filename, struct Options *options) {
    char *ptr;
    struct stat fileStat;

    if (lstat(filename, &fileStat) == -1) {
        perror("lstat");
        return;
    }

    if (S_ISREG(fileStat.st_mode)) {
        if ((ptr = strstr(filename, options->searchPattern)) != NULL &&
            ((int)strlen(filename) - (int)strlen(ptr)) / 2 <= options->searchDepth) {
            printf("%s\n", filename);
        }
    }
}

// Function to print file details
void printFileDetails(const char *filename, struct stat *fileStat) {
    char accessTime[20];
    strftime(accessTime, 20, "%b %d %H:%M:%S %Y", localtime(&(fileStat->st_atime)));

    printf("%s (%s) - Size: %ld bytes, Permissions: %o, Last Access Time: %s\n",
           filename, S_ISLNK(fileStat->st_mode) ? "Link" : "File", fileStat->st_size,
           fileStat->st_mode & 0777, accessTime);
}
