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

2. Compile source file (main.c)
    ```bash
    gcc -mavx -o conv main.c -O3 -DMINIMP3_FLOAT_OUTPUT -lsndfile -march=native -ffast-math -funroll-loops -fomit-frame-pointer -flto
    ```


# Audio Splitter Usage Guide


For testing purposes download a sample file ( here I choose a xeno-canto bird audio recording )

```bash
   wget -O "test.mp3" "https://xeno-canto.org/973558/download"
```

The Audio Splitter tool allows you to split audio files (MP3 or WAV) into multiple segments using three different modes.

## Mode 1: Fixed Length Segments  
Splits the audio into equal-length segments automatically.

### Usage:
```
./conv <input_file> AUTO <segment_length> ""
```
**Parameters:**  
- `<input_file>`: Path to the input audio file (MP3 or WAV)  
- `AUTO`: Keyword to indicate automatic mode  
- `<segment_length>`: Length of each segment in seconds (numeric value)  

**Example:**
```
./conv test.mp3 AUTO "2" ""
```
This will create segments of **2 seconds each**:  
- `input_1.wav` (0-2 seconds)  
- `input_2.wav` (2-4 seconds)  
- `input_3.wav` (4-6 seconds)  
- etc.  

---

## Mode 2: Auto-Named Custom Segments  
Creates segments with custom time ranges but auto-generated names.

### Usage:
```
./conv <input_file> AUTO <start_times> <end_times>
```
**Parameters:**  
- `<input_file>`: Path to the input audio file  
- `AUTO`: Keyword to indicate automatic naming  
- `<start_times>`: Comma-separated list of start times in seconds  
- `<end_times>`: Comma-separated list of end times in seconds  

**Example:**
```
./conv test.mp3 AUTO "0,5,10" "5,10,15"
```
This will create:  
- `input_1.wav` (0-5 seconds)  
- `input_2.wav` (5-10 seconds)  
- `input_3.wav` (10-15 seconds)  

---

## Mode 3: Custom Named Segments  
Creates segments with custom time ranges and custom names.

### Usage:
```
./conv <input_file> <output_names> <start_times> <end_times>
```
**Parameters:**  
- `<input_file>`: Path to the input audio file  
- `<output_names>`: Comma-separated list of output filenames (without extension)  
- `<start_times>`: Comma-separated list of start times in seconds  
- `<end_times>`: Comma-separated list of end times in seconds  

**Example:**
```
./conv test.mp3 "intro,verse,chorus" "0,15,30" "15,30,45"
```
This will create:  
- `intro.wav` (0-15 seconds)  
- `verse.wav` (15-30 seconds)  
- `chorus.wav` (30-45 seconds)  

---

## Notes:  
- **Maximum number of segments:** 400  
- **Supports both MP3 and WAV input files**  
- **All output files are in WAV format**  
- **Times should be specified in seconds**  
- **For fixed-length mode, the last segment will be truncated if it would exceed the audio length**  
- **File names are automatically appended with `.wav` extension**  


## Configuration Options

### Output Format:
By default, the tool produces **16-bit PCM WAV files**, but this can lead to distortion due to improper scaling during MP3 decoding. The **recommended format is 32-bit floating-point**, which avoids these issues and preserves audio quality.

#### Compiler Flags:
- **Default (16-bit PCM, may cause distortion):**  
  No extra flags needed.  
- **Enable 32-bit floating-point output (recommended for accuracy):**  
  ```bash
  gcc -DMINIMP3_FLOAT_OUTPUT ...
  ```

#### Why use 32-bit float?  
MP3 decoding outputs floating-point values (`[-1.0, 1.0]`), which must be scaled to **16-bit integers (`[-32768, 32767]`)**. If this scaling isn’t handled correctly, it can cause **clipping and distortion**.  

**32-bit floating-point output avoids scaling issues** and ensures the best possible audio quality. **For most cases, it is recommended to use `-DMINIMP3_FLOAT_OUTPUT`** to maintain accuracy and avoid artifacts.

### Slicing Mode:
The code supports asynchronous multithreaded slicing (default). For sequential processing, you can replace the call to async_sliced_write_wave with sliced_write_wave in the main() function.

## Why Use libsndfile?

While a custom WAV reader was initially considered, the variety in WAV file formats made libsndfile the optimal choice for robust input handling. In contrast, a custom WAV writer was implemented to allow fine-tuned control over output performance, such as the use of asynchronous I/O and direct file structure manipulation.

# Why this project 
Initially, this project was created as a faster alternative to FFMPEG for converting MP3 to WAV, specifically to improve the preprocessing speed for generating spectrograms with my C-based spectrogram tool.

In theory, my implementation should perform better since it avoids unnecessary processing and is optimized for this specific task. However, after running benchmarks, the results confirmed that my implementation is on avarge 1.8x faster [benchmark](https://github.com/8g6-new/mp3_to_wav/blob/main/benchmark/benchmark.md). This validates the approach of using a lightweight, dedicated MP3 decoder for this specific purpose. The benchmarks demonstrate that minimp3 achieves faster decoding times and lower resource utilization compared to FFmpeg for MP3-to-WAV conversion.

That being said, this project's value extends beyond just decoding speed. A key advantage of this custom MP3 decoder is that my spectrogram generator can now directly support MP3 files without requiring intermediate WAV conversions. This capability streamlines the processing pipeline and is expected to significantly speed up overall spectrogram generation by eliminating the disk I/O bottleneck associated with intermediate WAV files.

There’s still room for optimization. Future improvements like deeper profiling, and adding async I/O might further enhance the performance and resource efficiency of this MP3 decoding and slicing tool, potentially yielding even greater gains in real-world applications.
## License
-------

This project is licensed under the [MIT License](https://github.com/your-username/slice_mp3_to_wav/blob/master/LICENSE).


## TODO
- [ ] **Fix 16-bit PCM distortion:** The 16-bit output sounds off because of scaling issues. Need to properly normalize MP3’s floating-point values when converting to avoid clipping and weird artifacts.  
- [ ] **Optimize async I/O:** Look into better async I/O strategies like double buffering or direct I/O to speed things up. Need to run some benchmarks to see what actually helps.  
- [ ] **Support more audio formats:** Would be nice to add FLAC, OGG, and maybe others. Probably means integrating more decoders or using libsndfile more effectively.  


## Acknowledgments
--------------

This project uses the [minimp3](https://github.com/lieff/minimp3) library, a minimalistic MP3 decoder library developed and maintained by [Lieff](https://github.com/lieff).

[Original minimp3 repository](https://github.com/lieff/minimp3)
