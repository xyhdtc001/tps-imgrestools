#pragma once

#include <basecode/ref_ptr.h>

template<typename T>
class SingleQueue
{
public:
    typedef ref_ptr<T>    DataRef;

private:
    SingleQueue<T>& operator =(const SingleQueue<T>&) {}

    struct DataPlus;
    typedef ref_ptr<DataPlus>   DataPlusRef;
	class DataPlus : public CReferenced
    {
	public:
        DataRef         data;
        bool            hasRead;
        volatile bool   hasNext;
        DataPlusRef     next;
        DataPlus( DataRef dataRef ) : data(dataRef), hasRead(false), hasNext(false) {}
        DataPlus() : hasRead(true), hasNext(false) {}
    };
    DataPlusRef         m_write;
    DataPlusRef         m_read;
    unsigned int        m_writeCount;
    unsigned int        m_readCount;
    const unsigned int  m_maxSize;

public:
    SingleQueue(unsigned int queueLimit = 5000) : m_maxSize( queueLimit )
    {
        m_writeCount = m_readCount = 0;
        m_read = new DataPlus();
        m_write = m_read;
    }
    SingleQueue(SingleQueue& que) : m_maxSize(que.m_maxSize)
    {
        m_writeCount    = que.m_writeCount;
        m_readCount     = que.m_readCount;
        m_read.swap( que.m_read );
        m_write.swap( que.m_write );
        que.m_readCount = que.m_writeCount;
    }

    bool Write( DataRef dataRef )
    {
        if ( Size()>m_maxSize )
            return false;

        DataPlusRef plusRef( new DataPlus(dataRef) );
        if ( !plusRef )
        {
            throw "DataPlus::Write: new DataPlus false";
        }

        volatile bool& flag = m_write->hasNext;
        m_write->next       = plusRef;
        m_write             = plusRef;  // release
        ++m_writeCount;
        flag                = true;     // keyword
        return true;
    }

    DataRef Read()
    {
        if ( !m_read->hasRead )
        {
            m_read->hasRead = true;
            ++m_readCount;
            return m_read->data;
        }

        if ( m_read->hasNext )      // keyword
        {
            m_read = m_read->next;  // release
            return Read();
        }

        return DataRef();
    }

    unsigned int Size() { return (m_writeCount-m_readCount); }
};


//////////////////////////////////////////////////////////////////////////
// ����Ϊ�����õĴ���
//////////////////////////////////////////////////////////////////////////

// struct tt
// {
//     int aa;
//     int bb;
// };
// typedef boost::shared_ptr<tt> ttRef;
// 
// static SingleQueue<tt> sq;
// 
// inline void th11()
// {
//     while(true)
//     {
//         for (int i=0;i<1000;i++)
//         {
//             ttRef pr(new tt);
//             pr->aa = 1;
//             pr->bb = 2;
//             sq.Write( pr );
//         }
//         printf("fuck\n");
//         Sleep(1000);
//         if ( sq.Size()!=0 )
//         {
//             assert(0);
//             printf("error!!!!!!!!!!!");
//         }
//     }
// }
// 
// inline void th22()
// {
//     while(true)
//     {
//         ttRef r = sq.Read();
//         if  (r)
//         {
//             if (r->aa != 1 || r->bb!=2)
//             {
//                 printf("!@#!#!@#!@#@!#!@\n");
//             }
//         }
//     }
// }
// 
// inline void sqtest()
// {
//     boost::thread th1(th11);
//     boost::thread th2(th22);
//     th1.join();
//     th2.join();
// }

