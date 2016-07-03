#include "OpCount.h"

OpCount OpCount::reset() {
  OpCount res = *this;
  *this = OpCount();
  return res;
}

void OpCount::countAddition() {
  ++numAdditions_;
}

void OpCount::countMultiplication() {
  ++numMultiplications_;
}

void OpCount::countDivision() {
  ++numDivisions_;
}

void OpCount::countShift() {
  ++numShifts_;
}

void OpCount::countBitwise() {
  ++numBitwise_;
}

std::size_t OpCount::getNumAdditions() const {
  return numAdditions_;
}

std::size_t OpCount::getNumMultiplications() const {
  return numMultiplications_;
}

std::size_t OpCount::getNumDivisions() const {
  return numDivisions_;
}

std::size_t OpCount::getNumShifts() const {
  return numShifts_;
}

std::size_t OpCount::getNumBitwise() const {
  return numBitwise_;
}

std::ostream& operator<<(std::ostream& oss, const OpCount& opCount) {
  oss << opCount.getNumAdditions() << " additions, "
      << opCount.getNumMultiplications() << " multiplications, "
      << opCount.getNumDivisions() << " divisions, "
      << opCount.getNumShifts() << " shifts, "
      << opCount.getNumBitwise() << " bitwise";
  return oss;
}
