# Dynamic r-index

This repository provides an implementation of [the dynamic r-index](https://arxiv.org/abs/2504.19482) supporting count and locate queries on an input string $T[0..n-1]$ represented as a BWT $L[0..n-1]$. 
The dynamic r-index is a dynamic version of [the r-index](https://dl.acm.org/doi/10.1145/3375890) and 
can be stored in $O(r \log n)$ bytes for the number $r$ of runs in the BWT. 
Please refer to [the r-index repository](https://github.com/nicolaprezza/r-index) for more detailed information on the r-index. 

The following table lists the main operations and update operations supported by the dynamic r-index. 

| Operation             | Time complexity                           | Description                                                         |
| --------------------- | ----------------------------------------- | ------------------------------------------------------------------- |
| build_from_BWT($L$)     | $O(n \log σ \log n)$                     | Return the dynamic r-index built from the BWT $L[0..n-1]$           |
| insert_string($i$, $P$)   | average $O((m + L_{avg}) \log σ \log n)$ | Insert a given string $P[0..m-1]$ into $T[0..n-1]$ at position $i$  |
| delete_string($i$, $m$)   | average $O((m + L_{avg}) \log σ \log n)$ | Delete substring $T[i..i+m-1]$ from $T[0..n-1]$                     |
| count_query($P$)        | $O(m \log σ \log n)$                     | Return the number of the occurrences of $P[0..m-1]$ in $T[0..n-1]$  |
| locate_query($P$)       | $O((m+occ) \log σ \log n)$               | Return the occurrences of $P[0..m-1]$ in $T[0..n-1]$                |
| backward_search($P$)    | $O(m \log σ \log n)$                     | Return the sa-interval of $P[0..m-1]$ in $T[0..n-1]$                |

> [!NOTE]  
> - $σ$ is the alphabet size of the input string $T$
> - $L_{avg}$ is the average of the LCP values in the LCP array of $T$.
> - insert_string and delete_string run in $O((m + L_{max}) \log σ \log n)$ time in the worst case for the maximum value $L_{max}$ in the LCP array.

> [!IMPORTANT]
> These time complexites are slightly larger than the time complexities descrived in the original paper. 
> This is because this implementation uses [B-trees](https://github.com/TNishimoto/b_tree_plus_alpha) for performance reasons.

## Dynamic FM-index

This repository also provides an implementation of [the dynamic FM-index](https://www.sciencedirect.com/science/article/pii/S1570866709000343) proposed by Salson et al. 
The dynamic FM-index is a dynamic version of [the FM-index](https://en.wikipedia.org/wiki/FM-index) and can be stored $O(n \log σ + (n/s) \log n)$ bytes 
for a parameter $1 \leq s \leq n$. 
Please refer to [the original dynamic FM-index repository](https://framagit.org/mikaels/dfmi) for more detailed information on the dynamic FM-index. 

The following table lists the main operations and update operations supported by the dynamic FM-index. 

| Operation             | Time complexity                               | Description                                                         |
| --------------------- | --------------------------------------------- | ------------------------------------------------------------------- |
| build_from_BWT($L$)     | $O(n \log σ \log n)$                       | Return the dynamic r-index built from the BWT $L[0..n-1]$           |
| insert_string($i$, $P$)   | average $O((m + L_{avg}) \log σ \log n)$ | Insert a given string $P[0..m-1]$ into $T[0..n-1]$ at position $i$  |
| delete_string($i$, $m$)   | average $O((m + L_{avg}) \log σ \log n)$ | Delete substring $T[i..i+m-1]$ from $T[0..n-1]$                     |
| count_query($P$)        | $O(m \log σ \log n)$                       | Return the number of the occurrences of $P[0..m-1]$ in $T[0..n-1]$  |
| locate_query($P$)       | $O((m+s・occ) \log σ \log n)$              | Return the occurrences of $P[0..m-1]$ in $T[0..n-1]$                |
| backward_search($P$)    | $O(m \log σ \log n)$                       | Return the sa-interval of $P[0..m-1]$ in $T[0..n-1]$                |

## Dependencies

- [STool](https://github.com/TNishimoto/stool)
- [B-tree_plus_alpha](https://github.com/TNishimoto/b_tree_plus_alpha) 
- [SDSL](https://github.com/simongog/sdsl-lite)

## Download

The source codes in 'modules' directory are maintained in different repositories. 
So, to download all the necessary source codes, do the following:

```
git clone https://github.com/TNishimoto/dynamic_r_index.git  
cd dynamic_r_index  
git submodule init  
git submodule update  
```

## Compile

This implementation uses [SDSL library](https://github.com/simongog/sdsl-lite). 
Assuming that the library and header files are installed in the ~/lib and ~/include directories, respectively, the source code of this repository can be compiled using the following commands:

```
mkdir build  
cd build  
cmake .. -DCMAKE_BUILD_TYPE=Release -DSDSL_LIBRARY_DIR=~/lib -DSDSL_INCLUDE_DIR=~/include  
make  
```

When the compilation is successful, the following executable files are generated: 

- build_bwt.out  
- build_r_index.out  
- build_fm_index.out
- print_index.out  
- query.out  

## Executable files

### build_bwt.out

This executable file computes and outputs the BWT of a given file.

```
Command-line options:
  -i, --input_file_path           The file path to a text (string)
  -o, --output_file_path          The path to the file where the BWT will be written (string [=])
  -c, --null_terminated_string    The special character indicating the end of text (string [=\0])
  -e, --detailed_message_flag     The value is 1 if detailed messages are printed, and 0 otherwise (unsigned int [=0])
  -?, --help                      print this message
```

> [!NOTE]  
> The null terminated string $c$ is appended to the input text as the last character.
> The input string must not contain $c$, and this character must be smallest than the characters in the input text.


```
## Example
% ./build_bwt.out -i ../examples/ab.txt -o ab$.bwt -c "$"        

=============RESULT===============
Input File:             ../examples/ab.txt
Text Length:            26
Text:                   aaaaABAaaaaABAaaaABAaaaab$
Null terminated string: $
Output File:            ab$.bwt
BWT:                    baaaBBBAAAaaaaaaaaAA$Aaaaa
Number of BWT Runs:     9
Construction Time:      0[ms] (Avg: 6884[ns/char])
Writing Time:           0[ms]
Total allocated space: 62KB
==================================

% cat ab$.bwt
> baaaBBBAAAaaaaaaaaAA$Aaaaa        
```


### build_r_index.out

This executable file builds the dynamic r-index from a given string or BWT. 

```
Command-line options:
  -i, --input_file_path           The file path to either a text or a BWT (string)
  -o, --output_file_path          The path to the file where the dynamic r-index will be written (string [=])
  -c, --null_terminated_string    The special character indicating the end of text (string [=\0])
  -u, --is_bwt                    This value is 1 if the input file is a BWT, and 0 otherwise (unsigned int [=0])
  -?, --help                      print this message
```

```
## Example
% ./build_r_index.out -i ../examples/ab.txt -o ab.dri  

=============RESULT===============
Input File:                                     ../examples/ab.txt
Output File:                                    ab.dri
The type of the input file:                     text
Statistics(DynamicRIndex):
  Text length:                                  26
  Text:                                         aaaaABAaaaaABAaaaABAaaaab/0
  Alphabet size:                                5
  Alphabet:                                     [/0, A, B, a, b]
  BWT:                                          baaaBBBAAAaaaaaaaaAA/0Aaaaa
  The number of runs in BWT:                    9
Total time:                                     0 sec (76923 ms/MB)
Total allocated space: 190KB
==================================

% ./build_bwt.out -i ../examples/ab.txt -o ab$.bwt -c "$"    
% ./build_r_index.out -i ab$.bwt -o ab$.dri -u 1 

=============RESULT===============
Input File:                                     ab$.bwt
Output File:                                    ab$.dri
The type of the input file:                     bwt
Statistics(DynamicRIndex):
  Text length:                                  26
  Text:                                         aaaaABAaaaaABAaaaABAaaaab$
  Alphabet size:                                5
  Alphabet:                                     [$, A, B, a, b]
  BWT:                                          baaaBBBAAAaaaaaaaaAA$Aaaaa
  The number of runs in BWT:                    9
Total time:                                     0 sec (76923 ms/MB)
Total allocated space: 186KB
==================================

```


### build_fm_index.out

This executable file builds the dynamic FM-index from a given string or BWT. 

```
Command-line options:
  -i, --input_file_path           The file path to either a text or a BWT (string)
  -o, --output_file_path          The path to the file where the dynamic FM-index will be written (string [=])
  -c, --null_terminated_string    The special character indicating the end of text (string [=\0])
  -u, --is_bwt                    This value is 1 if the input file is a BWT, and 0 otherwise (unsigned int [=0])
  -s, --sampling_interval         The sampling interval for the suffix array (unsigned int [=32])
  -?, --help                      print this message
```

```
## Example
% ./build_fm_index.out -i ../examples/ab.txt -o ab.dfmi  

=============RESULT===============
Input File:                                     ../examples/ab.txt
Output File:                                    ab.dfmi
The type of the input file:                     text
Statistics(DynamicFMIndex):
  Text length:                                  26
  Text:                                         aaaaABAaaaaABAaaaABAaaaab/0
  Alphabet size:                                5
  Alphabet:                                     [/0, A, B, a, b]
  BWT:                                          baaaBBBAAAaaaaaaaaAA/0Aaaaa
  Sampling interval for Sampled suffix array:   32
  The number of sampled sa-values:              2
  Average sampling interval:                    13
[END]
Total time:                                     0 sec (0 ms/MB)
Total allocated space: 591KB
==================================

% ./build_bwt.out -i ../examples/ab.txt -o ab$.bwt -c "$"    
% ./build_fm_index.out -i ab$.bwt -o ab$.dfmi -s 3 -u 1

=============RESULT===============
Input File:                                     ab$.bwt
Output File:                                    ab$.dfmi
The type of the input file:                     bwt
Statistics(DynamicFMIndex):
  Text length:                                  26
  Text:                                         aaaaABAaaaaABAaaaABAaaaab$
  Alphabet size:                                5
  Alphabet:                                     [$, A, B, a, b]
  BWT:                                          baaaBBBAAAaaaaaaaaAA$Aaaaa
  Sampling interval for Sampled suffix array:   3
  The number of sampled sa-values:              10
  Average sampling interval:                    2
[END]
Total time:                                     0 sec (76923 ms/MB)
Total allocated space: 1148KB
==================================

```

### print_index.out

This executable file shows the information about a given dynamic r-index or dynamic FM-index.

```
Command-line options:
  -i, --input_file_path     The file path to the dynamic r-index or the dynamic FM-index (string)
  -o, --output_text_path    The path to the file where the text will be written (string [=])
  -b, --output_bwt_path     The path to the file where the BWT will be written (string [=])
  -?, --help                print this message
```

```
## Example
% ./build_r_index.out -i ../examples/ab.txt -o ab$.dri -c "$"
% ./print_index.out -i ab$.dri -o ab$2.txt -b ab$2.bwt

Statistics(DynamicRIndex):
  Text length:                                  26
  Text:                                         aaaaABAaaaaABAaaaABAaaaab$
  Alphabet size:                                5
  Alphabet:                                     [$, A, B, a, b]
  BWT:                                          baaaBBBAAAaaaaaaaaAA$Aaaaa
  The number of runs in BWT:                    9

% cat ab$2.txt
> aaaaABAaaaaABAaaaABAaaaab$

% cat ab$2.bwt
> baaaBBBAAAaaaaaaaaAA$Aaaaa
```

### query.out

This executable file reads a given command file and performs commands to the loaded index (dynamic r-index or dynamic FM-index). 
The command file is a tsv format and contains one command per line. 
The following six commands are supported:

| Command    | Parameter 1 | Parameter 2 | Description                                                   |
|------------|-------------|-------------|---------------------------------------------------------------|
| INSERT     | i           | P           | Insert a given string $P$ into $T[0..n-1]$ at position $i$    |
| DELETE     | i           | m           | Delete the substring $T[i..i+m-1]$ from $T[0..n-1]$           |
| COUNT      | P           | -           | Return the number of the occurrences of $P$ in $T$            |
| LOCATE     | P           | -           | Return the occurrence positions of $P$ in $T$                 |
| LOCATE_SUM | P           | -           | Compute the sum of the occurrence positions of $P$ in $T$     |
| PRINT      | -           | -           | Print $T$ and its BWT                                         |

```
Command-line options:
  -i, --input_index_path     The file path to the dynamic r-index or the dynamic FM-index (string)
  -q, --command_file         The file path to commands (string)
  -w, --log_file             The file path to the log file (string)
  -o, --output_index_path    The path to the file where the updated index will be written (string [=])
  -?, --help                 print this message
```

```
## Example

% ./build_r_index.out -i ../examples/ab.txt -o ab$.dri -c "$"
% cat ../examples/command.tsv 
> PRINT
> COUNT   ABA
> COUNT   BBB
> LOCATE  ABA
> LOCATE  BBB
> LOCATE_SUM      ABA
> LOCATE_SUM      BBB
> INSERT  3       AAAA
> PRINT
> INSERT  5       BBBB
> PRINT
> DELETE  4       3
> PRINT
> DELETE  11      3
> PRINT

% ./query.out -i ./ab$.dri -o updated_ab$.dri -q ../examples/command.tsv -w command.log 
=============RESULT===============
Index File:                                     ./ab$.dri
Index Type:                                     Dynamic r-index
Output File:                                    updated_ab$.dri
Query File:                                     ../examples/command.tsv
Log File:                                       command.log
Checksum:                                       11
Total time:                                     0 sec
Total allocated space: 20KB
==================================

% cat command.log
> 0       PRINT   Text:   aaaaABAaaaaABAaaaABAaaaab$      BWT:    baaaBBBAAAaaaaaaaaAA$Aaaaa
> 1       COUNT   The number of occurrencces of the given pattern:        3       Time (microseconds):    9
> 2       COUNT   The number of occurrencces of the given pattern:        0       Time (microseconds):    3
> 3       LOCATE  The occurrences of the given pattern:   [11, 4, 17]     Time (microseconds):    8
> 4       LOCATE  The occurrences of the given pattern:   []      Time (microseconds):    2
> 5       LOCATE_SUM      The sum of occurrence positions of the given pattern:   32      Time (microseconds):    5
> 6       LOCATE_SUM      The sum of occurrence positions of the given pattern:   0       Time (microseconds):    2
> 7       INSERT  Reorder count:  3       Time (microseconds):    88
> 8       PRINT   Text:   aaaAAAAaABAaaaaABAaaaABAaaaab$  BWT:    baAAaaaABBBAAAaaAaaaa$aAAAaaaa
> 9       INSERT  Reorder count:  1       Time (microseconds):    56
> 10      PRINT   Text:   aaaAABBBBAAaABAaaaaABAaaaABAaaaab$      BWT:    baBaaaAABBBBAAABBAaaAaaaa$aAAAaaaa
> 11      DELETE  Reorder count:  4       Time (microseconds):    83
> 12      PRINT   Text:   aaaABBAAaABAaaaaABAaaaABAaaaab$ BWT:    bBaaaaABBBBAAAAaAaaaaaaA$AAaaaa
> 13      DELETE  Reorder count:  3       Time (microseconds):    72
> 14      PRINT   Text:   aaaABBAAaABaaABAaaaABAaaaab$    BWT:    bBaaaaABBBAAAAaaaABaaA$Aaaaa

% ./print_index.out -i updated_ab\$.dri
Statistics(DynamicRIndex):
  Text length:                                  28
  Text:                                         aaaABBAAaABaaABAaaaABAaaaab$
  Alphabet size:                                5
  Alphabet:                                     [$, A, B, a, b]
  BWT:                                          bBaaaaABBBAAAAaaaABaaA$Aaaaa
  The number of runs in BWT:                    14

```


## API Documentation (in preparation)

[Doxygen](https://TNishimoto.github.io/dynamic_r_index/html/index.html)
