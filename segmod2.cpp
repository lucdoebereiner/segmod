// SEGMOD (c) 2018 by Luc Döbereiner and Martin Lorenz
// luc.doebereiner@gmail.com

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <algorithm>

#ifndef EMSCRIPTEN
#include <sndfile.hh>
#endif

using namespace std; 

float freqToSampleLength (float freq, int sampleRate) {
  return (float) sampleRate / freq;
}

float freqToPhaseInc (float freq, int sampleRate) {
  return freq / (float) sampleRate;
}

int totalLength (std::vector<float> &freqs, int sampleRate) {
  float sum = 0.0;
  for (auto &f : freqs) {
    sum += freqToSampleLength(f, sampleRate);
  };
  return (int) ceil(sum);
}

float linInterp (float x, float y1, float y2) {
  return y1 + ((y2 - y1) * x);
}

float sine(float phase, float phaseOffset) {
  return sin((phase + phaseOffset) * 6.283185307);
}

float triangle(float phase, float phaseOffset) {
  float ph = fmod(phase + phaseOffset, 1.0);
  float result;
  if (ph <= 0.25) {
    result = linInterp(ph / 0.25, 0.0, 1.0);
  }  else if (ph <= 0.75) {
    result =linInterp((ph - 0.25) / 0.5, 1.0, -1.0);
  } else {
    result = linInterp((ph - 0.75) / 0.25, -1.0, 0.0);
  }
  return result;
}

float pulse(float phase, float phaseOffset) {
  float ph = phase + phaseOffset;
  if (ph < 0.5) {
    return 1.0;
  }
  else {
    return -1.0;
  }
}

bool isNumber(const std::string &s) {
  return !s.empty() && all_of(s.begin(), s.end(), ::isdigit);
}
    

float sawtooth(float phase, float phaseOffset) {
  float ph = fmod(phase + phaseOffset, 1.0);
  return linInterp(ph, 1.0, -1.0);
}

// #ifdef EMSCRIPTEN
// using sample_t = int;
// #else
// using sample_t = float;
// #endif


void synthesize(float * snd,
		unsigned int sndLength,
		vector<float> &freqs,
		vector<int> &waves,
		bool staticWave,
		bool startWave,
		float phaseOffset,
		int sampleRate,
		int breakpointPosition) {
  float freq;
  int tableIndex;
  float tableFreq;
  int wave;
  float curPhase = 0.0;
  unsigned int curFreq = 0;
  float lastPhase = 0.0;
  float curPhaseInc; 
  int curWave = startWave;
  float thisSample = 0.0;
   
  
  //printf("sndLenght %d\n",sndLength);

  curPhaseInc = freqToPhaseInc(freqs[curFreq], sampleRate);
  if (!staticWave) {
    curWave = waves[(int) curFreq % (int) waves.size()];
  };
  for (unsigned int i = 0; i < sndLength; i++) {
    if ((curPhase >= 1.0) || ((breakpointPosition == 0) && (curPhase >= 0.5) && (lastPhase < 0.5))) {
      curFreq++;
      curPhase = fmod(curPhase, 1.0);
      curPhaseInc = freqToPhaseInc(freqs[curFreq], sampleRate);
      lastPhase = curPhase;
      if (!staticWave) {
	curWave = waves[curFreq % waves.size()];
      }
    }

    switch(curWave) {
    case 0: thisSample = sine(curPhase, phaseOffset);
      break;
    case 1: thisSample = triangle(curPhase, phaseOffset);
      break;
    case 2: thisSample = pulse(curPhase, phaseOffset);
      break;
    case 3: thisSample = sawtooth(curPhase, phaseOffset);
      break;
    case 4: thisSample = 0.0;
      break;
    default:
      printf("wrong waveform argument\n");
      break;      
    };
    snd[i] = thisSample;
    curPhase += curPhaseInc;
  }

  return;
}


