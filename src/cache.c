//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"
#include <math.h>   // for log2()
#include <stdio.h>  // for printf();
#include <stdint.h>

//
// TODO:Student Information
//
const char *studentName = "Sha Liu";
const char *studentID   = "A53239717";
const char *email       = "shl237@ucsd.edu";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

uint32_t offsetSize;

uint32_t iIndexSize;
// uint32_t iIndexMask;
uint32_t iTagSize;
// uint32_t iTagMask;


uint32_t dIndexSize;
// uint32_t dIndexMask;
uint32_t dTagSize;
// uint32_t dTagMask;

uint32_t l2IndexSize;
// uint32_t l2IndexMask;
uint32_t l2TagSize;
// uint32_t l2TagMask;

char IorD;

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//

typedef struct Block {
  uint32_t tag;
  uint32_t timeStamp;
  uint32_t isValid;
} Block;

typedef struct Set {
  Block* blockList;
} Set;

typedef struct Cache {
  Set* setList;
} Cache;

Cache iCache;
Cache dCache;
Cache l2Cache;
//------------------------------------//
//          Cache Functions           //
//------------------------------------//

int is_miss(uint32_t addr);
int lru_add(uint32_t addr);
void lru_evict(uint32_t addr, int evictBlock);
// Initialize the Cache Hierarchy
//

uint32_t 
calc_log2(uint32_t x) 
{
  if (x <= 1) { return 0; }
  return (uint32_t)log2(x);
}

int
is_miss(uint32_t addr)
{
  if (IorD == 'I')
  {
    int tag = addr >> (ADDR_SIZE - iTagSize);
    int index = (addr << iTagSize) >> (ADDR_SIZE - iIndexSize);

    for (int k = 0; k < icacheAssoc; k++) 
    {
      if (iCache.setList[index].blockList[k].isValid == TRUE && 
          iCache.setList[index].blockList[k].tag == tag) {
        // hit
        iCache.setList[index].blockList[k].timeStamp = icacheRefs;
        return FALSE;
      }
    }
    return TRUE;
  }
  else if (IorD == 'D')
  {
    int tag = addr >> (ADDR_SIZE - dTagSize);
    int index = (addr << dTagSize) >> (ADDR_SIZE - dIndexSize);

    for (int k = 0; k < dcacheAssoc; k++) 
    {
      if (dCache.setList[index].blockList[k].isValid == TRUE && 
          dCache.setList[index].blockList[k].tag == tag) {
        // hit
        dCache.setList[index].blockList[k].timeStamp = dcacheRefs;
        return FALSE;
      }
    }
    return TRUE;
  }
  else 
  {
    int tag = addr >> (ADDR_SIZE - l2TagSize);
    int index = (addr << l2TagSize) >> (ADDR_SIZE - l2IndexSize);

    for (int k = 0; k < l2cacheAssoc; k++) 
    {
      if (l2Cache.setList[index].blockList[k].isValid == TRUE && 
          l2Cache.setList[index].blockList[k].tag == tag) {
        // hit
        l2Cache.setList[index].blockList[k].timeStamp = l2cacheRefs;
        return FALSE;
      }
    }
    return TRUE;
  }
}

int
lru_add(uint32_t addr) {
  if (IorD == 'I')
  {
    int tag = addr >> (ADDR_SIZE - iTagSize);
    int index = (addr << iTagSize) >> (ADDR_SIZE - iIndexSize);
    
    uint32_t leastRecent = UINT32_MAX;
    int evict = -1;
    for (int k = 0; k < icacheAssoc; k++) 
    {
      if (iCache.setList[index].blockList[k].isValid == FALSE) 
      {
        iCache.setList[index].blockList[k].isValid = TRUE;
        iCache.setList[index].blockList[k].timeStamp = icacheRefs;
        iCache.setList[index].blockList[k].tag = tag;
        return -1;
      } 
      else 
      {
        if (iCache.setList[index].blockList[k].timeStamp < leastRecent)
        {
          leastRecent = iCache.setList[index].blockList[k].timeStamp;
          evict = k;
        }
      }
    }
    iCache.setList[index].blockList[evict].timeStamp = icacheRefs;
    iCache.setList[index].blockList[evict].tag = tag;
    return evict;
  }
  else if (IorD == 'D')
  {
    int tag = addr >> (ADDR_SIZE - dTagSize);
    int index = (addr << dTagSize) >> (ADDR_SIZE - dIndexSize);
    uint32_t leastRecent = UINT32_MAX;
    int evict = -1;
    for (int k = 0; k < dcacheAssoc; k++) 
    {
      if (dCache.setList[index].blockList[k].isValid == FALSE) 
      {
        dCache.setList[index].blockList[k].isValid = TRUE;
        dCache.setList[index].blockList[k].timeStamp = dcacheRefs;
        dCache.setList[index].blockList[k].tag = tag;
        return -1;
      } 
      else 
      {
        if (dCache.setList[index].blockList[k].timeStamp < leastRecent)
        {
          leastRecent = dCache.setList[index].blockList[k].timeStamp;
          evict = k;
        }
      }
    }
    dCache.setList[index].blockList[evict].timeStamp = dcacheRefs;
    dCache.setList[index].blockList[evict].tag = tag;
    return evict;
  }
  else 
  {
    int tag = addr >> (ADDR_SIZE - l2TagSize);
    int index = (addr << l2TagSize) >> (ADDR_SIZE - l2IndexSize);
    uint32_t leastRecent = UINT32_MAX;
    int evict = -1;
    for (int k = 0; k < l2cacheAssoc; k++) 
    {
      if (l2Cache.setList[index].blockList[k].isValid == FALSE) 
      {
        l2Cache.setList[index].blockList[k].isValid = TRUE;
        l2Cache.setList[index].blockList[k].timeStamp = l2cacheRefs;
        l2Cache.setList[index].blockList[k].tag = tag;
        return -1;
      } 
      else 
      {
        if (l2Cache.setList[index].blockList[k].timeStamp < leastRecent)
        {
          leastRecent = l2Cache.setList[index].blockList[k].timeStamp;
          evict = k;
        }
      }
    }
    l2Cache.setList[index].blockList[evict].timeStamp = l2cacheRefs;
    l2Cache.setList[index].blockList[evict].tag = tag;
    return evict;
  }
}

