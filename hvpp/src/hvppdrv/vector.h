#pragma once

template <class T>
class vector
{
public:
  typedef T* iterator;

  vector()
  {
    my_capacity = 0;
    my_size = 0;
    buffer = 0;
  }
  vector(unsigned int size)
  {
    my_capacity = size;
    my_size = size;
    buffer = new T[size];
  }
  vector(unsigned int size, const T& initial)
  {
    my_size = size;
    my_capacity = size;
    buffer = new T[size];
    for (unsigned int i = 0; i < size; i++)
      buffer[i] = initial;
    //T();
  }
  vector(const vector<T>& v)
  {
    my_size = v.my_size;
    my_capacity = v.my_capacity;
    buffer = new T[my_size];
    for (unsigned int i = 0; i < my_size; i++)
      buffer[i] = v.buffer[i];
  }
  ~vector()
  {
    delete[] buffer;
  }

  unsigned int capacity() const
  {
    return my_capacity;
  }
  unsigned int size() const//
  {
    return my_size;
  }
  iterator begin()
  {
    return buffer;
  }
  iterator end()
  {
    return buffer + size();
  }
  T& front()
  {
    return buffer[0];
  }
  T& back()
  {
    return buffer[my_size - 1];
  }
  void push_back(const T& v)
  {
    if (my_size >= my_capacity)
      reserve(my_capacity + 5);
    buffer[my_size++] = v;
  }
  void pop_back()
  {
    buffer[my_size]->~T();
    my_size--;
  }

  void reserve(unsigned int capacity)
  {
    if (buffer == 0)
    {
      my_size = 0;
      my_capacity = 0;
    }
    T* Newbuffer = new T[capacity];
    //assert(Newbuffer);
    unsigned int l_Size = capacity < my_size ? capacity : my_size;
    //copy (buffer, buffer + l_Size, Newbuffer);

    for (unsigned int i = 0; i < l_Size; i++)
      Newbuffer[i] = buffer[i];

    my_capacity = capacity;
    delete[] buffer;
    buffer = Newbuffer;
  }
  void resize(unsigned int size)
  {
    reserve(size);
    my_size = size;
  }

  T& operator[](unsigned int index)
  {
    return buffer[index];
  }
  vector<T>& operator=(const vector<T>& v)
  {
    delete[] buffer;
    my_size = v.my_size;
    my_capacity = v.my_capacity;
    buffer = new T[my_size];
    for (unsigned int i = 0; i < my_size; i++)
      buffer[i] = v.buffer[i];
    return *this;
  }

  void clear()
  {
    my_capacity = 0;
    my_size = 0;
    delete[] buffer;
    buffer = 0;
  }

private:
  unsigned int my_size;
  unsigned int my_capacity;
  T* buffer;
};
