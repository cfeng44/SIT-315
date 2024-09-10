/*
 *  File: Vector.cs
 *  Version: 3.0
 *  Project/s:
 *    - 1.1P
 *    - 3.1P
 *    - 3.2D
 *  Author: Codey Funston
 *  Created: 4 March 2024
 *  Updated: 25 April 2024
 */

using System.Buffers;
using System.ComponentModel.Design.Serialization;
using System.Runtime.CompilerServices;
using System.Text;
using System.Collections.Generic;
using System;
using System.Collections;

namespace Vector
{
  public class Vector<T> : IEnumerable<T> where T : IComparable<T>
  {
    public IEnumerator<T> GetEnumerator()
    {
      return new EnumVector(this);
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
      return GetEnumerator();
    }

    private class EnumVector : IEnumerator<T>
    {
      private Vector<T> vec;
      // -1 is the positon before the first element in the collection since 
      // calling MoveNext() will increment it to 0, the index of the first
      // element.
      private int currentIndex = -1;

      public EnumVector(Vector<T> vector)
      {
        this.vec = vector;
      }

      public T Current
      {
        get
        {
          if (currentIndex == -1 || currentIndex >= vec.Count)
          {
            // Could also throw an exception but the test case wants the default value returned
            // when accessing the collection's current element before MoveNext() has been called
            // or after MoveNext() has returned false.
            return default(T);
          }
          else
          {
            return vec.data[currentIndex];
          }
        }
      }

      object IEnumerator.Current
      {
        get
        {
          return Current;
        }
      }

      /// <returns>Ture if there is another element in the collection. False if positioned
      /// after the last element.</returns>
      public bool MoveNext()
      {
        currentIndex++;
        return (currentIndex < vec.Count);
      }

      public void Reset()
      {
        currentIndex = -1;
      }

      public void Dispose()
      {
        vec.Clear();
      }
    }

    private const int DEFAULT_CAPACITY = 10;
    private T[] data;

    public int Count { get; private set; } = 0;
    public int Capacity { get; private set; } = 0;

    /// <exception cref="ArgumentOutOfRangeException"></exception>
    public Vector(int capacity)
    {
      ArgumentOutOfRangeException.ThrowIfNegative(capacity);
      data = new T[capacity];
      Capacity += capacity;
    }

    public Vector() : this(DEFAULT_CAPACITY) { }

    /// <exception cref="IndexOutOfRangeException"></exception>
    public T this[int index]
    {
      get
      {
        if (index >= Count || index < 0)
          throw new IndexOutOfRangeException();
        return data[index];
      }
      set
      {
        if (index >= Count || index < 0)
          throw new IndexOutOfRangeException();
        data[index] = value;
      }
    }

    private void ExtendData(int extraCapacity)
    {
      T[] newData = new T[data.Length + extraCapacity];
      for (int i = 0; i < Count; i++) newData[i] = data[i];
      data = newData;
      Capacity += extraCapacity;
    }

    public void Add(T element)
    {
      if (Count == data.Length)
        ExtendData(DEFAULT_CAPACITY);
      data[Count++] = element;
    }

    /// <returns>
    /// Index if present, else <c> -1 </c>.
    /// </returns>
    public int IndexOf(T element)
    {
      for (var i = 0; i < Count; i++)
      {
        if (data[i].Equals(element))
          return i;
      }
      return -1;
    }

    /// <exception cref="IndexOutOfRangeException"></exception>
    public void Insert(int index, T element)
    {
      if (index > Count || index < 0) throw new IndexOutOfRangeException();
      if (Count == Capacity) ExtendData(DEFAULT_CAPACITY);

      for (int i = Count - 1; i >= index; i--)
      {
        data[i + 1] = data[i];
      }
      data[index] = element;
      Count++;
    }

    public void Clear()
    {
      for (int i = 0; i < Count; i++)
      {
        data[i] = default(T);
      }
      Count = 0;
    }

    /// <returns><c> true </c> if present, else <c> false </c></returns>
    public bool Contains(T element)
    {
      if (IndexOf(element) >= 0) return true;
      return false;
    }

    /// <returns><c> true </c> if removed, else <c> false </c></returns>
    public bool Remove(T element)
    {
      int index = IndexOf(element);
      if (index < 0) return false;

      try
      {
        RemoveAt(index);
      }
      catch (IndexOutOfRangeException)
      {
        throw;
      }

      if (IndexOf(element) < 0) return true;
      return false;
    }