void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;
  
  //
  //TODO: Initialize Cache Simulator Data Structures
  //

  offsetSize = calc_log2(blocksize);
  iIndexSize = calc_log2(icacheSets);
  dIndexSize = calc_log2(dcacheSets);
  l2IndexSize = calc_log2(l2cacheSets);

  printf("offset size: %d\n", (int)offsetSize);
  printf("i index size: %d\n", (int)iIndexSize);
  printf("d index size: %d\n", (int)dIndexSize);
  printf("l2 index size: %d\n", (int)l2IndexSize);

  iTagSize = ADDR_SIZE - offsetSize - iIndexSize;
  dTagSize = ADDR_SIZE - offsetSize - dIndexSize;
  l2TagSize = ADDR_SIZE - offsetSize - l2IndexSize;

  iCache.setList = malloc(sizeof(Set) * icacheSets);
  dCache.setList = malloc(sizeof(Set) * dcacheSets);
  l2Cache.setList = malloc(sizeof(Set) * l2cacheSets);

  for (int s = 0; s < icacheSets; s++) 
  {
    iCache.setList[s].blockList = malloc(sizeof(Block) * icacheAssoc);
    for(int b = 0; b < icacheAssoc; b++) {
      iCache.setList[s].blockList[b].tag = 0;
      iCache.setList[s].blockList[b].timeStamp = 0;
      iCache.setList[s].blockList[b].isValid = FALSE;
    }
  }

  for (int s = 0; s < dcacheSets; s++) 
  {
    dCache.setList[s].blockList = malloc(sizeof(Block) * dcacheAssoc);
    for(int b = 0; b < dcacheAssoc; b++) {
      dCache.setList[s].blockList[b].tag = 0;
      dCache.setList[s].blockList[b].timeStamp = 0;
      dCache.setList[s].blockList[b].isValid = FALSE;
    }
  }
  
  for (int s = 0; s < l2cacheSets; s++) 
  {
    l2Cache.setList[s].blockList = malloc(sizeof(Block) * l2cacheAssoc);
    for(int b = 0; b < l2cacheAssoc; b++) {
      l2Cache.setList[s].blockList[b].tag = 0;
      l2Cache.setList[s].blockList[b].timeStamp = 0;
      l2Cache.setList[s].blockList[b].isValid = FALSE;
    }
  }
}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//

// uint32_t
// icache_access(uint32_t addr)
// {
//   //
//   //TODO: Implement I$
//   //
//   if (icacheSets == 0) {
//     return l2cache_access(addr);
//   }

//   icacheRefs++;
//   int tag = addr >> (ADDR_SIZE - iTagSize);
//   int index = (addr << iTagSize) >> (ADDR_SIZE - iIndexSize);

//   for (int k = 0; k < icacheAssoc; k++) 
//   {
//     if (iCache.setList[index].blockList[k].isValid == TRUE && 
//         iCache.setList[index].blockList[k].tag == tag) {
//       // hit
//       iCache.setList[index].blockList[k].timeStamp = icacheRefs;
//       return icacheHitTime;
//     }
//   }

//   icacheMisses++;
//   uint32_t l2Penalty = l2cache_access(addr);
//   icachePenalties += l2Penalty;