#ifndef EMSCRIPTEN
void gen_cli(int argc, char *argv[]) {
  float * snd;
  ifstream freqFile;
  ifstream waveFile;
  ifstream sampleDurFile;
  ifstream tableFile;
  vector<float> freqs;
  vector<float> freqTable;
  vector<int> freqIndices;
  vector<int> waves;
  SndfileHandle file;

  int sampleRate = 44100;

  float phaseOffset = 0.0;
  
  int breakpointPosition = 0;
  unsigned int sndLength;
  
  bool staticWave = true;
  bool tableInput = false;
  
  string outputFile = "out.wav";

  int curWave = 0;
  float freq;
  int wave;
  int tableIndex;
  float tableFreq;
  
  int opt;
  while ((opt = getopt(argc, argv, "s:w:o:f:p:b:i:")) != -1) {
    switch (opt) {
    case 's':
      sampleRate = atoi(optarg);
      break;
    case 'w':
      if (isNumber(optarg)) {
	staticWave = true;
	curWave = atoi(optarg);
      } else {
	waveFile.open(optarg);
	staticWave = false;
	if (!waveFile.good()) {
	  fprintf(stderr, "%s could not be opened.", optarg);
	  exit(EXIT_FAILURE);
	};
      };
      break;
    case 'f':
      freqFile.open(optarg);
      if (!freqFile.good()) {
	fprintf(stderr, "%s could not be opened.", optarg);
	exit(EXIT_FAILURE);
      };
      break;
    case 'i':
      tableFile.open(optarg);
      if (!tableFile.good()) {
	fprintf(stderr, "%s could not be opened.", optarg);
	exit(EXIT_FAILURE);
      };
      tableInput = true;
      break;
    case 'p':
      phaseOffset = atof(optarg);
      break;
    case 'b':
      breakpointPosition = atoi(optarg);
      break;
    case 'o':
      outputFile = optarg;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-s samplerate] [-w waveform file/static waveform] [-o output] [-p phase offset] [-b breakpoints per cycle (0 or 1)] [-f frequencies file] [-i index format input file]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if ((freqFile.is_open()) && (!tableInput)) {
    string line;
    while (getline(freqFile, line)) {
      int pos = line.find('#');
      if (pos != (int) string::npos) {
	line.erase(pos, -1);
      };
      istringstream iss(line);
      while (iss >> freq) {
	freqs.push_back(freq);
      }
    }
    freqFile.close();
  }

  if (waveFile.is_open()) {
    string line;
    while (getline(waveFile, line)) {
      int pos = line.find('#');
      if (pos != (int) string::npos) {
	line.erase(pos, -1);
      };
      istringstream iss(line);
      while (iss >> wave) {
	waves.push_back(wave);
      }
    }
    waveFile.close();
  }

  if (sampleDurFile.is_open()) {
    string line;
    while (getline(waveFile, line)) {
      int pos = line.find('#');
      if (pos != (int) string::npos) {
	line.erase(pos, -1);
      };
      istringstream iss(line);
      while (iss >> freq) {
	freqs.push_back(freq);
      }
    }
  }

  if (tableFile.is_open()) {
    string line;
    bool first = true;
    while (getline(tableFile, line)) {
      int pos = line.find('#');
      if (pos != (int) string::npos) {
	line.erase(pos, -1);
      };
      if (line.size() > 0) {
	istringstream iss(line);
	if (first) {
	  while (iss >> tableFreq) {
	    freqTable.push_back(tableFreq);
	  }
	  first = false;
	} else {
	  while (iss >> tableIndex) {
	    freqIndices.push_back(tableIndex);
	  }
	}
      }
    }
    waveFile.close();
  }


  if (tableInput) {
    for (auto &idx : freqIndices) {
      freqs.push_back(freqTable[idx-1]);
    }
  }
  //  int flent = static_cast<int>(freqs.size());
  //printf("freqs.length %d\n", flent);

  
  if (breakpointPosition == 0) {
    sndLength = (totalLength(freqs, sampleRate) / 2) + 1;
  } else {
    sndLength = totalLength(freqs, sampleRate);
  }
  snd = new float [sndLength];


  synthesize(snd, sndLength, freqs, waves, staticWave, curWave, phaseOffset, sampleRate, breakpointPosition);
  
  file = SndfileHandle(outputFile, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_FLOAT, 1, sampleRate) ;

  file.write(snd, sndLength);
 
}
#endif

vector<int> gen_wasm(string iFreqs, string seqs, string waveforms) {
  float * snd;
  vector<int> snd_int;
  int breakpointPosition = 0;
  int sampleRate = 44100;
  vector<float> freqs;
  vector<float> freqTable;
  vector<int> freqIndices;
  vector<int> waves;
  bool staticWave = false;
  unsigned int sndLength;
  int curWave = 0;
  float phaseOffset = 0.0;

  int wave;
  int tableIndex;
  float tableFreq;
  
  istringstream iss(iFreqs.c_str());
  while (iss >> tableFreq) {
    freqTable.push_back(tableFreq);
  }

  istringstream isr(seqs.c_str());
  while (isr >> tableIndex) {
    freqIndices.push_back(tableIndex);
  }

  istringstream isw(waveforms.c_str());
  while (isw >> wave) {
    waves.push_back(wave);
  }

  // TODO: allow for non table input (direct list of freqs)
  if (true) {
    for (auto &idx : freqIndices) {
      freqs.push_back(freqTable[idx-1]);
    }
  }
  //  int flent = static_cast<int>(freqs.size());
  //printf("freqs.length %d\n", flent);

  if (breakpointPosition == 0) {
    sndLength = (totalLength(freqs, sampleRate) / 2) + 1;
  } else {
    sndLength = totalLength(freqs, sampleRate);
  }
  
  snd = new float [sndLength];
  synthesize(snd, sndLength, freqs, waves, staticWave, curWave, phaseOffset, sampleRate, breakpointPosition);

  for (int i; i < sndLength; i++) {
    int thisSampleInt  = (((int) (snd[i] * 32768.5)) - 0.5);
    snd_int.push_back(thisSampleInt);
  }

  return snd_int;
}


int main(int argc, char *argv[]) {
#ifndef EMSCRIPTEN
  gen_cli(argc, argv);
#else
  gen_wasm("200 300 400 500 600 700", "1 1 1 1 1 2 2 2 2 2 3 3 3 3 3 4 4 4 4 4 5 5 5 5 5 6 6 6 6 6", "1 2 3");
#endif
}


#ifdef EMSCRIPTEN

#include <emscripten.h>
#include <emscripten/bind.h>
using namespace emscripten;

EMSCRIPTEN_BINDINGS(my_module) {
   register_vector<int>("vector<int>");	
   emscripten::function("gen", &gen_wasm);
}

#endif

