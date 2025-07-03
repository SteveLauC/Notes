> Segment files are pre-allocated to the length configured in `Configuration::preallocate_bytes`. 
> **Preallocating files is critical for performance, as overwriting existing bytes in general is 
> less expensive than allocating new bytes on disk**.

The above paragraph comes from the documentation of the OkayWAL. The point here is if you want
to write a sequence of bytes to file, you should let the file system allocate it once, then 
overwrite the allocated/existing bytes.

# Analysis (by Gemini)

Writing without pre-allocation is slow for two critical reasons:

* High Metadata Overhead: Each file extension involves at least two distinct 
  operations: the data write and the metadata write. These metadata updates are 
  small, random I/O operations that contend with the main data writes and 
  slow everything down.

* Causes Fragmentation: If the filesystem can't find a contiguous free block, it will 
  place the new data somewhere else. Over time, your single log file becomes scattered 
  across many non-contiguous physical locations on the disk. Reading this file back 
  (e.g., during recovery) becomes extremely slow because the disk head (on an HDD) 
  has to physically seek all over the platter, or the SSD has to handle many small, 
  disjointed read commands.

# File system metrics

# Okway benchmark

## With pre-allocation (Linux, BTRFS)

commit-1KB

| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.441ms | 1.120ms | 6.549ms | 499.2us | 0.016% |
| okaywal-02t | 2.520ms | 1.122ms | 5.650ms | 394.4us | 0.024% |
| okaywal-04t | 2.817ms | 1.418ms | 7.444ms | 334.6us | 0.008% |
| okaywal-08t | 2.673ms | 1.120ms | 9.199ms | 537.7us | 0.009% |
| okaywal-16t | 2.897ms | 1.269ms | 7.120ms | 440.3us | 0.019% |

commit-1MB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 2.310ms | 1.925ms | 4.156ms | 370.2us | 0.027% |
| okaywal-02t | 2.872ms | 1.843ms | 5.484ms | 796.4us | 0.020% |
| okaywal-04t | 4.049ms | 1.749ms | 25.00ms | 2.376ms | 0.013% |
| okaywal-08t | 7.529ms | 1.812ms | 45.80ms | 5.597ms | 0.018% |
| okaywal-16t | 13.95ms | 1.808ms | 135.1ms | 13.56ms | 0.018% |

commit-256B
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.171ms | 808.4us | 5.212ms | 242.2us | 0.002% |
| okaywal-02t | 2.839ms | 2.240ms | 8.436ms | 343.8us | 0.008% |
| okaywal-04t | 2.818ms | 1.279ms | 19.48ms | 899.2us | 0.011% |
| okaywal-08t | 2.736ms | 1.402ms | 13.39ms | 594.3us | 0.009% |
| okaywal-16t | 2.780ms | 1.120ms | 9.157ms | 556.1us | 0.011% |

commit-4KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.394ms | 1.120ms | 2.889ms | 164.5us | 0.016% |
| okaywal-02t | 2.874ms | 2.378ms | 5.763ms | 347.1us | 0.020% |
| okaywal-04t | 2.866ms | 1.277ms | 6.661ms | 455.4us | 0.012% |
| okaywal-08t | 3.110ms | 1.264ms | 14.10ms | 1.346ms | 0.016% |
| okaywal-16t | 3.715ms | 1.390ms | 8.275ms | 988.4us | 0.015% |


## Without pre-allocation (Linux, BTRFS)

commit-1KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.443ms | 1.124ms | 7.326ms | 491.6us | 0.012% |
| okaywal-02t | 2.855ms | 2.241ms | 5.383ms | 286.9us | 0.020% |
| okaywal-04t | 2.732ms | 1.564ms | 7.062ms | 423.2us | 0.008% |
| okaywal-08t | 2.742ms | 1.113ms | 5.238ms | 430.7us | 0.021% |
| okaywal-16t | 2.838ms | 1.305ms | 9.468ms | 593.5us | 0.020% |

commit-1MB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 2.259ms | 1.922ms | 4.717ms | 356.3us | 0.013% |
| okaywal-02t | 2.914ms | 1.735ms | 6.735ms | 969.3us | 0.013% |
| okaywal-04t | 4.696ms | 2.117ms | 26.88ms | 3.449ms | 0.027% |
| okaywal-08t | 8.072ms | 1.751ms | 40.69ms | 5.804ms | 0.027% |
| okaywal-16t | 15.28ms | 1.838ms | 115.1ms | 15.02ms | 0.017% |

commit-256B
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.356ms | 1.095ms | 5.459ms | 209.0us | 0.006% |
| okaywal-02t | 2.815ms | 2.392ms | 6.202ms | 324.8us | 0.016% |
| okaywal-04t | 2.898ms | 1.410ms | 12.36ms | 553.3us | 0.011% |
| okaywal-08t | 2.711ms | 1.127ms | 6.626ms | 449.5us | 0.019% |
| okaywal-16t | 2.679ms | 2.096ms | 8.025ms | 500.7us | 0.014% |

