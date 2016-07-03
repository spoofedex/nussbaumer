#include <iostream>
#include <iomanip>
#include <cstdint>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "Polynomial.h"
#include "RingModElt.h"
#include "avx2/Schoolbook.h"

const std::size_t NumTests = 100000;
typedef RingModElt<2047> RingType;
std::uint16_t data[128] __attribute__((aligned(32)));

void runTestOn(std::uint16_t* data) {
  // From the data, create the input polynomials
  std::vector<Polynomial<RingType>> pols1;
  std::vector<Polynomial<RingType>> pols2;
  for(std::size_t i = 0; i < 16; ++i) {
    Polynomial<RingType> pol1(4);
    Polynomial<RingType> pol2(4);
    for(std::size_t j = 0; j < 4; ++j)
      pol1[j] = data[i + 16*j];
    for(std::size_t j = 0; j < 4; ++j)
      pol2[j] = data[i + 16*(j + 4)];

    pols1.push_back(pol1);
    pols2.push_back(pol2);
  }

  // Perform the multiplication
  schoolbook4_test(data);

  // Get the results
  std::vector<Polynomial<RingType>> results;
  for(std::size_t i = 0; i < 16; ++i) {
    Polynomial<RingType> res(7);
    for(std::size_t j = 0; j < 7; ++j)
      res[j] = data[i + 16*j];

    results.push_back(res);
  }

  // Verify the bit count constraints
  for(std::size_t i = 0; i < 16; ++i) {
    for(std::size_t j = 0; j < 7; ++j) {
      std::uint16_t maxValue = (j == 0 || j == 7) ? 0x1FFF : 0x3FFF;
      if(data[i + 16*j] > maxValue)
        std::cout << "Coefficient " << j << " exceeds maximum value for: "
                  << pols1[i] << ") * (" << pols2[i] << ") != " << results[i] << std::endl;
    }
  }

  // Verify the polynomials
  for(std::size_t i = 0; i < 4; ++i) {
    if(pols1[i] * pols2[i] != results[i])
      std::cout << "Test failed: (" << pols1[i] << ") * (" << pols2[i] << ") != " << results[i] << std::endl;
  }
}

void runTest() {
  // Create a bunch of random data to test on
  for(std::size_t i = 0; i < 128; ++i)
    data[i] = rand() & 0x7FFF;

  runTestOn(data);
}

int main() {
  srand(static_cast<unsigned>(time(NULL)));
  for(std::size_t i = 0; i < NumTests; ++i)
    runTest();

  // Finally, run an extra test on the maximum numbers as input, as an extra test for overflows.
  for(std::size_t i = 0; i < 128; ++i)
    data[i] = 32767;

  runTestOn(data);
}
