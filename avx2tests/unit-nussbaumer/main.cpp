#include <iostream>
#include <iomanip>
#include <cstdint>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "Polynomial.h"
#include "RingModElt.h"
#include "NegaNussbaumer.h"
#include "NegaConvo.h"
#include "avx2/Nussbaumer.h"
#include "avx2/Componentwise.h"

constexpr std::size_t N = 1024;
constexpr std::size_t Modulo = 2047;
static const std::size_t NumTests = 5000;

typedef RingModElt<Modulo> RingType;
static std::uint16_t data[2*N] __attribute__((aligned(32)));
static std::uint16_t data2[2*N] __attribute__((aligned(32)));
static Transformed transformed, transformed2, transformed3;
static TransformedResult result;

void runTransformTestOn(std::uint16_t* data) {
  Polynomial<RingType> input(N);
  for(std::size_t i = 0; i < N; ++i)
    input[i] = data[i];

  // Run the forward Nussbaumer transform
  nussbaumer1024_forward(transformed, data);

  // Verify the output; ignore the first line (it is irrelevant)
  NegaNussbaumer<RingType> nb(N);
  auto output = nb.transformFast(input);
  for(std::size_t i = 1; i < output.size() /* = 64 */; ++i) {
    for(std::size_t j = 0; j < output[i].getSize() /* = 32 */; ++j) {
      // Note: the output is translated
      if(RingType(transformed[i + j*output.size()]) != output[i][j]) {
        std::cout << "Transform failed at (" << j << ", " << i <<  ") for: " << input << std::endl;
        return;
      }

      // Confirm the constraints
      std::uint16_t value = transformed[i + j*output.size()];
      if(value >= (1 << 14)) {
        std::cout << "Transform has too many bits set at (" << j << ", " << i <<  ") for: " << input << std::endl;
        return;
      }
    }
  }
}

void runInverseTransformTestOn(std::uint16_t* transformed) {
  // Translate the input to the format used for the inverse transform
  std::vector<Polynomial<RingType>> input;
  for(std::size_t col = 0; col < 64; ++col) {
    Polynomial<RingType> pol(32);
    for(std::size_t row = 0; row < 32; ++row) {
      pol[row] = transformed[row*64 + col];
    }

    input.push_back(pol);
  }


  nussbaumer1024_inverse(transformed, transformed);

  NegaNussbaumer<RingType> nb(N);
  auto expectedResult = nb.inverseTransform(input);


  for(std::size_t row = 0; row < 32; ++row) {
    for(std::size_t col = 0; col < 32; ++col) {
      if((transformed[row*32 + col] - expectedResult[row*32 + col].toInt()) % 2047)
        std::cout << "Inverse transform has invalid result at " << row << ", " << col << std::endl;
      if(transformed[row*32 + col] >= 2047)
        std::cout << "Inverse transform not properly normalized " << row << ", " << col << std::endl;
    }
  }
}


void runFullTestOn(std::uint16_t* data, std::uint16_t* data2) {
  Polynomial<RingType> pol1(1024), pol2(1024);
  for(std::size_t i = 0; i < 1024; ++i) {
    pol1[i] = data[i];
    pol2[i] = data2[i];
  }

  // Perform the multiplication through the AVX2 method
  nussbaumer1024_forward(transformed, data);
  nussbaumer1024_forward(transformed2, data2);
  componentwise32_64_prepare(transformed);
  componentwise32_64_prepare(transformed2);
  componentwise32_64_run(transformed3, transformed, transformed2);
  nussbaumer1024_inverse(result, transformed3);

  // Perform the multiplication through the naive method
  auto realResult = naivemult_negacyclic(1024, pol1, pol2);
  for(std::size_t i = 0; i < 1024; ++i) {
    if((realResult[i].toInt() - result[i]) % 2047)
      std::cout << "Nussbaumer AVX2 test failed at position " << i << std::endl;
    if(result[i] >= 2047)
      std::cout << "Nussbaumer AVX2 result has too high value at position " << i << std::endl;
  }
}


void runTransformTest() {
  // Create a bunch of random data to test on
  for(std::size_t i = 0; i < N; ++i)
    data[i] = rand() & ((1 << 11) - 1);

  runTransformTestOn(data);
}


void runInverseTransformTest() {
  // Create a bunch of random data to test on
  for(std::size_t i = 0; i < 2*N; ++i)
    data[i] = rand() & ((1 << 12) - 1);

  runInverseTransformTestOn(data);
}


void runFullTest() {
  for(std::size_t i = 0; i < N; ++i) {
    data[i] = rand() & ((1 << 11) - 1);
    data2[i] = rand() & ((1 << 11) - 1);
  }

  runFullTestOn(data, data2);
}

int main() {
  srand(static_cast<unsigned>(time(NULL)));

  // Run forward tests
  for(std::size_t i = 0; i < NumTests; ++i)
    runTransformTest();
  for(std::size_t i = 0; i < NumTests; ++i)
    runInverseTransformTest();
  for(std::size_t i = 0; i < NumTests; ++i)
    runFullTest();

  // Run an extra test on the maximum numbers as input, as an extra test for overflows.
  for(std::size_t i = 0; i < N; ++i)
    data[i] = 2046;
  runTransformTestOn(data);

  for(std::size_t i = 0; i < 2*N; ++i)
    transformed[i] = (1 << 12) - 1;
  runInverseTransformTestOn(transformed);
}