commit-4KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.460ms | 1.119ms | 4.706ms | 349.8us | 0.024% |
| okaywal-02t | 2.863ms | 2.384ms | 5.486ms | 293.0us | 0.012% |
| okaywal-04t | 2.843ms | 1.286ms | 5.512ms | 340.2us | 0.018% |
| okaywal-08t | 3.053ms | 1.282ms | 16.37ms | 1.356ms | 0.008% |
| okaywal-16t | 3.654ms | 1.142ms | 9.220ms | 963.9us | 0.009% |


## Without pre-alloc (Linux, Ext4)

commit-1KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.388ms | 1.146ms | 47.32ms | 2.911ms | 0.004% |
| okaywal-02t | 2.591ms | 2.327ms | 48.68ms | 2.882ms | 0.004% |
| okaywal-04t | 2.837ms | 1.243ms | 50.76ms | 3.882ms | 0.010% |
| okaywal-08t | 3.104ms | 1.335ms | 49.81ms | 4.332ms | 0.017% |
| okaywal-16t | 3.532ms | 1.197ms | 51.33ms | 5.131ms | 0.022% |

commit-1MB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 27.51ms | 26.53ms | 48.40ms | 3.470ms | 0.027% |
| okaywal-02t | 53.36ms | 26.83ms | 103.7ms | 20.75ms | 0.000% |
| okaywal-04t | 100.6ms | 26.87ms | 196.7ms | 44.64ms | 0.000% |
| okaywal-08t | 192.9ms | 26.62ms | 273.9ms | 82.63ms | 0.000% |
| okaywal-16t | 415.0ms | 26.95ms | 2.917s  | 311.6ms | 0.013% |

commit-256B
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.361ms | 1.141ms | 53.74ms | 2.617ms | 0.006% |
| okaywal-02t | 2.514ms | 2.315ms | 49.70ms | 2.240ms | 0.005% |
| okaywal-04t | 2.487ms | 2.325ms | 49.65ms | 2.095ms | 0.002% |
| okaywal-08t | 2.639ms | 1.173ms | 55.32ms | 3.041ms | 0.005% |
| okaywal-16t | 2.958ms | 1.214ms | 54.09ms | 3.981ms | 0.013% |

commit-4KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.648ms | 1.259ms | 47.31ms | 4.100ms | 0.008% |
| okaywal-02t | 3.306ms | 1.319ms | 49.87ms | 5.392ms | 0.020% |
| okaywal-04t | 3.682ms | 1.293ms | 49.97ms | 5.569ms | 0.030% |
| okaywal-08t | 5.843ms | 1.510ms | 61.75ms | 7.133ms | 0.028% |
| okaywal-16t | 8.022ms | 1.470ms | 62.68ms | 8.951ms | 0.027% |

## With pre-alloc (Linux, Ext4)

commit-1KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.388ms | 1.157ms | 47.10ms | 2.897ms | 0.004% |
| okaywal-02t | 2.686ms | 1.202ms | 70.92ms | 4.324ms | 0.004% |
| okaywal-04t | 2.834ms | 1.176ms | 50.71ms | 3.855ms | 0.010% |
| okaywal-08t | 3.104ms | 1.205ms | 50.71ms | 4.365ms | 0.017% |
| okaywal-16t | 3.481ms | 1.219ms | 51.37ms | 4.564ms | 0.015% |

commit-1MB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 27.66ms | 26.38ms | 49.85ms | 4.263ms | 0.040% |
| okaywal-02t | 53.47ms | 26.67ms | 103.3ms | 19.12ms | 0.000% |
| okaywal-04t | 99.39ms | 26.72ms | 179.3ms | 49.46ms | 0.000% |
| okaywal-08t | 200.9ms | 26.78ms | 625.8ms | 87.40ms | 0.010% |
| okaywal-16t | 386.3ms | 27.67ms | 789.2ms | 123.0ms | 0.005% |

commit-256B
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.288ms | 1.155ms | 50.89ms | 2.221ms | 0.002% |
| okaywal-02t | 2.603ms | 2.300ms | 58.89ms | 3.070ms | 0.006% |
| okaywal-04t | 2.600ms | 1.176ms | 71.07ms | 3.321ms | 0.004% |
| okaywal-08t | 2.737ms | 1.196ms | 52.87ms | 3.441ms | 0.009% |
| okaywal-16t | 2.804ms | 1.197ms | 56.49ms | 3.234ms | 0.009% |

