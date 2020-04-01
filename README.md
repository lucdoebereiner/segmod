SEGMOD (c) 2013-2020 by Luc Döbereiner and Martin Lorenz
luc.doebereiner@gmail.com

# TODO 
- [ ] Fix: Make cli tool work again
- [ ] Add all options to web UI:
  - [ ] Bit rate change
  - [ ] Download
- [ ] Fix: Add more efficient wasm->js bridge so longer tables are possible
- [ ] Make web UI pretty/usable
  - [ ] Share functionality
- [ ] Bonus: Add support for [wave-dsl](https://github.com/kfirmanty/wave-dsl)
- [ ] Bonus: Perhaps add looping, multitracking of audio



Usage: ```./segmod2 [-s samplerate] [-w waveform file/static waveform] [-o output] [-p phase offset] [-b breakpoints per cycle (0 or 1)] [-f frequencies file] [-i index format input file]```

```  
  -sr 	: sample rate. The default is 44100
  -w 	: either a file name (see below) or a static waveform number. 0 (default) is sine, 1 is triangle, 2 is pulse, 3 is sawtooth, 4 is silence.
  -f	: a file containing a sequence of frequences (see below), can be used instead of "-i" option/format.
  -i	: an index file contating a frequency table and a sequence of indices (see below), can be used instead of the "-f" option/format. 
  -b 	: breakpoints per cycle. 0 (default) two breakpoints, 1 one breakpoint.
  -p 	: if set phase will be offset by pi avoiding discontiunities with sine waveform.
  -o 	: output filename. The default is "out.wav". Always a wav file.
```  

The -f and the -i options cannot be used together.

EXAMPLES:

```$ ./segmod -i freqseq.txt -o seq.wav```

```$ ./segmod -f myfrequencies.txt -o output.wav -p -w 1 -sr 48000```

```$ ./segmod -f myfrequencies.txt -w waveformseq.txt -o output2.wav -b 1```


INPUT FILE FORMATS

Index file (same format as in segmod1)
----------

The input files consists of one line of frequency values specifying
the frequency table. The values are separated by one or more
whitespace characters.

The rest of the file is a sequence of indices into the frequency
table. Indexing starts with 1.

Lines starting with '#' are considered comments.



Frequencies file
----------------

Instead of using and index file (see above) the sequences can be
specified directly using the "-f" option. The file has to contain a
sequence of frequencies in Hz.

Lines starting with '#' are considered comments.



Waveform file
-------------

The waveform can be set to a static value or a sequence of waveforms
can be specified using the "-w" option. The text file has to contain a
series of integers (range 0 to 4), see "usage" for a description of
the available waveforms. The waveform may change whenver the frequency
changes, i.e. each cycle/half cycle (depending on the "-b" option").

Lines starting with '#' are considered comments.
