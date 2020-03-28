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

std::vector<int> gen(std::string iFreqs, std::string seqs) {
  float * snd;
  float phaseOffset = 0.0;
  int sampleRate = 44100;
  std::ifstream freqFile;
  std::ifstream waveFile;
  std::ifstream sampleDurFile;
  std::ifstream tableFile;
  float freq;
  std::vector<float> freqs;
  std::vector<float> freqTable;
  std::vector<int> freqIndices;
  std::vector<int> sndTwo;
  int tableIndex;
  float tableFreq;
  int wave;
  std::string outputFile = "out.wav";
  std::vector<int> waves;
  unsigned int sndLength;
  int breakpointPosition = 0;
  float curPhase = 0.0;
  unsigned int curFreq = 0;
  float lastPhase = 0.0;
  float curPhaseInc; 
  int curWave = 0;
  bool staticWave = true;
  float thisSample = 0.0;
  bool tableInput = false;
  int opt;

#ifndef EMSCRIPTEN
  SndfileHandle file;
#endif

//#ifndef EMSCRIPTEN
//  while ((opt = getopt(argc, argv, "s:w:o:f:p:b:i:")) != -1) {
//        switch (opt) {
//        case 's':
//          sampleRate = atoi(optarg);
//          break;
//        case 'w':
//          if (isNumber(optarg)) {
//            staticWave = true;
//            curWave = atoi(optarg);
//          } else {
//            waveFile.open(optarg);
//            staticWave = false;
//            if (!waveFile.good()) {
//              fprintf(stderr, "%s could not be opened.", optarg);
//              exit(EXIT_FAILURE);
//            };
//          };
//          break;
//        case 'f':
//          freqFile.open(optarg);
//          if (!freqFile.good()) {
//            fprintf(stderr, "%s could not be opened.", optarg);
//            exit(EXIT_FAILURE);
//          };
//          break;
//        case 'i':
//          tableFile.open(optarg);
//          if (!tableFile.good()) {
//            fprintf(stderr, "%s could not be opened.", optarg);
//            exit(EXIT_FAILURE);
//          };
//          tableInput = true;
//          break;
//        case 'p':
//          phaseOffset = atof(optarg);
//          break;
//        case 'b':
//          breakpointPosition = atoi(optarg);
//          break;
//        case 'o':
//          outputFile = optarg;
//          break;
//        default: /* '?' */
//          fprintf(stderr, "Usage: %s [-s samplerate] [-w waveform file/static waveform] [-o output] [-p phase offset] [-b breakpoints per cycle (0 or 1)] [-f frequencies file] [-i index format input file]\n",
//        	  argv[0]);
//          exit(EXIT_FAILURE);
//        }
//  }
//
//   if (freqFile.is_open()) {
//     while (freqFile >> freq) {
//       freqs.push_back(freq);
//     }
//     freqFile.close();
//   }
//
//  if ((freqFile.is_open()) && (!tableInput)) {
//    std::string line;
//    while (getline(freqFile, line)) {
//      int pos = line.find('#');
//      if (pos != (int) std::string::npos) {
//        line.erase(pos, -1);
//      };
//      std::istringstream iss(line);
//      while (iss >> freq) {
//        freqs.push_back(freq);
//      }
//    }
//    freqFile.close();
//  }
//
//  if (waveFile.is_open()) {
//    std::string line;
//    while (getline(waveFile, line)) {
//      int pos = line.find('#');
//      if (pos != (int) std::string::npos) {
//        line.erase(pos, -1);
//      };
//      std::istringstream iss(line);
//      while (iss >> wave) {
//        waves.push_back(wave);
//      }
//    }
//    waveFile.close();
//  }
//
//  if (sampleDurFile.is_open()) {
//    std::string line;
//    while (getline(waveFile, line)) {
//      int pos = line.find('#');
//      if (pos != (int) std::string::npos) {
//        line.erase(pos, -1);
//      };
//      std::istringstream iss(line);
//      while (iss >> freq) {
//        freqs.push_back(freq);
//      }
//    }
//  }
//
//  
//  if (tableFile.is_open()) {
//    std::string line;
//    bool first = true;
//    while (getline(tableFile, line)) {
//      int pos = line.find('#');
//      if (pos != (int) std::string::npos) {
//        line.erase(pos, -1);
//      };
//      if (line.size() > 0) {
//        std::istringstream iss(line);
//        if (first) {
//          while (iss >> tableFreq) {
//            freqTable.push_back(tableFreq);
//          }
//          first = false;
//        } else {
//          while (iss >> tableIndex) {
//            freqIndices.push_back(tableIndex);
//          }
//        }
//      }
//    }
//    waveFile.close();
//  }

//#endif


#ifdef EMSCRIPTEN
  
  std::istringstream iss(iFreqs.c_str());
  while (iss >> tableFreq) {
    freqTable.push_back(tableFreq);
  }

  std::istringstream isr(seqs.c_str());
  while (isr >> tableIndex) {
    freqIndices.push_back(tableIndex);
  }

  int lent = static_cast<int>(freqIndices.size());
  printf("freqIndices.length %d\n", lent);

#endif
  
  if (true) {
    for (auto &idx : freqIndices) {
      freqs.push_back(freqTable[idx-1]);
    }
  }
  int flent = static_cast<int>(freqs.size());
  printf("freqs.length %d\n", flent);
  
  if (breakpointPosition == 0) {
    sndLength = (totalLength(freqs, sampleRate) / 2) + 1;
  } else {
    sndLength = totalLength(freqs, sampleRate);
  }
  
  printf("sndLenght %d\n",sndLength);

  snd = new float [sndLength];

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
      printf("wrond wagfd\n");
      break;      
    };
    snd[i] = thisSample;
    int thisSampleInt  = (((int) (thisSample * 32768.5)) - 0.5);
    sndTwo.push_back(thisSampleInt);
    curPhase += curPhaseInc;
  }


#ifndef EMSCRIPTEN
  file = SndfileHandle(outputFile, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_FLOAT, 1, sampleRate) ;

  file.write(snd, sndLength);
#endif
  return sndTwo;
  //printf("snd3 %f\n", snd[3] );
  //return snd;
}

int main(int argc, char *argv[]) {
  printf("hello world\n");
  gen("200 300 400 500 600 700", "1 1 1 1 1 2 2 2 2 2 3 3 3 3 3 4 4 4 4 4 5 5 5 5 5 6 6 6 6 6");
}

#ifdef EMSCRIPTEN

#include <emscripten.h>
#include <emscripten/bind.h>
using namespace emscripten;

EMSCRIPTEN_BINDINGS(my_module) {
   register_vector<int>("vector<int>");	
   function("gen", &gen);
}

#endif

