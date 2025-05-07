# Dynamic r-index

The dynamic r-index is a dynamic version of r-index (https://github.com/nicolaprezza/r-index). 
The size of the dynamic r-index is $O(r)$ words for the r runs in the BWT of a given string $T$. 
It supports count and locate queries in $O(m \log n)$ and $O((m+occ) \log n)$ for a given pattern of length $m$, 
where $n$ is the length of $T$, and $occ$ is the number of occurrences of the given pattern in $T$. 
In addition, we can insert a substring of length $m$ into $T$ at arbitrary positoin or delete a substring of length $m$ from $T$ in $O((m+L_{avg}) \log n)$ time in the average-case, where $L_{avg}$ is the average of the values in the LCP array of $T$.

## Download

The source codes in 'module' directory are maintained in different repositories. 
So, to download all the necessary source codes, do the following:

```
git clone https://github.com/TNishimoto/dynamic_r_index.git  
cd dynamic_r_index  
git submodule init  
git submodule update  
```

## Compile

This program uses the [SDSL library](https://github.com/simongog/sdsl-lite). Assuming that the library and header files are installed in the ~/lib and ~/include directories, respectively, the source code of this repository can be compiled using the following commands:

```
mkdir build  
cd build  
cmake .. -DCMAKE_BUILD_TYPE=Release -DSDSL_LIBRARY_DIR=~/lib -DSDSL_INCLUDE_DIR=~/include  
make  
```

## Builders and Viewers

### build_r_index.out

This executable file builds the dynamic r-index for a given file. 
The following are the command-line options: 

```
usage: ./build_r_index.out --input_file_path=string [options] ... 
options:
  -i, --input_file_path           The path of a file containing either a input text or a BWT (string)
  -o, --output_file_path          The path to the file where the dynamic r-index will be written (string [=])
  -c, --null_terminated_string    The special character indicating the end of text (string [=\0])
  -u, --is_bwt                    This value is 1 if the input file is a BWT, and 0 otherwise (unsigned int [=0])
  -?, --help                      print this message
```

> [!NOTE]  
> The null terminated string is appended to the input text as the last character.

> [!WARNING]  
> The input string must not contain the null terminated string, and the character must be smallest than the characters in the input text.

### build_fm_index.out

The executable file builds the dynamic fm-index for a given file. 
The following are the command-line options: 

```
usage: ./build_fm_index.out --input_file_path=string [options] ... 
options:
  -i, --input_file_path           The path of a file containing either a input text or a BWT (string)
  -o, --output_file_path          The path to the file where the dynamic FM-index will be written (string [=])
  -c, --null_terminated_string    The special character indicating the end of text (string [=\0])
  -u, --is_bwt                    This value is 1 if the input file is a BWT, and 0 otherwise (unsigned int [=0])
  -s, --sampling_interval         The sampling interval for the suffix array (unsigned int [=32])
  -?, --help                      print this message
```

> [!NOTE]  
> The null terminated string is appended to the input text as the last character.

> [!WARNING]  
> The input string must not contain the null terminated string, and the character must be smallest than the characters in the input text.


### build_bwt.out

This executable file builds the BWT of a given file.
The following are the command-line options: 

```
usage: ./build_bwt.out --input_file_path=string [options] ... 
options:
  -i, --input_file_path           The file path of an input text (string)
  -o, --output_file_path          The path to the file where the BWT will be written (string [=])
  -c, --null_terminated_string    The special character indicating the end of text (string [=\0])
  -e, --detailed_message_flag     The value is 1 if detailed messages are printed, and 0 otherwise (unsigned int [=0])
  -?, --help                      print this message
```

### print_index.out

This executable file shows the information about a given index.
The following are the command-line options: 

```
usage: ./print_index.out --input_file=string [options] ... 
options:
  -i, --input_file    input index name (string)
  -?, --help          print this message
```


### Examples

```
% mkdir result
% ./build_r_index.out -i ../examples/ab.txt -o result/ab.dri  

=============RESULT===============  
Input File:                                     ../examples/ab.txt  
Output File:                                    result/ab.dri  
The type of the input file:                     text  
Statistics(DynamicRIndex):  
  Text length:                                  26  
  Alphabet size:                                        5  
  The number of runs in BWT:                    9  
[END]  
Total time:                                     0 sec (76923 ms/MB)  
Total allocated space: 714KB  
==================================  
```



## Query Commands

### Print the input text and its BWT
Command: VIEW  

% cat ../examples/view_query.txt 
> VIEW 

% ./query.out -i ./result/ab.dri -q ../examples/view_query.txt -w ./result/view1.log
% cat result/view1.log 
> 0, VIEW, Text length,26, Text, aaaaABAaaaaABAaaaABAaaaab, BWT, baaaBBBAAAaaaaaaaaAAAaaaa

### Locate query
Command: LOCATE!(TABKEY)(pattern)  

% cat ../examples/locate_query.txt 
> LOCATE! ABA
> LOCATE! aaaa
> LOCATE! BBB%    



% ./query.out -i ./result/ab.dri -q ../examples/locate_query.txt -w ./result/locate1.log

% cat result/locate1.log 
> 0, LOCATE!, The execution time of backward search (microseconds), 9, The execution time of computing sa-values (microseconds), 3, Total time (microseconds),12, The number of occurrencces, 3, Checksum, 32  
> 0, Occurrences of Pattern, 11, 4, 17  
> 1, LOCATE!, The execution time of backward search (microseconds), 4, The execution time of computing sa-values (microseconds), 1, Total time (microseconds),5, The number of occurrencces, 3, Checksum, 27  
> 1, Occurrences of Pattern, 7, 0, 20  
> 2, LOCATE!, The execution time of backward search (microseconds), 2, The execution time of computing sa-values (microseconds), 0, Total time (microseconds),2, The number of occurrencces, 0, Checksum, 0  
> 2, Occurrences of Pattern,   

### Count query
Command: COUNT(TABKEY)(pattern)

% cat ../examples/count_query.txt 
> COUNT   ABA  
> COUNT   aaaa  
> COUNT   BBB  

% ./query.out -i ./result/ab.dri -q ../examples/count_query.txt -w ./result/count1.log
% cat result/count1.log

> 0, COUNT, Time (microseconds), 12, The number of occurrencces, 3  
> 1, COUNT, Time (microseconds), 4, The number of occurrencces, 3  
> 2, COUNT, Time (microseconds), 3, The number of occurrencces, 0  

### Delete query
Command: DELETE(TABKEY)(the starting position of a substring deleted from the input text)(TABKEY)(its length)

% cat ../examples/delete_query.txt
> DELETE  4       3
> DELETE  11      3

% ./query.out -i ./result/ab.dri -o ./result/ab_del.dri  -q ../examples/delete_query.txt -w ./result/delete1.log  

% ./query.out -i ./result/ab_del.dri -q ../examples/view_query.txt -w ./result/view2.log
% cat result/view2.log
> 0, VIEW, Text length,20, Text, aaaaaaaaABAABAaaaab, BWT, bBaABAAaaaaaaaAaaaa

### Insert query
Command: INSERT(TABKEY)(the insertion position of a substring inserted into the input text)(TABKEY)(the substring)

% cat ../examples/insert_query.txt
> INSERT  3       AAAA  
> INSERT  5       BBBB  

% ./query.out -i ./result/ab.dri -o ./result/ab_ins.dri  -q ../examples/insert_query.txt -w ./result/insert1.log
% ./query.out -i ./result/ab_ins.dri -q ../examples/view_query.txt -w ./result/view3.log  
% cat result/view3.log
> 0, VIEW, Text length,34, Text, aaaAABBBBAAaABAaaaaABAaaaABAaaaab, BWT, baBaaaAABBBBAAABBAaaAaaaaaAAAaaaa


## API Documentation (in preparation)

[Doxygen](https://TNishimoto.github.io/dynamic_r_index/html/index.html)
