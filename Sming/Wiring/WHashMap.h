/* $Id: HashMap.h 1198 2011-06-14 21:08:27Z bhagman $
||
|| @author         Alexander Brevig <abrevig@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://alexanderbrevig.com/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
||
|| @description
|| | Implementation of a HashMap data structure.
|| |
|| | Wiring Cross-platform Library
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef HASHMAP_H
#define HASHMAP_H

#include "Countable.h"

template<typename K, typename V>
class HashMap
{
  public:
    typedef bool (*comparator)(K, K);

    /*
    || @constructor
    || | Initialize this HashMap
    || #
    ||
    || @parameter compare optional function for comparing a key against another (for complex types)
    */
    HashMap(comparator compare = 0)
    {
      cb_comparator = compare;
      currentIndex = 0;
      size = 0;
      keys = NULL;
      values = NULL;
    }

    ~HashMap()
    {
    	clear();
    }

    /*
    || @description
    || | Get the size of this HashMap
    || #
    ||
    || @return The size of this HashMap
    */
    unsigned int count() const
    {
      return currentIndex;
    }

    /*
    || @description
    || | Get a key at a specified index
    || #
    ||
    || @parameter idx the index to get the key at
    ||
    || @return The key at index idx
    */
    K keyAt(unsigned int idx) const
    {
      return *keys[idx];
    }

    /*
    || @description
    || | Get a value at a specified index
    || #
    ||
    || @parameter idx the index to get the value at
    ||
    || @return The value at index idx
    */
    V valueAt(unsigned int idx) const
    {
      return *values[idx];
    }

    /*
    || @description
    || | An indexer for accessing and assigning a value to a key
    || | If a key is used that exists, it returns the value for that key
    || | If there exists no value for that key, the key is added
    || #
    ||
    || @parameter key the key to get the value for
    ||
    || @return The const value for key
    */
    const V& operator[](const K key) const
    {
      return operator[](key);
    }

    /*
    || @description
    || | An indexer for accessing and assigning a value to a key
    || | If a key is used that exists, it returns the value for that key
    || | If there exists no value for that key, the key is added
    || #
    ||
    || @parameter key the key to get the value for
    ||
    || @return The value for key
    */
    V& operator[](const K key)
    {
      if (contains(key))
      {
        return *values[indexOf(key)];
      }
      if (currentIndex >= size)
      {
    	  allocate(currentIndex + 1);
      }
      *keys[currentIndex] = key;
      *values[currentIndex] = nil;
      currentIndex++;
      return *values[currentIndex - 1];
    }

    void allocate(int newSize)
    {
    	if (newSize <= size) return;

    	K** nkeys = new K*[newSize];
    	V** nvalues = new V*[newSize];

    	if (keys != NULL)
    	{
			for (int i = 0; i < size; i++)
			{
				nkeys[i] = keys[i];
				nvalues[i] = values[i];
			}

			delete[] keys;
			delete[] values;
    	}
		for (int i = size; i < newSize; i++)
		{
			nkeys[i] = new K();
			nvalues[i] = new V();
		}

    	keys = nkeys;
    	values = nvalues;
    	size = newSize;
    }

    /*
    || @description
    || | Get the index of a key
    || #
    ||
    || @parameter key the key to get the index for
    ||
    || @return The index of the key, or -1 if key does not exist
    */
    unsigned int indexOf(K key) const
    {
      for (int i = 0; i < currentIndex; i++)
      {
        if (cb_comparator)
        {
          if (cb_comparator(key, *keys[i]))
          {
            return i;
          }
        }
        else
        {
          if (key == *keys[i])
          {
            return i;
          }
        }
      }
      return -1;
    }

    /*
    || @description
    || | Check if a key is contained within this HashMap
    || #
    ||
    || @parameter key the key to check if is contained within this HashMap
    ||
    || @return true if it is contained in this HashMap
    */
    bool contains(K key) const
    {
      for (int i = 0; i < currentIndex; i++)
      {
        if (cb_comparator)
        {
          if (cb_comparator(key, *keys[i]))
          {
            return true;
          }
        }
        else
        {
          if (key == *keys[i])
          {
            return true;
          }
        }
      }
      return false;
    }

    /*
    || @description
    || | Check if a key is contained within this HashMap
    || #
    ||
    || @parameter key the key to remove from this HashMap
    */
    void remove(K key)
    {
      int index = indexOf(key);
      if (contains(key))
      {
        for (int i = index; i < size - 1; i++)
        {
          *keys[i] = *keys[i + 1];
          *values[i] = *values[i + 1];
        }
        currentIndex--;
      }
    }

    void clear()
    {
    	if (keys != NULL)
    	{
    		for (int i = 0; i < size; i++)
			{
				delete keys[i];
				delete values[i];
			}
			delete[] keys;
			delete[] values;
			keys = NULL;
			values = NULL;
    	}
    	currentIndex = 0;
    	size = 0;
    }

    void setMultiple(const HashMap<K, V>& map)
    {
    	for (int i = 0; i < map.count(); i++)
    	{
    		(*this)[map.keyAt(i)] = map.valueAt(i);
    	}
    }

    void setNullValue(V nullv)
    {
      nil = nullv;
    }

  protected:
    K **keys;
    V **values;
    V nil;
    int16_t currentIndex;
    int16_t size;
    comparator cb_comparator;

  private:
    HashMap(const HashMap<K, V>& that);
};

#endif
// HASHMAP_H
