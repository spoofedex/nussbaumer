#include <iostream>
#include <iomanip>
#include <cstdint>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "Polynomial.h"
#include "RingModElt.h"
#include "NegaConvo.h"
#include "avx2/Componentwise.h"
#include "avx2/Nussbaumer.h"

const std::size_t NumTests = 5000;
typedef RingModElt<2047> RingType;
Transformed data1, data2, result;

void runTestOn(std::uint16_t* data1, std::uint16_t* data2) {
  // Get the multiplications that will be executed
  std::vector<Polynomial<RingType>> pols1;
  std::vector<Polynomial<RingType>> pols2;
  for(std::size_t i = 0; i < 64; ++i) {
    Polynomial<RingType> pol1(32);
    Polynomial<RingType> pol2(32);
    for(std::size_t j = 0; j < 32; ++j) {
      pol1[j] = data1[i + j*64];
      pol2[j] = data2[i + j*64];
    }

    pols1.push_back(pol1);
    pols2.push_back(pol2);
  }

  // Prepare for componentwise transformation
  componentwise32_64_prepare(data1);
  componentwise32_64_prepare(data2);
  componentwise32_64_run(result, data1, data2);

  // Verify the results
  for(std::size_t i = 0; i < 64; ++i) {
    Polynomial<RingType> res(32);
    for(std::size_t j = 0; j < 32; ++j) {
      res[j] = result[i + j*64];

      // Verify bit constraints
      if(result[i + j*64] >= 1 << 13) {
        std::cout << "Coefficient " << i + j*64 << " of answer exceeds maximum value" << std::endl;
      }
    }

    if(naivemult_negacyclic(32, pols1[i], pols2[i]) != res) {
      std::cout << "Invalid result: (" << pols1[i] << ") * (" << pols2[i] << ") != " << res << " (mod X^32 + 1)" << std::endl;
    }
  }

  // Verify the bit count constraints
  for(std::size_t i = 0; i < 3840; ++i) {
    if(data1[i] >= (1 << 15) || data2[i] >= (1 << 15))
      std::cout << "Coefficient " << i << " of data exceeds maximum value" << std::endl;
  }
}

void runTest() {
  // Create a bunch of random data to test on
  for(std::size_t i = 0; i < 3840; ++i)
    data1[i] = rand() & 0x3FFF;
  for(std::size_t i = 0; i < 3840; ++i)
    data2[i] = rand() & 0x3FFF;

  runTestOn(data1, data2);
}

int main() {
  srand(static_cast<unsigned>(time(NULL)));
  for(std::size_t i = 0; i < NumTests; ++i)
    runTest();

  // Finally, run an extra test on the maximum numbers as input, as an extra test for overflows.
  for(std::size_t i = 0; i < 3840; ++i) {
    data1[i] = data2[i] = (1 << 14) - 1;
  }

  runTestOn(data1, data2);
}
