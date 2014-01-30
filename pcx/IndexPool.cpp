#include <pcx/IndexPool.h>

#include <exception>

namespace pcx
{
      IndexPool::IndexPool(long size)
         : reserved_(size), size_(0)
         , freeListStart_(0), freeListEnd_(reserved_ - 1)
         , allocListStart_(-1), allocListEnd_(-1)
         , refList_(size), backRefList_(size)
      {
         for (long i = 0; i < size; ++i)
         {
            refList_[i] = std::make_pair(false, i + 1);
         }
         refList_[size - 1].second = -1;
      }

      long IndexPool::Allocate()
      {
         if (-1 == freeListStart_) throw std::runtime_error("Free list full");

         auto index = freeListStart_;

         if (refList_[index].first)
         {
            throw std::runtime_error("Assertion failure: index already allocated");
         }

         // take this index from the start of the 'free' list
         freeListStart_ = refList_[index].second;
         if (-1 == freeListStart_) freeListEnd_ = -1;

         auto first = -1 == allocListStart_;

         // add this index to the end of the 'alloc' list
         if (!first)
         {
            refList_[allocListEnd_].second = index;
            backRefList_[index] = allocListEnd_;
            allocListEnd_ = index;
         }
         else
         {
            allocListStart_ = allocListEnd_ = index;
            backRefList_[index] = -1;
         }

         // update this element 
         refList_[index].second = -1;
         refList_[index].first = true;

         ++size_;

         return index;
      }

      void IndexPool::Free(long index)
      {
         if (!refList_[index].first)
         {
            throw std::runtime_error("Assertion failure: index not allocated");
         }

         // remove this index from the linked list
         auto next = refList_[index].second;
         auto prev = backRefList_[index];

         refList_[index].first = false;
         refList_[index].second = freeListEnd_;

         if (prev != -1) refList_[prev].second = next;
         if (next != -1) backRefList_[next] = prev;

         if (-1 == prev)
         {
            // freeing the head of the 'alloc' list
            allocListStart_ = next;
            backRefList_[next] = -1;
         }

         if (-1 == next)
         {
            // freeing the tail of the 'alloc' list
            allocListEnd_ = prev;
            refList_[prev].second = -1;
         }

         if (freeListStart_ == -1)
         {
            freeListStart_ = freeListEnd_ = index;
         }
         else
         {
            // add to the end of the free list
            refList_[freeListEnd_].second = index;
            freeListEnd_ = index;
         }

         --size_;
      }

} // namespace pcx

