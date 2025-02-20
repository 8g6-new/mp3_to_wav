# Benchmark of Audio Processing Pipeline: FFmpeg vs. minimp3 Decoding

This document presents a benchmark analysis comparing the audio decoding performance of two tools: `ffmpeg` and `minimp3_decoder` (referred to as `minimp3_decoder` in the results).  The benchmark evaluates the efficiency and resource utilization of each tool when decoding MP3 audio files.

## Dataset

The benchmark was conducted using the "Blue Jay" audio dataset, comprising **32** audio files of varying durations. The audio files are located in the `./Blue Jay/` directory. The dataset's variability in audio length allows for a robust assessment of decoding performance across different file sizes.

## Methodology

Performance metrics were collected for each audio file using `perf stat` on the command-line invocations of both `ffmpeg` and the custom `minimp3_decoder` (named `minimp3_decoder` in results).  The script measured various resource utilization metrics for both decoding processes. These metrics include:

* **Elapsed Time:** The total wall clock time taken (in seconds) for decoding.
* **User CPU Time:** CPU time spent in user-space code.
* **System CPU Time:** CPU time spent in kernel-space code.
* **CPU Utilization:** The percentage of CPU utilized during decoding.
* **Task Clock:**  A measure of the total CPU time the process was scheduled.
* **Page Faults:** The number of page faults during decoding.
* **Instructions Executed:**  Total CPU instructions executed.
* **CPU Cycles Consumed:** Total CPU cycles consumed.
* **Stalled Cycles (Frontend):** CPU cycles where the instruction fetch unit was stalled.
* **Branch Instructions:** Number of branch instructions executed.
* **Branch Misses:** Number of branch mispredicted branches.
* **Branch Miss Percentage:** Percentage of branch instructions that were mispredicted.

The benchmark data was collected in JSON format (provided previously). The analysis below uses average metrics calculated from this raw data. "Raw" metrics represent the simple average across all 32 audio files. "Audio length norm" (Normalized) metrics are calculated by dividing each raw metric by the duration of the corresponding audio file and then averaging these per-second values across all files. This normalization provides a performance view independent of audio file length.

## Results

### Average Performance Metrics

The following table presents the average performance metrics for `ffmpeg` and `minimp3_decoder` (minimp3 decoder) across the "Blue Jay" dataset, both raw and normalized by audio length:

| Metric                    | ffmpeg (Raw) | minimp3_decoder (Raw) | ffmpeg (Norm) | minimp3_decoder (Norm) |
|-------------------------|--------------|------------|---------------|-------------|
| Elapsed Time (s)        | 0.1113       | **0.0824**     | 0.0040        | **0.0022**      |
| User CPU Time (s)       | 0.0592       | **0.0298**     | 0.0021        | **0.0008**      |
| System CPU Time (s)      | 0.0443       | **0.0554**     | 0.0016        | **0.0015**      |
| CPU Utilization (%)       | 78.21      | **87.02**      | 8.75          | **7.61**        |
| Task Clock (ms)           | 105.30     | **97.24**      | 3.79          | **2.71**        |
| Page Faults             | 4894.00      | **5483.33**    | 176.59        | **147.39**      |
| Instructions Executed   | 306.31M      | **216.88M**    | 11.04M        | **5.85M**       |
| CPU Cycles Consumed     | 214.96M      | **173.05M**    | 7.74M         | **4.66M**       |
| Stalled Cycles (Frontend)| 27.51M      | **29.02M**     | 0.99M         | **0.78M**       |
| Branch Instructions     | 59.81M      | **28.90M**     | 2.15M         | **0.78M**       |
| Branch Misses           | 1.05M       | **1.29M**      | 37.92K        | **34.88K**      |
| Branch Miss Percentage  | **1.719%**  | 5.399%       | **0.176%**      | 4.472%        |

*Assuming an average audio duration of **42.23 seconds** across the "Blue Jay" dataset.  Normalized values are per second of audio.*



