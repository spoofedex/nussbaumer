//
//  OpCount.h
//  Thesis
//
//  Created by Gerben on 28-02-16.
//  Copyright © 2016 Gerben van der Lubbe. All rights reserved.
//

#ifndef OPCOUNT_H
#define OPCOUNT_H

#include <iostream>
#include <cstddef>

class OpCount {
public:
  OpCount reset();

  void countAddition();
  void countMultiplication();
  void countDivision();
  void countShift();
  void countBitwise();

  std::size_t getNumAdditions() const;
  std::size_t getNumMultiplications() const;
  std::size_t getNumDivisions() const;
  std::size_t getNumShifts() const;
  std::size_t getNumBitwise() const;

public:
  std::size_t numAdditions_         = 0;
  std::size_t numMultiplications_   = 0;
  std::size_t numDivisions_        = 0;
  std::size_t numShifts_            = 0;
  std::size_t numBitwise_           = 0;
};

std::ostream& operator<<(std::ostream& oss, const OpCount& opCount);

#endif
