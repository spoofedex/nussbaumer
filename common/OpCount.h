/**
 * @file OpCount.h
 * @author Gerben van der Lubbe
 *
 * File to keeps track of operation counts.
 */

#ifndef OPCOUNT_H
#define OPCOUNT_H

#include <iostream>
#include <cstddef>

/**
 * Class to keep track of (ring) operation counts.
 */
class OpCount {
public:
  OpCount reset();

  void countAddition();
  void countMultiplication();
  void countConstMult();
  void countDivision();
  void countShift();
  void countBitwise();

  std::size_t getNumAdditions() const;
  std::size_t getNumMultiplications() const;
  std::size_t getNumConstMults() const;
  std::size_t getNumDivisions() const;
  std::size_t getNumShifts() const;
  std::size_t getNumBitwise() const;

public:
  std::size_t numAdditions_         = 0;       //!< Number of additions
  std::size_t numMultiplications_   = 0;       //!< Number of multiplications
  std::size_t numConstMults_        = 0;       //!< Number of constant multiplications
  std::size_t numDivisions_         = 0;       //!< Number of divisions
  std::size_t numShifts_            = 0;       //!< Number of bitwise shifts
  std::size_t numBitwise_           = 0;       //!< Number of bitwise and/or/xor
};

std::ostream& operator<<(std::ostream& oss, const OpCount& opCount);

#endif