//   uint32_t leastRecent = UINT32_MAX;
//   int evict = -1;
//   for (int k = 0; k < icacheAssoc; k++) 
//   {
//     if (iCache.setList[index].blockList[k].isValid == FALSE) 
//     {
//       iCache.setList[index].blockList[k].isValid = TRUE;
//       iCache.setList[index].blockList[k].timeStamp = icacheRefs;
//       iCache.setList[index].blockList[k].tag = tag;
//       return icacheHitTime + l2Penalty;
//     } 
//     else 
//     {
//       if (iCache.setList[index].blockList[k].timeStamp < leastRecent)
//       {
//         leastRecent = iCache.setList[index].blockList[k].timeStamp;
//         evict = k;
//       }
//     }
//   }

//   iCache.setList[index].blockList[evict].timeStamp = icacheRefs;
//   iCache.setList[index].blockList[evict].tag = tag;
//   return icacheHitTime + l2Penalty;
// }

uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //

  IorD = 'I';
  uint32_t accessTime; 

  if (icacheSets == 0) {
    accessTime = l2cache_access(addr);
  }
  else 
  {
    icacheRefs++;
    if (is_miss(addr) == TRUE)
    {
      icacheMisses++;
      uint32_t l2AccessTime = l2cache_access(addr);
      lru_add(addr);  
      icachePenalties += l2AccessTime;
      accessTime = icacheHitTime + l2AccessTime;
    } 
    else
    {
      accessTime = icacheHitTime;
    }
  }

  return accessTime;
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//

// uint32_t
// dcache_access(uint32_t addr)
// {
//   //
//   //TODO: Implement D$
//   //
//   IorD = 'D';

//   if (dcacheSets == 0) {
//     return l2cache_access(addr);
//   }

//   dcacheRefs++;
//   int tag = addr >> (ADDR_SIZE - dTagSize);
//   int index = (addr << dTagSize) >> (ADDR_SIZE - dIndexSize);

//   for (int k = 0; k < dcacheAssoc; k++) 
//   {
//     if (dCache.setList[index].blockList[k].isValid == TRUE && 
//         dCache.setList[index].blockList[k].tag == tag) {
//       // hit
//       dCache.setList[index].blockList[k].timeStamp = dcacheRefs;
//       return dcacheHitTime;
//     }
//   }

//   dcacheMisses++;
//   uint32_t l2Penalty = l2cache_access(addr);
//   dcachePenalties += l2Penalty;

//   uint32_t leastRecent = UINT32_MAX;
//   int evict = -1;
//   for (int k = 0; k < dcacheAssoc; k++) 
//   {
//     if (dCache.setList[index].blockList[k].isValid == FALSE) 
//     {
//       dCache.setList[index].blockList[k].isValid = TRUE;
//       dCache.setList[index].blockList[k].timeStamp = dcacheRefs;
//       dCache.setList[index].blockList[k].tag = tag;
//       return dcacheHitTime + l2Penalty;
//     } 
//     else 
//     {
//       if (dCache.setList[index].blockList[k].timeStamp < leastRecent)
//       {
//         leastRecent = dCache.setList[index].blockList[k].timeStamp;
//         evict = k;
//       }
//     }
//   }
  
//   dCache.setList[index].blockList[evict].timeStamp = dcacheRefs;
//   dCache.setList[index].blockList[evict].tag = tag;
//   return dcacheHitTime + l2Penalty;
// }


uint32_t
dcache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //
  uint32_t accessTime;
  IorD = 'D';

  if (dcacheSets == 0) {
    accessTime = l2cache_access(addr);
  }
  else 
  {
    dcacheRefs++;
    if (is_miss(addr) == TRUE)
    {
      dcacheMisses++;
      uint32_t l2AccessTime = l2cache_access(addr);
      lru_add(addr);  
      dcachePenalties += l2AccessTime;
      accessTime = dcacheHitTime + l2AccessTime;
    } 
    else
    {
      accessTime = dcacheHitTime;
    }
  }

  return accessTime;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//

// uint32_t
// l2cache_access(uint32_t addr)
// {
//   //
//   //TODO: Implement L2$
//   //

//   if (l2cacheSets == 0) {
//     return memspeed;
//   }

//   l2cacheRefs++;
//   int tag = addr >> (ADDR_SIZE - l2TagSize);
//   int index = (addr << l2TagSize) >> (ADDR_SIZE - l2IndexSize);

//   for (int k = 0; k < l2cacheAssoc; k++) 
//   {
//     if (l2Cache.setList[index].blockList[k].isValid == TRUE && 
//         l2Cache.setList[index].blockList[k].tag == tag) {
//       // hit
//       l2Cache.setList[index].blockList[k].timeStamp = l2cacheRefs;
//       return l2cacheHitTime;
//     }
//   }

//   l2cacheMisses++;
//   l2cachePenalties += memspeed;

