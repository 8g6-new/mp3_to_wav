# A Minimal MP3 to WAV Slicing Tool
=====================================

This project provides a utility to slice an MP3 file into multiple WAV segments. It uses the [minimp3](https://github.com/lieff/minimp3) library, a minimalistic MP3 decoder library, to decode MP3 files and supports SIMD optimizations for improved performance. The program takes an MP3 file, a set of start and end times for slices, and produces WAV files for each slice.

## Credits
--------

This project relies on the [minimp3](https://github.com/lieff/minimp3) library, which is a minimalistic MP3 decoder library. The [minimp3](https://github.com/lieff/minimp3) library is developed and maintained by [Lieff](https://github.com/lieff), and is available under the [MIT License](https://github.com/lieff/minimp3/blob/master/LICENSE).

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
```bash
./mp3_to_wav input.mp3 output_folder 0,10,20 10,20,30
```
This will slice the `input.mp3` file into three segments, starting at 0 seconds, 10 seconds, and 20 seconds, and ending at 10 seconds, 20 seconds, and 30 seconds, respectively. The sliced WAV files will be saved in the `output_folder`.

## License
-------

This project is licensed under the [MIT License](https://github.com/your-username/slice_mp3_to_wav/blob/master/LICENSE).

## Acknowledgments
--------------

This project would not have been possible without the [minimp3](https://github.com/lieff/minimp3) library, which is a minimalistic MP3 decoder library developed and maintained by [Lieff](https://github.com/lieff).

[Original minimp3 repository](https://github.com/lieff/minimp3)

[MIT License](https://github.com/lieff/minimp3/blob/master/LICENSE)