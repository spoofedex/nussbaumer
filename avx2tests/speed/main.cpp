#include <iostream>
#include <iomanip>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>

#include "Karatsuba.h"
#include "avx2/Nussbaumer.h"
#include "avx2/Componentwise.h"

extern "C" {
#include "newhope/avx2/poly.h"
#include "newhope/avx2/cpucycles.h"
}

constexpr std::size_t NumTests = 1000;

std::uint16_t input1[1024] __attribute__((aligned(32)));
std::uint16_t input2[1024] __attribute__((aligned(32)));
Transformed transformed1, transformed2, result;

unsigned long long getMedian(std::vector<unsigned long long> timeDiff) {
  std::sort(timeDiff.begin(), timeDiff.end());
  std::size_t numEntries = timeDiff.size();
  if(numEntries % 2 == 1)
    return timeDiff[numEntries / 2];
  return (timeDiff[numEntries/2 - 1] + timeDiff[numEntries/2]) / 2;
}

unsigned long long getMean(const std::vector<unsigned long long> timeDiff) {
  return std::accumulate(timeDiff.begin(), timeDiff.end(), 0ull) / timeDiff.size();
}

void printResults(const std::string& section, unsigned long long* timing, std::size_t numTests) {
  std::vector<unsigned long long> timeDiff;
  for(std::size_t i = 0; i < numTests; ++i)
    timeDiff.push_back(timing[i + 1] - timing[i]);

  std::cout << section << ":" << std::endl;
  std::cout << "Median: " << getMedian(timeDiff) << std::endl
            << "Mean: " << getMean(timeDiff) << std::endl
            << std::endl;
}

int main() {
  unsigned long long timing[NumTests + 1];
  std::size_t i;
  poly p1;

  // Prepare some input (not really needed)
  for(i = 0; i < 1024; ++i)
    input1[i] = i;
  for(i = 0; i < 1024; ++i)
    input2[i] = i;

  // Run the Nussbaumer forward transform timing tests
  for(i = 0; i < NumTests + 1; ++i) {
    timing[i] = cpucycles();
    nussbaumer1024_forward(transformed1, input1);
    componentwise32_64_prepare(transformed1);
  }
  nussbaumer1024_forward(transformed2, input2);
  printResults("Nussbaumer forward transform", timing, NumTests);

  // Run the Nussbaumer inverse transform timing tests
  for(i = 0; i < NumTests + 1; ++i) {
    timing[i] = cpucycles();
    nussbaumer1024_inverse(transformed1, transformed1);
  }
  printResults("Nussbaumer inverse transform", timing, NumTests);

  // Run the componentwise core
  componentwise32_64_prepare(transformed2);
  for(i = 0; i < NumTests + 1; ++i) {
    timing[i] = cpucycles();
    componentwise32_64_run(result, transformed1, transformed2);
  }
  printResults("Pointwise multiplication", timing, NumTests);

  // Run the NTT forward transform timing tests
  for(i = 0; i < NumTests + 1; ++i) {
    timing[i] = cpucycles();
    poly_ntt(&p1);
  }
  printResults("NTT forward transform", timing, NumTests);

  // Run the NTT inverse transform timing tests
  for(i = 0; i < NumTests + 1; ++i) {
    timing[i] = cpucycles();
    poly_bitrev(&p1);
    poly_invntt(&p1);
  }
  printResults("NTT inverse transform", timing, NumTests);

  // Run the NTT pointwise multiplication timing tests
  poly r, a, b;
  for(i = 0; i < NumTests + 1; ++i) {
    timing[i] = cpucycles();
    poly_pointwise(&r, &a, &b);
  }
  printResults("NTT pointwise (unoptimized)", timing, NumTests);
}
