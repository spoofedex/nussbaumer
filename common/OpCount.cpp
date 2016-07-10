#include "OpCount.h"

/**
 * Reset the operation counts to all zeroes.
 */
OpCount OpCount::reset() {
  OpCount res = *this;
  *this = OpCount();
  return res;
}

/**
 * Count an addition.
 */
void OpCount::countAddition() {
  ++numAdditions_;
}


/**
 * Count a multiplication.
 */
void OpCount::countMultiplication() {
  ++numMultiplications_;
}


/**
 * Count a constant multiplication.
 */
void OpCount::countConstMult() {
  ++numConstMults_;
}


/**
 * Count a division
 */
void OpCount::countDivision() {
  ++numDivisions_;
}


/**
 * Count a bitwise shift.
 */
void OpCount::countShift() {
  ++numShifts_;
}


/**
 * Count an AND/OR/XOR operation.
 */
void OpCount::countBitwise() {
  ++numBitwise_;
}


/**
 * Get the number of additions.
 * @return   The count.
 */
std::size_t OpCount::getNumAdditions() const {
  return numAdditions_;
}


/**
 * Get the number of multiplications.
 * @return   The count.
 */
std::size_t OpCount::getNumMultiplications() const {
  return numMultiplications_;
}


/**
 * Get the number of constant multiplications.
 * @return   The count.
 */
std::size_t OpCount::getNumConstMults() const {
  return numConstMults_;
}


/**
 * Get the number of divisions.
 * @return   The count.
 */
std::size_t OpCount::getNumDivisions() const {
  return numDivisions_;
}


/**
 * Get the number of bitwise shifts.
 * @return   The count.
 */
std::size_t OpCount::getNumShifts() const {
  return numShifts_;
}


/**
 * Get the number of AND/OR/XOR operations.
 * @return   The count.
 */
std::size_t OpCount::getNumBitwise() const {
  return numBitwise_;
}

/**
 * Write the data of an OpCount class to a stream.
 * @param[in] oss      The output stream to write to.
 * @param[in] opCount  The operation count to write.
 * @return    A reference to the stream.
 */
std::ostream& operator<<(std::ostream& oss, const OpCount& opCount) {
  oss << opCount.getNumAdditions() << " additions, "
      << opCount.getNumMultiplications() << " multiplications, "
      << opCount.getNumConstMults() << " constant multiplications, "
      << opCount.getNumDivisions() << " divisions, "
      << opCount.getNumShifts() << " shifts, "
      << opCount.getNumBitwise() << " bitwise";
  return oss;
}
