//
// Created by Gabriele Gaetano Fronzé on 14/03/2018.
//

#ifndef MTR_SHUTTLE_MTRCONDITIONS_H
#define MTR_SHUTTLE_MTRCONDITIONS_H

#include "RunObject.h"

class MTRConditions
{
  protected:
    typedef std::function<bool(RunObject const *)> cond_type;
    typedef std::vector<cond_type> cond_vector;

  public:
    cond_type* operator[](size_t i){ return &(fConditions[i]); }
    size_t size(){ return fConditions.size(); }
    template<class ...Args> void addCondition(bool (RunObject::*condition)(Args...) const, bool negate, Args... args){
      fConditions.emplace_back([=](RunObject const * rObj) -> bool { return (rObj->*condition)(args...) == !negate; });
    }
    
    cond_vector fConditions;
};

#endif //MTR_SHUTTLE_MTRCONDITIONS_H
