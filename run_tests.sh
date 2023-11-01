#!/usr/bin/env zsh

export ASAN_OPTIONS="halt_on_error=1"
export MSAN_OPTIONS="halt_on_error=1"
export UBSAN_OPTIONS="halt_on_error=1"
export TSAN_OPTIONS="halt_on_error=1"

output_dir=/media/night/Files/tests

# check if the parent directory exists
if [[ ! -d $output_dir:h ]]; then
  echo "Directory ${output_dir:h} does not exit" >&2
  exit 1
fi

rm -rf $output_dir
mkdir $output_dir

# test the reader for all benchmark SPEF files, and stop if one of them returns
# a non-zero value
./build/test_reader benchmark/c3_slack.spef && \
./build/test_reader benchmark/simple.spef && \
./build/test_reader benchmark/iso_example.spef && \
./build/test_reader benchmark/c17_slack.spef && \
./build/test_reader benchmark/c17.spef && \
./build/test_reader benchmark/s27.spef && \
./build/test_reader benchmark/s344.spef && \
./build/test_reader benchmark/s386.spef && \
./build/test_reader benchmark/c432.spef && \
./build/test_reader benchmark/s349.spef && \
./build/test_reader benchmark/s400.spef && \
./build/test_reader benchmark/c499.spef && \
./build/test_reader benchmark/c1355.spef && \
./build/test_reader benchmark/s526.spef && \
./build/test_reader benchmark/c1908.spef && \
./build/test_reader benchmark/s510.spef && \
./build/test_reader benchmark/c880.spef && \
./build/test_reader benchmark/c2670.spef && \
./build/test_reader benchmark/s1196.spef && \
./build/test_reader benchmark/s1494.spef && \
./build/test_reader benchmark/c3540.spef && \
./build/test_reader benchmark/c5315.spef && \
./build/test_reader benchmark/c7552_slack.spef && \
./build/test_reader benchmark/c7552.spef && \
./build/test_reader benchmark/c6288.spef && \
./build/test_reader benchmark/systemcdes.spef && \
./build/test_reader benchmark/wb_dma.spef && \
./build/test_reader benchmark/tv80.spef && \
./build/test_reader benchmark/systemcaes.spef && \
./build/test_reader benchmark/ac97_ctrl.spef && \
./build/test_reader benchmark/usb_funct.spef && \
./build/test_reader benchmark/pci_bridge32.spef && \
./build/test_reader benchmark/aes_core.spef && \
./build/test_reader benchmark/fft_ispd.spef && \
./build/test_reader benchmark/des_perf.spef && \
./build/test_reader benchmark/vga_lcd.spef

