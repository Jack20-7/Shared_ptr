#include <iostream>
#include <mutex>
#include <utility>

using namespace std;

class Counter{
public:
   Counter() :m_counter{0}{}

   //禁止进行拷贝操作
   Counter(const Counter&) = delete;
   Counter& operator= (Counter& ) = delete;

   ~Counter(){}

   void reset(){ m_counter = 0;}

   unsigned int get()const{ return m_counter; }

   //重载++
   void operator++(){ ++m_counter; }
   void operator++(int){ ++m_counter; }

   //重载--
   void operator--() { --m_counter; }
   void operator--(int) { --m_counter; }
private:
   unsigned int m_counter;
};

template<typename T>
class Shared_ptr{
public:
     explicit Shared_ptr(T* ptr = nullptr)
        :m_ptr{ptr},m_pCounter{new Counter()},m_pMutex{new std::mutex}{
            if(ptr){
                AddRefCount();
            }
    }
    Shared_ptr(Shared_ptr<T>& sp){
        m_ptr = sp.m_ptr;
        m_pCounter = sp.m_pCounter;
        m_pMutex = sp.m_pMutex;
        AddRefCount();
    }

    Shared_ptr<T>& operator= (const Shared_ptr<T>& rhv){
        if(m_ptr != rhv.m_ptr){
            //如果不是自我赋值的话
            Realse();
            m_ptr = rhv.m_ptr;
            m_pCounter = rhv.m_pCounter;
            m_pMutex = rhv.m_pMutex;
            AddRefCount();
        }
        return *this;
    }

    T* operator->(){
        return m_ptr;
    }
    T& operator*(){
        return *m_ptr;
    }

    T* get(){
        return m_ptr;
    }

    unsigned int UseCount(){
        return m_pCounter->get();
    }

    ~Shared_ptr(){
        Realse();
    }

private:
     void AddRefCount(){
        m_pMutex->lock();
        (*m_pCounter)++;
        m_pMutex->unlock();
     }

     void Realse(){
        bool deleteFlag = false;
        m_pMutex->lock();
        --(*m_pCounter);
        if(m_pCounter->get() == 0){
            //如果引用计数 == 0
            delete m_ptr;
            delete m_pCounter;
            deleteFlag = true;
        }
        m_pMutex->unlock();
        if(deleteFlag == true){
            delete m_pMutex;
        }
     }

     T* m_ptr;
     Counter* m_pCounter;
     std::mutex* m_pMutex;  //为了保证线程安全而引入的锁
};




int main(int argc,char** argv){
    Shared_ptr<int> s1(new int);
    *s1 = 5;
    cout << s1.UseCount() << endl; // 1
    {
       Shared_ptr<int> s2(s1);
       cout << s2.UseCount() << endl; //2
    }
    cout << s1.UseCount() << endl; // 1
    
    {
        Shared_ptr<int> s3;
        s3 = s1;
        cout << s3.UseCount() << endl;
    }

    cout << s1.UseCount() << endl;
    cout << *s1 << endl;
    return 0;
}