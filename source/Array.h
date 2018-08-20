#pragma once

#include <assert.h>
#include <algorithm>
#include <vector>
using namespace std;

class xStream;

/******************************************************************
*
* xArray: like std::vector
*
******************************************************************/
template<class T>
class xArray : public std::vector<T>
{
	typedef const T& const_reference;
	typedef T& reference;

public:
/*	const_iterator begin() const
	{
		return std::vector<T>::begin();
	}

	iterator begin()
	{
		return std::vector<T>::begin();
	}
*/
    const_reference At(int i) const
    {
		ASSERT(i >= 0 && i < Count()); 
		return begin()[i];
    }

    reference At(int i)        
    {
       
		ASSERT(i >= 0 && i < Count()); 
        return std::vector<T>::begin()[i];
    }

    const_reference operator[](int i) const    
    {  
        return At(i); 
    }

    reference operator[](int i)    
    { 
        return At(i); 
    }

    void Copy(const xArray<T>& src)    
    { 
        *this = src;    
    }

    void Append(const xArray<T>& src)
    { 
        insert(end(), src.begin(), src.end());
    }

    void InsertAt(int i, const T& arg) 
    { 
        insert(begin()+i, arg);
    }

    void RemoveAt(int i)        
    { 
        erase(begin()+i);
    }

    //�Ƿ������ӣ��������ͼ���࣬�ǲ����������ظ���
    void Add(const T& arg)        
    {
        if(CanAdd(arg))
        {
            push_back(arg);
        }
    }

    virtual bool CanAdd(const T& arg)
    {
        return true;
    }

    void AddRange(const xArray<T>& arg)        
    {
        int count = arg.Count();
        const_iterator p = arg.begin();
        for (int i = 0; i < count; i++)
        {
            Add(*p);
            p++;
        }
    }

    int Count() const
    {
        return (int)size();
    }

    void Clear()
    {
        clear();
    }

    /*
    * reverse
    */
    void Reverse()        
    { 
        /*int count = Count();
        count /= 2;
        reverse_iterator rit = rbegin();
        iterator it = begin();
        for (int i = 0; i < count; i++)
        {
            T& p = rit[i];
            rit[i] = it[i];
            it[i] = p;
        }*/
    }

    /*
    * ����ָ����arg������,-1��ʾδ�ҵ�
    */
    int Find(const T& arg) const
    {
        int count = Count();
        const_iterator p = begin();
        for (int rtv = 0; rtv < count; rtv++, p++)
        {
            if((*p) == arg)
                return (int)rtv;
        }
        return -1;
    }

    /*
    * return true if p is in PtrArray & delete successfully
    */
    BOOL Remove(const T& arg)
    {
        int count = Count();
        iterator p = begin();
        for (int i = 0; i < count; i++, p++)
        {
            if((*p) == arg)
            {
                erase(p);
                return true;
            }
        }
        return false;
    }

    /*
    * replace all oldVal with newVal
    */
    int Replace(const T& oldVal, const T& newVal)
    {
        int rtv = 0;
        iterator p = begin();
        int count = Count();
        for (int i = 0; i < count; i++, p++)
        {
            if(*p == oldVal)
            {
                *p = newVal;
                rtv++;
            }
        }
        return rtv;
    }

    /*
    *    array serialize function : read/write memory
    */
   /* void ReadDirect(xStream& stm)
    {
        int n = stm.ReadInt32();
        resize(n);
        if (n > 0)
            stm.Read(static_cast<void*>(&*begin()), n*sizeof(T));
    }

    void WriteDirect(xStream& stm)
    {
        int n = (int)Count();
        stm.WriteInt32(n);
        if (n > 0)
            stm.Write(static_cast<void*>(&*begin()), n*sizeof(T));
    }*/

    ///*
    //*    array serialize function : serialize array, call T::read & T::write
    //*/
    //void Read(xStream& stm)
    //{
    //    int n;
    //    stm >> n;
    //    resize(n);
    //    for(iterator p = begin(); n > 0; n--, p++)
    //    {
    //        p->Read(stm);
    //    }
    //}

    //void Write(xStream& stm)
    //{
    //    int n = Count();
    //    stm << n;
    //    for(iterator p = begin(); n > 0; n--, p++)
    //    {
    //        p->Write(stm);
    //    }
    //}
};

/******************************************************************
*
* template PtrArray
* �ڲ�ά��ָ��ռ䣬ɾ������Ԫ�ص�ʱ����Զ�ɾ��ָ���ָ��ָ��ռ�
*
******************************************************************/
template<class T>
class PtrArray : public xArray<T*>
{
public:
    typedef T    ValueType;

    /*
    *    ɾ��ָ��Ԫ��(����ָ������)
    * @param isSetNullOnly : ��������ֱ��ɾ��Ԫ�أ�������ʱ����ΪNULL
    *                         �������ΪNULL�󣬿���ͳһ����purge()������ΪNULL��Ԫ�ش�������ɾ��
    */

    void RemoveAt(int index, BOOL isSetNullOnly = false)
    {
        delete (*this)[index];
        if(isSetNullOnly)
            (*this)[index] = NULL;
        else
            erase(begin() + index);
    }

    void Release()
    {
        int count = Count();
        for (int i = 0; i < count; i++)
        {
            T* ptr = (*this)[i];
            delete ptr;
        }
    }
};

class Ints : public xArray<int>
{
public:
    void Sort()        {  std::sort(begin(), end()); }
    void SortDesc()    {  std::sort(begin(), end(), _greator); }

    static BOOL _greator(int e, int arg)    {  return arg < e; }
};

class Doubles : public xArray<double>
{
public:
    void Sort()        {  std::sort(begin(), end()); }
    void SortDesc()    {  std::sort(begin(), end(), _greator); }

    static BOOL _greator(double e, double arg)    {  return arg < e; }
};