commit-4KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 1.650ms | 1.258ms | 47.28ms | 4.097ms | 0.008% |
| okaywal-02t | 3.301ms | 1.275ms | 49.54ms | 5.399ms | 0.020% |
| okaywal-04t | 3.659ms | 1.270ms | 50.09ms | 5.459ms | 0.030% |
| okaywal-08t | 4.198ms | 1.294ms | 50.74ms | 4.934ms | 0.016% |
| okaywal-16t | 6.484ms | 1.748ms | 46.54ms | 5.948ms | 0.044% |

## Without pre-allocation (macOS, APFS)

commit-1KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 3.875ms | 1.435ms | 7.645ms | 487.2us | 0.012% |
| okaywal-02t | 7.957ms | 4.512ms | 17.01ms | 877.1us | 0.020% |
| okaywal-04t | 7.965ms | 3.003ms | 19.80ms | 1.398ms | 0.016% |
| okaywal-08t | 7.949ms | 3.013ms | 20.89ms | 1.202ms | 0.018% |
| okaywal-16t | 8.400ms | 2.957ms | 20.83ms | 1.717ms | 0.022% |

commit-1MB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 8.409ms | 5.432ms | 14.15ms | 1.442ms | 0.013% |
| okaywal-02t | 13.97ms | 8.939ms | 19.89ms | 2.388ms | 0.000% |
| okaywal-04t | 25.55ms | 8.951ms | 276.1ms | 30.67ms | 0.027% |
| okaywal-08t | 48.57ms | 7.261ms | 1.087s  | 92.71ms | 0.020% |
| okaywal-16t | 99.49ms | 7.000ms | 1.669s  | 185.6ms | 0.027% |

commit-256B
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 4.075ms | 1.226ms | 11.93ms | 903.0us | 0.026% |
| okaywal-02t | 7.904ms | 5.565ms | 19.92ms | 859.5us | 0.014% |
| okaywal-04t | 7.789ms | 2.796ms | 12.72ms | 852.6us | 0.037% |
| okaywal-08t | 7.884ms | 2.880ms | 20.88ms | 927.5us | 0.027% |
| okaywal-16t | 7.979ms | 2.883ms | 18.15ms | 949.2us | 0.018% |

commit-4KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 3.954ms | 2.851ms | 9.238ms | 718.6us | 0.016% |
| okaywal-02t | 7.956ms | 4.980ms | 15.92ms | 1.059ms | 0.024% |
| okaywal-04t | 7.824ms | 3.049ms | 17.77ms | 1.219ms | 0.052% |
| okaywal-08t | 9.017ms | 3.929ms | 26.01ms | 2.406ms | 0.016% |
| okaywal-16t | 10.82ms | 2.613ms | 27.88ms | 2.965ms | 0.014% |

## With pre-allocation (macOS, APFS)

commit-1KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 3.950ms | 2.856ms | 7.513ms | 381.4us | 0.008% |
| okaywal-02t | 7.866ms | 3.950ms | 12.00ms | 560.3us | 0.020% |
| okaywal-04t | 7.838ms | 2.987ms | 18.93ms | 948.0us | 0.030% |
| okaywal-08t | 7.919ms | 2.966ms | 18.05ms | 1.055ms | 0.037% |
| okaywal-16t | 8.090ms | 2.869ms | 17.06ms | 1.299ms | 0.036% |

commit-1MB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 8.448ms | 5.990ms | 13.93ms | 1.278ms | 0.013% |
| okaywal-02t | 12.84ms | 7.910ms | 26.17ms | 2.255ms | 0.013% |
| okaywal-04t | 22.73ms | 7.343ms | 178.7ms | 22.48ms | 0.027% |
| okaywal-08t | 45.42ms | 5.000ms | 588.6ms | 72.29ms | 0.028% |
| okaywal-16t | 88.64ms | 7.071ms | 1.693s  | 169.4ms | 0.028% |

commit-256B
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 3.991ms | 2.825ms | 14.99ms | 686.8us | 0.010% |
| okaywal-02t | 7.952ms | 5.963ms | 15.47ms | 623.1us | 0.017% |
| okaywal-04t | 7.846ms | 2.884ms | 17.91ms | 834.2us | 0.029% |
| okaywal-08t | 7.905ms | 2.936ms | 19.14ms | 1.001ms | 0.029% |
| okaywal-16t | 8.013ms | 2.810ms | 19.13ms | 1.056ms | 0.030% |

commit-4KB
| Label       | avg     | min     | max     | stddev  | out%   |
|-------------|---------|---------|---------|---------|--------|
| okaywal-01t | 3.930ms | 2.861ms | 7.635ms | 461.7us | 0.008% |
| okaywal-02t | 7.934ms | 4.518ms | 11.97ms | 644.1us | 0.028% |
| okaywal-04t | 7.999ms | 3.985ms | 19.49ms | 1.489ms | 0.022% |
| okaywal-08t | 8.917ms | 3.091ms | 24.08ms | 2.289ms | 0.019% |
| okaywal-16t | 10.18ms | 3.024ms | 32.06ms | 3.085ms | 0.013% |