//   uint32_t leastRecent = UINT32_MAX;
//   int evict = -1;
//   for (int k = 0; k < l2cacheAssoc; k++) 
//   {
//     if (l2Cache.setList[index].blockList[k].isValid == FALSE) 
//     {
//       l2Cache.setList[index].blockList[k].isValid = TRUE;
//       l2Cache.setList[index].blockList[k].timeStamp = l2cacheRefs;
//       l2Cache.setList[index].blockList[k].tag = tag;
//       return l2cacheHitTime + memspeed;
//     } 
//     else 
//     {
//       if (l2Cache.setList[index].blockList[k].timeStamp < leastRecent)
//       {
//         leastRecent = l2Cache.setList[index].blockList[k].timeStamp;
//         evict = k;
//       }
//     }
//   }
  
//   // inclusive
//   if(inclusive == TRUE)
//   {
//     uint32_t evictTag = l2Cache.setList[index].blockList[evict].tag;
//     uint32_t evictIndex = index;
//     uint32_t evictAddress = (evictTag << (offsetSize + l2IndexSize)) | (evictIndex << offsetSize);

//     if(icacheSets > 0)
//     {
//       uint32_t iEvictTag = evictAddress >> (offsetSize + iIndexSize);
//       uint32_t iEvictIndex = (evictAddress << iTagSize) >> (iTagSize + offsetSize);
//       for(int k = 0; k < icacheAssoc; k++)
//       {
//         if(iCache.setList[iEvictIndex].blockList[k].isValid == TRUE && 
//           iCache.setList[iEvictIndex].blockList[k].tag == iEvictTag)
//         {
//           iCache.setList[iEvictIndex].blockList[k].isValid = FALSE;
//         }
//       }
//     }

//     if(dcacheSets > 0)
//     {
//       uint32_t dEvictTag = evictAddress >> (offsetSize + dIndexSize);
//       uint32_t dEvictIndex = (evictAddress << dTagSize) >> (dTagSize + offsetSize);
//       for(int k = 0; k < dcacheAssoc; k++)
//       {
//         if(dCache.setList[dEvictIndex].blockList[k].isValid == TRUE &&
//           dCache.setList[dEvictIndex].blockList[k].tag == dEvictTag)
//         {
//           dCache.setList[dEvictIndex].blockList[k].isValid = FALSE;
//         }
//       }
//     }
//   }

//   l2Cache.setList[index].blockList[evict].timeStamp = l2cacheRefs;
//   l2Cache.setList[index].blockList[evict].tag = tag;
//   return l2cacheHitTime + memspeed;
// }

uint32_t
l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //
  uint32_t accessTime;
  IorD = 'L';

  if (l2cacheSets == 0) {
    accessTime = memspeed;
  }
  else 
  {
    l2cacheRefs++;
    if (is_miss(addr) == TRUE) 
    {
      l2cacheMisses++;
      l2cachePenalties += memspeed;
      accessTime = l2cacheHitTime + memspeed;
      int evictBlock = lru_add(addr);
      if (inclusive == TRUE && evictBlock >= 0)
        lru_evict(addr, evictBlock);
    }
    else 
    {
      accessTime = l2cacheHitTime;
    }
  }

  return accessTime;
}

void
lru_evict(uint32_t addr, int evictBlock)
{
  int tag = addr >> (ADDR_SIZE - l2TagSize);
  int index = (addr << l2TagSize) >> (ADDR_SIZE - l2IndexSize);

  uint32_t evictTag = l2Cache.setList[index].blockList[evictBlock].tag;
  uint32_t evictIndex = index;
  uint32_t evictAddress = (evictTag << (offsetSize + l2IndexSize)) | (evictIndex << offsetSize);

  if(icacheSets > 0)
  {
    uint32_t iEvictTag = evictAddress >> (offsetSize + iIndexSize);
    uint32_t iEvictIndex = (evictAddress << iTagSize) >> (iTagSize + offsetSize);
    for(int k = 0; k < icacheAssoc; k++)
    {
      if(iCache.setList[iEvictIndex].blockList[k].isValid == TRUE && 
        iCache.setList[iEvictIndex].blockList[k].tag == iEvictTag)
      {
        iCache.setList[iEvictIndex].blockList[k].isValid = FALSE;
      }
    }
  }

  if(dcacheSets > 0)
  {
    uint32_t dEvictTag = evictAddress >> (offsetSize + dIndexSize);
    uint32_t dEvictIndex = (evictAddress << dTagSize) >> (dTagSize + offsetSize);
    for(int k = 0; k < dcacheAssoc; k++)
    {
      if(dCache.setList[dEvictIndex].blockList[k].isValid == TRUE &&
        dCache.setList[dEvictIndex].blockList[k].tag == dEvictTag)
      {
        dCache.setList[dEvictIndex].blockList[k].isValid = FALSE;
      }
    }
  }
}