    /// <summary>
    /// Sets the element at <c> index </c> to the default value of <c> T </c>.
    /// </summary>
    /// <exception cref="IndexOutOfRangeException"></exception>
    public void RemoveAt(int index)
    {
      if (index >= Count || index < 0) throw new IndexOutOfRangeException();

      if (index == Count - 1) data[index] = default(T);
      else
      {
        for (int i = index; i < Count; i++)
        {
          data[i] = data[i + 1];
        }
      }
      Count--;
    }

    /// <returns> String of the form [element_1, element_2, ..., element_n] </returns>
    public override string ToString()
    {
      StringBuilder vectorString = new StringBuilder();

      vectorString.Append("[");
      for (int i = 0; i < Count; i++)
      {
        vectorString.Append(data[i]);
        vectorString.Append(",");
      }
      vectorString.Append("]");

      return vectorString.ToString();
    }

    public ISorter Sorter { set; get; } = new DefaultSorter();

    internal class DefaultSorter : ISorter
    {
      public void Sort<K>(K[] sequence, IComparer<K> comparer) where K : IComparable<K>
      {
        if (comparer == null) comparer = Comparer<K>.Default;
        Array.Sort(sequence, comparer);
      }
    }

    public void Sort()
    {
      if (Sorter == null) Sorter = new DefaultSorter();
      Array.Resize(ref data, Count);
      Sorter.Sort(data, null);
    }

    public void Sort(IComparer<T> comparer)
    {
      if (Sorter == null) Sorter = new DefaultSorter();
      Array.Resize(ref data, Count);
      if (comparer == null) Sorter.Sort(data, null);
      else Sorter.Sort(data, comparer);
    }

    /// <returns>
    /// Index if present, else <c> -1 </c>.
    /// </returns>
    public int BinarySearch(T item)
    {
      IComparer<T> comp = Comparer<T>.Default;
      int leftPos = 0;
      int rightPos = Count - 1;
      int currentMiddle = -1;

      // If leftPos is more than rightPos this means the element is not in the array
      while (leftPos <= rightPos)
      {
        currentMiddle = (leftPos + rightPos) / 2;
        if (comp.Compare(item, data[currentMiddle]) > 0)
        {
          // Shrink area to search from the left
          leftPos = currentMiddle + 1;
        }
        else if (comp.Compare(item, data[currentMiddle]) < 0)
        {
          // Shrink area to search from the right
          rightPos = currentMiddle - 1;
        }
        else
        {
          break; // (Found)
        }

      }

      if (leftPos <= rightPos)
      {
        return currentMiddle;
      }
      else
      {
        return -1;
      }
    }

    /// <returns>
    /// Index if present, else <c> -1 </c>.
    /// </returns>
    public int BinarySearch(T item, IComparer<T> comparer)
    {
      int leftPos = 0;
      int rightPos = Count - 1;
      int currentMiddle = -1;

      // If leftPos is more than rightPos this means the element is not in the array
      while (leftPos <= rightPos)
      {
        currentMiddle = (leftPos + rightPos) / 2;
        if (comparer.Compare(item, data[currentMiddle]) > 0)
        {
          // Shrink area to search from the left
          leftPos = currentMiddle + 1;
        }
        else if (comparer.Compare(item, data[currentMiddle]) < 0)
        {
          // Shrink area to search from the right
          rightPos = currentMiddle - 1;
        }
        else
        {
          break; // (Found)
        }
      }

      if (leftPos <= rightPos)
      {
        return currentMiddle;
      }
      else
      {
        return -1;
      }
    }
  }

  public class InsertionSort : ISorter
  {
    public InsertionSort() { }

    public void Sort<K>(K[] sequence, IComparer<K> comparer) where K : IComparable<K>
    {
      for (int i = 1; i < sequence.Length; i++)
      {
        int j = i;
        while (j > 0 && comparer.Compare(sequence[j - 1], sequence[j]) > 0)
        {
          // Each iteration of this loop essentially does a reverse bubble of the (i)th
          // element down to its position, as if every element above index i does not
          // exist yet, this allows for less comparison than bubble sort.
          Tools.Swap(ref sequence[j - 1], ref sequence[j]);
          j--;
        }
      }
    }
  }

  public class BubbleSort : ISorter
  {
    public BubbleSort() { }

