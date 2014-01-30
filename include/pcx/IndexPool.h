#ifndef PCX_INDEX_POOL_H
#define PCX_INDEX_POOL_H

#include <vector>
#include <algorithm>
#include <numeric>
#include <unordered_map>

namespace pcx
{
   /**
    * Still experimental, but intended to be used to efficiently track indexes into
    * other collections if objects in those collections are frequrently iterated over,
    * added and removed. This class is meant to house the logic of ensuring that
    * iteration over those indicies is always in a cache-friendly (sorted) order.
    */
   class IndexPool
   {
   public:
      IndexPool(long size);

      long Allocate();
      void Free(long index);
      long Size() const { return size_; }

      long First() const { return allocListStart_; }
      long Next(long index) const { return refList_.at(index).second; }

      template <typename T>
      void ForEach(T f)
      {
         bool end = false;
         for (auto idx = allocListStart_; !end; idx = refList_[idx].second)
         {
            end = idx == allocListEnd_;
            f(idx);
         }
      }

   private:
      long  reserved_;
      long size_;

      long  freeListStart_;
      long  freeListEnd_;

      long  allocListStart_;
      long  allocListEnd_;

      std::vector<std::pair<bool, long>> refList_;
      // std::vector<long> refList_;
      std::vector<long> backRefList_;
   };

} // namespace pcx

#endif // #ifndef PCX_INDEX_POOL_H
