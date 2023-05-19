# Multithreading Program 

This program is an exploration of multithreading using the `<pthread.h>` library, with execution time measured using the time function in the bash/zsh shell. It has been tested with different numbers of threads in both Github Codespaces and on a local machine.

## Execution Summary

The results of running the program are summarized below:

- For the local machine, the optimal number of threads is found to be 25, providing the best balance between performance and overhead. The execution time decreases sharply from 1 to 25 threads and then levels off.
  
- When running on Github Codespaces, the most efficient execution time is obtained with 4 threads, which is the recommended choice for that environment.

## Implementation Details

The `<pthread.h>` library is utilized for creating and joining threads in this program. All other libraries are included in the base code template.

To measure the execution time, the program was executed using the time function in the bash/zsh shell. The intent was to keep time tracking external from the code itself to avoid any confusion. For example:

```sh
> time ./findAngular -t 4
```

## Run Time Results

The tables below summarize the execution time for various thread numbers for both Github Codespaces and a local machine:

**Codespace Run:**

| Threads | Time Taken |
| ------- | ---------- |
| 1       | 42.290     |
| 2       | 37.680     |
| 4       | 34.476     |
| 10      | 42.018     |
| 25      | 44.853     |
| 100     | 48.061     |
| 1000    | 47.646     |

**Local Run:**

| Threads | Time Taken |
| ------- | ---------- |
| 1       | 18.944     |
| 2       | 14.444     |
| 4       | 8.806      |
| 10      | 4.088      |
| 25      | 2.789      |
| 100     | 2.583      |
| 1000    | 2.558      |

## Observations

In Github Codespaces, the time taken paradoxically increased after 4 threads were added, contrary to the initial expectations. The execution time decreased again at 1000 threads, which was unexpected. Moreover, during initial testing, I discovered unnecessary locks applied to parts of the code that could be independently accessed. These were removed in the final git push.

## Conclusions

Based on the data:

- The local machine shows a significant decrease in execution time as the number of threads increases from 1 to 25, after which the decrease becomes less significant. Therefore, 25 threads appear to provide the optimal balance between performance and overhead on the local machine.
  
- On Codespaces, the minimum execution time is obtained with 4 threads, making this the optimal number of threads for that environment.

## Screenshots

Screenshots from the codespaces terminal and my local terminal showing the time it took to run using the specified number of threads are included in the repository.



## Copyright 

Copyright © 2023. All rights reserved.

This software is provided for educational purposes only. It is prohibited to use this code, for any college assignment or personal use. Unauthorized distribution, modification or commercial usage is strictly forbidden. Please respect the rights of the author.