    public void Sort<K>(K[] sequence, IComparer<K> comparer) where K : IComparable<K>
    {
      for (int i = 1; i < sequence.Length; i++)
      {
        int num_sorts = 0;
        for (int j = 0; j < sequence.Length - i; j++)
        // Each run of this loop moves the largest element in the sequence to the end
        // (excluding already moved largest elements from previous runthroughs). e.g for a
        // sequence of integers [1][4][3][3][0][7][1][1], 4 would be moved up the list
        // until it is compared with 7, then 7 would be moved until it reaches the end.
        {
          if (comparer.Compare(sequence[j], sequence[j + 1]) > 0)
          {
            Tools.Swap(ref sequence[j], ref sequence[j + 1]);
            num_sorts++;
          }
        }
        // run if all elements in the sequence are in order per the copmarer
        if (num_sorts == 0) break;
      }
    }
  }

  public class SelectionSort : ISorter
  {
    public SelectionSort() { }

    // This sorting algorithm finds the minimum for the current selection of the array
    // and moves it to the index i, ie the lowest index position of the current selection.
    public void Sort<K>(K[] sequence, IComparer<K> comparer) where K : IComparable<K>
    {
      for (int i = 0; i < sequence.Length - 1; i++)
      {
        int minimum = i;
        for (int j = i + 1; j < sequence.Length; j++)
        {
          if (comparer.Compare(sequence[minimum], sequence[j]) > 0) minimum = j;
        }

        if (minimum != i)
        {
          Tools.Swap(ref sequence[i], ref sequence[minimum]);
        }
      }
    }
  }

  public class RandomizedQuickSort : ISorter
  {
    public RandomizedQuickSort() { }

    public void Sort<K>(K[] sequence, IComparer<K> comparer) where K : IComparable<K>
    {
      QuickSort(sequence, comparer, 0, sequence.Length - 1);
    }

    // *Based on SIT221 Unit Textbook, Code Fragment 12.6
    public void QuickSort<K>(K[] sequence, IComparer<K> comparer, int start, int end) where K : IComparable<K>
    {
      if (start >= end) return;

      int leftPos = start;
      int rightPos = end;
      Random rnd = new Random();
      K pivotElement = sequence[rnd.Next(start, end)];

      while (leftPos <= rightPos)
      {
        while (leftPos <= rightPos && comparer.Compare(pivotElement, sequence[leftPos]) > 0)
        {
          leftPos++; // moves right through array
        }
        while (leftPos <= rightPos && comparer.Compare(pivotElement, sequence[rightPos]) < 0)
        {
          rightPos--; // moves left through array
        }
        if (leftPos <= rightPos)
        {
          Tools.Swap(ref sequence[leftPos], ref sequence[rightPos]);
          leftPos++; // -->
          rightPos--; // <--
        }
      }

      Tools.Swap(ref sequence[leftPos], ref pivotElement);
      QuickSort<K>(sequence, comparer, start, leftPos - 1);
      QuickSort<K>(sequence, comparer, leftPos + 1, end);
    }
  }

  public class MergeSortTopDown : ISorter
  {
    public MergeSortTopDown() { }

    // *Based on SIT221 Unit Textbook, Code Fragment 12.2. It sorts the two initial splits of the
    // array one before the other, this is handy for large inputs and distributed compute.
    public void Sort<K>(K[] sequence, IComparer<K> comparer) where K : IComparable<K>
    {
      if (sequence.Length <= 1) return;
      int split = sequence.Length / 2;

      K[] split1 = new K[split];
      K[] split2 = new K[sequence.Length - split];
      Array.Copy(sequence, 0, split1, 0, split);
      Array.Copy(sequence, split, split2, 0, sequence.Length - split);

      Sort<K>(split1, comparer);
      Sort<K>(split2, comparer);

      Tools.MergeTopDown(split1, split2, sequence, comparer);
    }
  }

  public class MergeSortBottomUp : ISorter
  {
    public MergeSortBottomUp() { }

    // *Based on SIT221 Unit Textbook, Code Fragment 12.4
    public void Sort<K>(K[] sequence, IComparer<K> comparer) where K : IComparable<K>
    {
      int length = sequence.Length;
      K[] destArr = new K[length];

      for (int i = 1; i < length; i *= 2)
      {
        for (int j = 0; j < length; j += 2 * i)
        {
          Tools.MergeBottomUp(sequence, destArr, comparer, j, i);
        }
        Tools.Swap(sequence, destArr);
      }
    }
  }

  public class Tools
  {
    public static void Swap<K>(ref K a, ref K b)
    {
      K holder = a;
      a = b;
      b = holder;
    }

    public static void Swap<K>(K[] arrA, K[] arrB)
    {
      // Originally I tried changing the arrays like in the Swap<K>() method with standard inputs,
      // but that only changes the pointers not the values.
      for (int i = 0; i < arrA.Length; i++)
      {
        K holder = arrA[i];
        arrA[i] = arrB[i];
        arrB[i] = holder;
      }
    }
  }
    