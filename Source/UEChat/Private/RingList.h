#pragma once
#include <CoreMinimal.h>

template<class T>
class TRingList
{
public:
    TRingList();
    ~TRingList();

    uint32 Capacity() const;
    uint32 Size() const;

    void PushTail(const T& X);
    void PushTail(T&& X);
    void PopHead();

    const T& Head() const;
    T& Head();
    const T& Tail() const;
    T& Tail();

private:
    TRingList(const TRingList&) = delete;
    TRingList& operator=(const TRingList&) = delete;
    void Expand();

    uint32 Capacity_ = 0;
    uint32 Size_ = 0;
    uint32 Head_ = 0;
    uint32 Tail_ = 0;
    T* Items_ = nullptr;
};

template<class T>
TRingList<T>::TRingList()
{
}

template<class T>
TRingList<T>::~TRingList()
{
    FMemory::Free(Items_);
    Items_ = nullptr;
}

template<class T>
uint32 TRingList<T>::Capacity() const
{
    return Capacity_;
}

template<class T>
uint32 TRingList<T>::Size() const
{
    return Size_;
}

template<class T>
void TRingList<T>::PushTail(const T& X)
{
    if(Capacity_ <= Size_) {
        Expand();
    }
    uint32 Index = (0 < Tail_) ? Tail_ - 1 : Capacity_ - 1;
    new(&Items_[Index]) T(X);
    ++Size_;
    ++Tail_;
    if(Capacity_ <= Tail_) {
        Tail_ = 0;
    }
}

template<class T>
void TRingList<T>::PushTail(T&& X)
{
    if(Capacity_ <= Size_) {
        Expand();
    }
    new(&Items_[Tail_]) T(MoveTemp(X));
    ++Size_;
    ++Tail_;
    if(Capacity_ <= Tail_) {
        Tail_ = 0;
    }
}

template<class T>
void TRingList<T>::PopHead()
{
    check(0 < Size_);
    Items_[Head_].~T();
    --Size_;
    ++Head_;
    if(Capacity_ <= Head_) {
        Head_ = 0;
    }
}

template<class T>
const T& TRingList<T>::Head() const
{
    check(0 < Size_);
    return Items_[Head_];
}

template<class T>
T& TRingList<T>::Head()
{
    check(0 < Size_);
    return Items_[Head_];
}

template<class T>
const T& TRingList<T>::Tail() const
{
    check(0 < Size_);
    uint32 Index = (0 < Tail_) ? Tail_ - 1 : Capacity_ - 1;
    return Items_[Index];
}

template<class T>
T& TRingList<T>::Tail()
{
    check(0 < Size_);
    uint32 Index = (0 < Tail_) ? Tail_ - 1 : Capacity_ - 1;
    return Items_[Index];
}

template<class T>
void TRingList<T>::Expand()
{
    uint32 NewCapacity = Capacity_ + 64;
    T* Items = static_cast<T*>(FMemory::Malloc(sizeof(T) * NewCapacity));
    check(nullptr != Items);
    for(uint32 i = 0; i < Size_; ++i){
        uint32 Index = (i + Head_) & 63UL;
        new(&Items[i]) T(MoveTemp(Items_[Index]));
    }
    FMemory::Free(Items_);
    Head_ = 0;
    Tail_ = Size_;
    Capacity_ = NewCapacity;
    Items_ = Items;
}
