# A Minimal MP3 to WAV Slicing/Converting tool based on minimp3

This project provides a utility to slice an MP3 file into multiple WAV segments. It uses the [minimp3](https://github.com/lieff/minimp3) library, a minimalistic MP3 decoder library, to decode MP3 files and supports SIMD optimizations for improved performance.

## Features
--------

* Single dependency of [minimp3](https://github.com/lieff/minimp3) which is already included in this directory.
* Slices the decoded audio based on specified time ranges.
* Supports SIMD optimizations (AVX) for faster processing.
* Flexible slicing functionality to support multiple segments.
* Outputs each slice as a separate WAV file.

## Dependency
------------

* **minimp3**: A minimalistic MP3 decoder library. ([Included in the directory](https://github.com/lieff/minimp3))

## Installation
------------

1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/slice_mp3_to_wav.git
   cd slice_mp3_to_wav
   ```

2. Compile source file (mp3_wav.c)
    ```bash
    gcc -mavx -o mp3_to_wav mp3_wav.c -O3 -march=native -ffast-math -funroll-loops -lm 
    ```

## Usage
-----

To use the program, simply run the compiled executable with the following arguments:

* `input.mp3`: The input MP3 file to be sliced.
* `output_folder`: The folder where the sliced WAV files will be saved.
* `starts`: A comma-separated list of start times for each slice.
* `ends`: A comma-separated list of end times for each slice.

Example:

1. Download a test mp3 file
```bash
wget -O "test.mp3" "https://onlinetestcase.com/wp-content/uploads/2023/06/10-MB-MP3.mp3"
```
2. Convert 
```bash
./mp3_to_wav "test.mp3" "a" "1,2" "2,3"
// Should create 2 1 second length wav files which starts from 1 and 2 and ends at 2 ans 3 respectvly 
```

## License
-------

This project is licensed under the [MIT License](https://github.com/your-username/slice_mp3_to_wav/blob/master/LICENSE).

## Acknowledgments
--------------

This project uses the [minimp3](https://github.com/lieff/minimp3) library, a minimalistic MP3 decoder library developed and maintained by [Lieff](https://github.com/lieff).

[Original minimp3 repository](https://github.com/lieff/minimp3)
