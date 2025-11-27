# Dynamic r-index

A C++ implementation of the **dynamic r-index** and **dynamic FM-index** — space-efficient data structures supporting pattern matching queries and dynamic text updates.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

## Table of Contents

- [Quick Start](#quick-start)
- [Overview](#overview)
  - [Dynamic r-index](#dynamic-r-index-1)
  - [Dynamic FM-index](#dynamic-fm-index)
  - [Which Index Should I Use?](#which-index-should-i-use)
- [Requirements](#requirements)
- [Installation](#installation)
  - [Download](#download)
  - [Install SDSL](#install-sdsl)
  - [Build](#build)
- [Usage](#usage)
  - [build_bwt.out](#build_bwtout)
  - [build_r_index.out](#build_r_indexout)
  - [build_fm_index.out](#build_fm_indexout)
  - [print_index.out](#print_indexout)
  - [query.out](#queryout)
- [API Documentation](#api-documentation)
- [License](#license)
- [References](#references)

---

## Quick Start

```bash
# 1. Clone and setup
git clone https://github.com/TNishimoto/dynamic_r_index.git
cd dynamic_r_index
git submodule update --init --recursive

# 2. Build (adjust SDSL paths as needed)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DSDSL_LIBRARY_DIR=~/local/lib \
         -DSDSL_INCLUDE_DIR=~/local/include
make

# 3. Try it out
./build_r_index.out -i ../examples/ab.txt -o ab.dri
./query.out -i ab.dri -q ../examples/command.tsv -w result.log
cat result.log
```

---

## Overview

### Dynamic r-index

This repository provides an implementation of [the dynamic r-index](https://arxiv.org/abs/2504.19482) supporting count and locate queries on an input string $T[0..n-1]$ represented as a BWT $L[0..n-1]$. 
The dynamic r-index is a dynamic version of [the r-index](https://dl.acm.org/doi/10.1145/3375890) and 
can be stored in $O(r \log n)$ bytes for the number $r$ of runs in the BWT. 
Please refer to [the r-index repository](https://github.com/nicolaprezza/r-index) for more detailed information on the r-index. 

| Operation               | Time Complexity                          | Description                                                        |
| ----------------------- | ---------------------------------------- | ------------------------------------------------------------------ |
| build_from_BWT($L$)     | $O(n \log σ \log n)$                     | Build the dynamic r-index from the BWT $L[0..n-1]$                 |
| insert_string($i$, $P$) | avg. $O((m + L_{avg}) \log σ \log n)$    | Insert string $P[0..m-1]$ into $T$ at position $i$                 |
| delete_string($i$, $m$) | avg. $O((m + L_{avg}) \log σ \log n)$    | Delete substring $T[i..i+m-1]$ from $T$                            |
| count_query($P$)        | $O(m \log σ \log n)$                     | Return the number of occurrences of $P$ in $T$                     |
| locate_query($P$)       | $O((m + occ) \log σ \log n)$             | Return all occurrence positions of $P$ in $T$                      |
| backward_search($P$)    | $O(m \log σ \log n)$                     | Return the SA-interval of $P$ in $T$                               |

> [!NOTE]  
> - $σ$: alphabet size of $T$
> - $L_{avg}$: average LCP value in the LCP array of $T$
> - Worst-case time for insert/delete: $O((m + L_{max}) \log σ \log n)$

### Dynamic FM-index

This repository also provides an implementation of [the dynamic FM-index](https://www.sciencedirect.com/science/article/pii/S1570866709000343) proposed by Salson et al. 
The dynamic FM-index is a dynamic version of [the FM-index](https://en.wikipedia.org/wiki/FM-index) and can be stored in $O(n \log σ + (n/s) \log n)$ bytes 
for a parameter $1 \leq s \leq n$. 
Please refer to [the original dynamic FM-index repository](https://framagit.org/mikaels/dfmi) for more detailed information on the dynamic FM-index. 

| Operation               | Time Complexity                          | Description                                                        |
| ----------------------- | ---------------------------------------- | ------------------------------------------------------------------ |
| build_from_BWT($L$)     | $O(n \log σ \log n)$                     | Build the dynamic FM-index from the BWT $L[0..n-1]$                |
| insert_string($i$, $P$) | avg. $O((m + L_{avg}) \log σ \log n)$    | Insert string $P[0..m-1]$ into $T$ at position $i$                 |
| delete_string($i$, $m$) | avg. $O((m + L_{avg}) \log σ \log n)$    | Delete substring $T[i..i+m-1]$ from $T$                            |
| count_query($P$)        | $O(m \log σ \log n)$                     | Return the number of occurrences of $P$ in $T$                     |
| locate_query($P$)       | $O((m + s \cdot occ) \log σ \log n)$     | Return all occurrence positions of $P$ in $T$                      |
| backward_search($P$)    | $O(m \log σ \log n)$                     | Return the SA-interval of $P$ in $T$                               |

### Which Index Should I Use?

| Feature                | Dynamic r-index         | Dynamic FM-index          |
| ---------------------- | ----------------------- | ------------------------- |
| **Space**              | $O(r \log n)$           | $O(n \log σ + (n/s) \log n)$ |
| **Locate query**       | $O((m + occ) \log σ \log n)$ | $O((m + s \cdot occ) \log σ \log n)$ |
| **Best for**           | Highly repetitive texts | General texts             |
| **Trade-off**          | Smaller for repetitive data | Adjustable via parameter $s$ |

**Recommendation:**
- Use **r-index** when your text is highly repetitive (e.g., genome collections, versioned documents)
- Use **FM-index** for general-purpose text indexing with tunable space/time trade-off

> [!IMPORTANT]
> These time complexities are slightly larger than described in the original paper due to the use of [B-trees](https://github.com/TNishimoto/b_tree_plus_alpha) for performance reasons.

---

## Requirements

- **C++ Compiler**: C++17 or later (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake**: 3.10 or later
- **Make**: GNU Make
- **SDSL Library**: [sdsl-lite](https://github.com/simongog/sdsl-lite)

---

## Installation

### Download

```bash
git clone https://github.com/TNishimoto/dynamic_r_index.git  
cd dynamic_r_index  
git submodule update --init --recursive
```

### Install SDSL

If you haven't installed SDSL yet:

```bash
git clone https://github.com/simongog/sdsl-lite.git
cd sdsl-lite
./install.sh ~/local
cd ..
```

This installs the library to `~/local/lib` and headers to `~/local/include`.

### Build

```bash
mkdir build  
cd build  
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DSDSL_LIBRARY_DIR=~/local/lib \
         -DSDSL_INCLUDE_DIR=~/local/include
make  
```

> [!TIP]
> **Alternative SDSL locations:**
> - Homebrew (Apple Silicon): `-DSDSL_LIBRARY_DIR=/opt/homebrew/lib -DSDSL_INCLUDE_DIR=/opt/homebrew/include`
> - Homebrew (Intel Mac): `-DSDSL_LIBRARY_DIR=/usr/local/lib -DSDSL_INCLUDE_DIR=/usr/local/include`

**Generated executables:**

| Executable           | Description                                      |
| -------------------- | ------------------------------------------------ |
| `build_bwt.out`      | Compute BWT from a text file                     |
| `build_r_index.out`  | Build dynamic r-index from text or BWT           |
| `build_fm_index.out` | Build dynamic FM-index from text or BWT          |
| `print_index.out`    | Display index information and extract text/BWT   |
| `query.out`          | Execute queries from a command file              |

---

## Usage

### build_bwt.out

Computes and outputs the BWT of a given file.

```
Options:
  -i, --input_file_path           Input text file path
  -o, --output_file_path          Output BWT file path
  -c, --null_terminated_string    End-of-text character (default: \0)
  -e, --detailed_message_flag     Print detailed messages (0 or 1)
  -?, --help                      Show help
```

> [!NOTE]  
> The end-of-text character is appended to the input and must be lexicographically smallest.

**Example:**

```bash
./build_bwt.out -i ../examples/ab.txt -o ab.bwt -c "$"
```

```
=============RESULT===============
Input File:             ../examples/ab.txt
Text Length:            26
Text:                   aaaaABAaaaaABAaaaABAaaaab$
BWT:                    baaaBBBAAAaaaaaaaaAA$Aaaaa
Number of BWT Runs:     9
==================================
```

---

### build_r_index.out

Builds the dynamic r-index from a text file or BWT.

```
Options:
  -i, --input_file_path           Input file path (text or BWT)
  -o, --output_file_path          Output index file path (.dri)
  -c, --null_terminated_string    End-of-text character (default: \0)
  -u, --is_bwt                    Set to 1 if input is BWT
  -?, --help                      Show help
```

**Example:**

```bash
# From text
./build_r_index.out -i ../examples/ab.txt -o ab.dri

# From BWT
./build_bwt.out -i ../examples/ab.txt -o ab.bwt -c "$"
./build_r_index.out -i ab.bwt -o ab.dri -u 1
```

---

### build_fm_index.out

Builds the dynamic FM-index from a text file or BWT.

```
Options:
  -i, --input_file_path           Input file path (text or BWT)
  -o, --output_file_path          Output index file path (.dfmi)
  -c, --null_terminated_string    End-of-text character (default: \0)
  -u, --is_bwt                    Set to 1 if input is BWT
  -s, --sampling_interval         SA sampling interval (default: 32)
  -?, --help                      Show help
```

**Example:**

```bash
# Default sampling interval (32)
./build_fm_index.out -i ../examples/ab.txt -o ab.dfmi

# Custom sampling interval
./build_fm_index.out -i ../examples/ab.txt -o ab.dfmi -s 8
```

---

### print_index.out

Displays index information and optionally extracts the text/BWT.

```
Options:
  -i, --input_file_path     Input index file path (.dri or .dfmi)
  -o, --output_text_path    Output text file path (optional)
  -b, --output_bwt_path     Output BWT file path (optional)
  -?, --help                Show help
```

**Example:**

```bash
./print_index.out -i ab.dri -o extracted.txt -b extracted.bwt
```

```
Statistics(DynamicRIndex):
  Text length:              26
  Text:                     aaaaABAaaaaABAaaaABAaaaab$
  Alphabet size:            5
  Alphabet:                 [$, A, B, a, b]
  BWT:                      baaaBBBAAAaaaaaaaaAA$Aaaaa
  The number of runs:       9
```

---

### query.out

Executes queries from a TSV command file.

```
Options:
  -i, --input_index_path    Input index file path (.dri or .dfmi)
  -q, --command_file        Command file path (TSV format)
  -w, --log_file            Output log file path
  -o, --output_index_path   Save updated index (optional)
  -?, --help                Show help
```

**Supported Commands:**

| Command      | Param 1 | Param 2 | Description                              |
| ------------ | ------- | ------- | ---------------------------------------- |
| `COUNT`      | P       | -       | Count occurrences of pattern P           |
| `LOCATE`     | P       | -       | Find all positions of pattern P          |
| `LOCATE_SUM` | P       | -       | Sum of all occurrence positions          |
| `INSERT`     | i       | P       | Insert string P at position i            |
| `DELETE`     | i       | m       | Delete m characters starting at position i |
| `PRINT`      | -       | -       | Print current text and BWT               |

**Example command file** (`command.tsv`):

```
PRINT
COUNT	ABA
LOCATE	ABA
INSERT	3	HELLO
PRINT
DELETE	3	5
PRINT
```

**Example:**

```bash
./build_r_index.out -i ../examples/ab.txt -o ab.dri -c "$"
./query.out -i ab.dri -q ../examples/command.tsv -w result.log -o updated.dri
cat result.log
```

**Sample output** (`result.log`):

```
0	PRINT	Text:	aaaaABAaaaaABAaaaABAaaaab$	BWT:	baaaBBBAAAaaaaaaaaAA$Aaaaa
1	COUNT	The number of occurrences of the given pattern:	3	Time (microseconds):	9
2	LOCATE	The occurrences of the given pattern:	[11, 4, 17]	Time (microseconds):	8
...
```

---

## API Documentation

[Doxygen Documentation](https://TNishimoto.github.io/dynamic_r_index/html/index.html) *(in preparation)*

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## References

- **Dynamic r-index**: [arXiv:2504.19482](https://arxiv.org/abs/2504.19482)
- **r-index**: [ACM DL](https://dl.acm.org/doi/10.1145/3375890)
- **Dynamic FM-index**: [ScienceDirect](https://www.sciencedirect.com/science/article/pii/S1570866709000343)
- **FM-index**: [Wikipedia](https://en.wikipedia.org/wiki/FM-index)

### Dependencies

- [STool](https://github.com/TNishimoto/stool)
- [B-tree_plus_alpha](https://github.com/TNishimoto/b_tree_plus_alpha)
- [SDSL](https://github.com/simongog/sdsl-lite)