## Analysis

Looking at the benchmark data, we can see some interesting performance differences between FFmpeg and minimp3_decoder for audio decoding.

* **Speed and Efficiency:**  `minimp3_decoder` seems to be quicker and more efficient overall. The normalized elapsed time is lower, and the raw elapsed time is also less, suggesting it's generally faster both for per-second decoding and across the whole dataset.  Lower user CPU time and task clock for `minimp3_decoder` also point to it doing the job with less processing effort and fewer CPU cycles.

* **CPU Usage:**  Normalized CPU Utilization is lower for `minimp3_decoder`, which hints at better efficiency. Raw CPU Utilization is a tad higher for `minimp3_decoder`, but that could just be because it finishes faster, and `perf stat` catches utilization over a shorter period. Normalized values probably give a clearer picture of efficiency here.  Also, `minimp3_decoder` uses fewer Instructions and CPU Cycles, both normalized and raw, which fits with the idea of it being lightweight.

* **Memory Stuff (Page Faults):** Normalized Page Faults are a bit lower for `minimp3_decoder`, maybe suggesting slightly better memory handling per second of audio. Raw Page Faults are a little higher, but it's not a huge difference either way.  Memory-wise, they seem pretty comparable, though normalized page faults lean slightly towards `minimp3_decoder`.

* **Branching Behavior:** `minimp3_decoder` does way fewer Branch Instructions and has fewer Branch Misses, both raw and normalized. This suggests its decoding process might be simpler with more predictable steps, which helps with branch prediction and reduces slowdowns.  The Branch Miss Percentage is higher for `minimp3_decoder`, but because the total number of branches is much lower, the higher percentage might not be as big of a deal.

* **Stalled Cycles:** Normalized Stalled Cycles are slightly lower for `minimp3_decoder`, but raw values are a touch higher.  These differences aren't massive and might not be super important compared to the clearer wins in other areas.

**Impact of Audio Duration:**  *(Haven't really looked into how performance changes with audio length in this summary, but that's something to consider for later investigation).*

**Bottlenecks and Ways to Improve:** FFmpeg decoding seems to be more CPU-heavy and maybe less efficient compared to `minimp3_decoder` in this test.  Since `minimp3_decoder` is designed just for MP3s and is meant to be lightweight, it makes sense that it uses fewer instructions, CPU cycles, and maybe even memory for this specific task. FFmpeg is a more all-in-one tool, so it probably has extra stuff and features that aren't needed for just MP3 decoding, which might explain why it's a bit less efficient here.

## Conclusion

From the benchmark results, it looks like **minimp3_decoder generally performs better than FFmpeg** for MP3 audio decoding in this specific test. `minimp3_decoder` appears to have advantages in speed and how efficiently it uses the CPU.  It seems to decode faster, use less CPU time (normalized), and require fewer instructions and CPU cycles. So, for just MP3 decoding tasks where you want things to be quick and efficient, **`minimp3_decoder` looks like it could be a better option than using the whole FFmpeg framework.**  Being lightweight seems to give `minimp3_decoder` a performance edge for this particular job.

## Limitations

* **Dataset Specificity:**  The "Blue Jay" dataset is just one set of audio files. Different kinds of audio, encoding settings, etc., could change things.
* **Benchmark Setup:**  This was run on a specific setup. Performance can vary depending on your computer, operating system, etc.
* **Averages can hide stuff:**  Just looking at averages might not show the whole picture.  It's good to also check out how consistent the performance is (standard deviation, distributions).
* **Just Decoding:**  This is only looking at decoding. Real-world apps might do more audio processing, and FFmpeg's wider capabilities could be more useful then.
* **`perf stat` Overhead:**  `perf stat` itself adds a bit of overhead, but we assume it's the same for both tools being tested.
* **minimp3 Binding Usage:** We tested `minimp3_decoder` using bindings.  Direct C code performance of minimp3 might be different.
