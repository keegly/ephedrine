#pragma once
#ifndef APU_H
#define APU_H

#include "mmu.h"

class APU {
 public:
  APU(MMU& mmu);

 private:
  MMU& mmu_;
};
#endif
