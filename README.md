# MP3 to WAV Slicing Utility

This project provides a utility to slice an MP3 file into multiple WAV segments. It uses the `minimp3` library to decode MP3 files and supports SIMD optimizations for improved performance. The program takes an MP3 file, a set of start and end times for slices, and produces WAV files for each slice.

## Features
- Decodes MP3 files to WAV format using the `minimp3` library.
- Slices the decoded audio based on specified time ranges.
- Supports SIMD optimizations (AVX) for faster processing.
- Flexible slicing functionality to support multiple segments.
- Outputs each slice as a separate WAV file.
- Customizable to handle different sample rates, bit rates, and channel configurations.

## Requirements
- **minimp3**: A minimalistic MP3 decoder library.

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/slice_mp3_to_wav.git
   cd slice_mp3_to_wav
   ```
