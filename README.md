# Parallel And Serial Text Analyzer

This repository contains two implementations of a text processing tool for searching and replacing words in `.txt` files:
1. A **Serial Implementation**.
2. A **Parallel Implementation** using MPI (Message Passing Interface).

## Features

### Serial Implementation
- **Search and Replace Options:**
  - Case-sensitive and case-insensitive search.
  - Whole word-only matching.
  - Regular expression support.
- **File Handling:**
  - Processes multiple `.txt` files in the current directory.
  - Excludes the `operation_log.txt` file.
- **Logging:**
  - Logs all operations (search/replace) with timestamps and details.
- **Performance:**
  - Sequential execution suitable for small datasets or single-threaded environments.

### Parallel Implementation
- **MPI-based Parallelism:**
  - Divides `.txt` files among available MPI processes for concurrent processing.
- **Distributed Search and Replace:**
  - Supports the same search and replace features as the serial version.
- **Improved Performance:**
  - Exploits parallelism to handle large datasets efficiently.
- **Master-Slave Architecture:**
  - Master process distributes files and aggregates results, while worker processes perform the operations.
- **Scalability:**
  - Designed to work effectively on clusters or multi-core systems.

---

## Requirements

### For Serial Code
- C Compiler (GCC recommended)
- POSIX-compliant environment for regex support.

### For Parallel Code
- MPI implementation (e.g., OpenMPI or MPICH)
- C Compiler (GCC with MPI support)

---

## Usage

### Compiling the Code

#### Serial Code
  gcc -o serial.c -lm -pthread

#### Parallel Code
  mpicc -o parallel.c -lm

#### Running the Code
##### Serial Code
  ./serial

##### Parallel Code
mpirun -np <num_processes> ./parallel

## Input
The program processes all .txt files in the current directory (excluding operation_log.txt).

### User Interaction
Both versions prompt the user for:

The operation: Search or Replace.
The search word or regex pattern.
Replace word (for Replace operation).
Matching options (case sensitivity, whole word).

## Log File
Both implementations maintain an operation_log.txt file that records:

Timestamp of the operation.
Type of operation (Search or Replace).
Details of the search/replace (word, file name, count).

## Notes
Ensure all .txt files to be processed are in the same directory as the executable.
For the parallel version, the number of MPI processes should not exceed the number of .txt files.
If using regex patterns, ensure proper escaping for special characters.

## Future Enhancements
Adding GUI support for ease of use.
Extending parallelism to support GPU acceleration (e.g., with CUDA).
Enhanced logging with real-time progress tracking.
