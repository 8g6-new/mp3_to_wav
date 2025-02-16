# MP3/WAV to WAV Slicing Tool

This project provides a utility to slice MP3 or WAV audio files into multiple WAV segments based on specified time ranges. It utilizes the [minimp3](https://github.com/lieff/minimp3) library for MP3 decoding, the [libsndfile](http://www.mega-nerd.com/libsndfile/) library for WAV file handling, and employs multithreading with pthreads and asynchronous I/O for optimized performance.

## Features

* Supports both MP3 and WAV input formats.
* Uses [minimp3](https://github.com/lieff/minimp3) for MP3 decoding (included).
* Employs [libsndfile](https://github.com/libsndfile/libsndfile) for WAV file reading and writing.
* Slices audio based on provided start and end times.
* Flexible slicing for multiple segments.
* Outputs each slice as a separate WAV file.
* Configurable output format: 16-bit PCM or 32-bit floating-point.
* Multithreading with pthreads for concurrent slice processing.

## Dependencies

* **minimp3**: A minimalistic MP3 decoder library. ([Included in the repository](https://github.com/lieff/minimp3))
* **libsndfile**: A library for reading and writing audio files.  Install system-wide (e.g., `sudo apt-get install libsndfile1-dev` on Debian/Ubuntu, `brew install libsndfile` on macOS).
------------

## Installation
------------

1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/mp3_to_wav.git
   cd mp3_to_wav
   ```

2. Compile source file (mp3_wav.c)
    ```bash
    gcc -mavx -o conv main.c -O3 -march=native -ffast-math -funroll-loops -lm -lsndfile -pthread
    ```

## Usage
-----

To use the program, simply run the compiled executable with the following arguments:

* `input.mp3`: The input MP3 file to be sliced.
* `output_folder`: The file name .
* `starts`: A comma-separated list of start times for each slice.
* `ends`: A comma-separated list of end times for each slice.

Example:

1. Download a test mp3 file
```bash
wget -O "test.mp3" "https://onlinetestcase.com/wp-content/uploads/2023/06/10-MB-MP3.mp3"
```
2. Convert 
```bash
./mp3_to_wav "test.mp3" "a,b" "1,2" "2,3" 
```
Expected output

```bash
a.wav Float 32 bit WAV file written successfully.
b.wav Float 32 bit WAV file written successfully.
```

## Configuration Options

### Output Format:
By default, the tool produces 32-bit floating-point WAV files. To switch to 16-bit PCM, modify the compile-time definitions (e.g., disable MINIMP3_FLOAT_OUTPUT).

### Slicing Mode:
The code supports asynchronous multithreaded slicing (default). For sequential processing, you can replace the call to async_sliced_write_wave with sliced_write_wave in the main() function.

## Why Use libsndfile?

While a custom WAV reader was initially considered, the variety in WAV file formats made libsndfile the optimal choice for robust input handling. In contrast, a custom WAV writer was implemented to allow fine-tuned control over output performance, such as the use of asynchronous I/O and direct file structure manipulation.

# Why this project 
Initially, this project was created as a faster alternative to FFMPEG for converting MP3 to WAV, specifically to improve the preprocessing speed for generating spectrograms with my C-based spectrogram tool.

In theory, my implementation should perform better since it avoids unnecessary processing and is optimized for this specific task. However, after running benchmarks, the results were surprising FFMPEG still outperforms my implementation by 1-2% in speed. Given that my code only handles a small subset of what FFMPEG can do (without format conversion overheads, filters, or optimizations at its scale), this experience gave me even more appreciation for FFMPEG’s engineering.

That being said, this project is far from a wasted effort. A major advantage of having this custom MP3 decoder is that my spectrogram generator can now directly support MP3 files without requiring intermediate WAV conversions. This will significantly speed up processing since all MP3 data can be loaded into memory at once, eliminating the disk I/O bottleneck of repeatedly reading WAV slices.

There’s still room for optimization future improvements like better memory management, deeper profiling, and adding async I/O might help close the performance gap with FFMPEG or even surpass it in this specific use case. So, I’ll be giving this another attempt in the future.

## License
-------

This project is licensed under the [MIT License](https://github.com/your-username/slice_mp3_to_wav/blob/master/LICENSE).


## TODO
- [ ]  Asynchronous I/O: Explore further optimizations for asynchronous I/O, such as double buffering or direct I/O. Benchmark to assess the benefits.
- [ ]   More Audio Formats: Investigate adding support for other audio formats (e.g., FLAC, OGG, etc.) as input formats. This would likely involve integrating additional decoding libraries or leveraging libsndfile's capabilities further.

## Acknowledgments
--------------

This project uses the [minimp3](https://github.com/lieff/minimp3) library, a minimalistic MP3 decoder library developed and maintained by [Lieff](https://github.com/lieff).

[Original minimp3 repository](https://github.com/lieff/minimp3)
