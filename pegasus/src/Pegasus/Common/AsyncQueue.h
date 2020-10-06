//%LICENSE////////////////////////////////////////////////////////////////
//
// Licensed to The Open Group (TOG) under one or more contributor license
// agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
// this work for additional information regarding copyright ownership.
// Each contributor licenses this file to you under the OpenPegasus Open
// Source License; you may not use this file except in compliance with the
// License.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////
//
//%/////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_AsyncQueue_h
#define Pegasus_AsyncQueue_h

#include <Pegasus/Common/Linkage.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/Common/Condition.h>

PEGASUS_NAMESPACE_BEGIN

/** AsyncQueue implementation (formerly AsyncDQueue).
*/
template<class ElemType>
class AsyncQueue
{
public:

    /** Constructor.
    */
    AsyncQueue();

    /** Destructor.
    */
    virtual ~AsyncQueue();

    /** Close the queue.
    */
    void close();

    /** Enqueue an element at the back of queue.
        @param element The element to enqueue.
        @return True if the element is successfully enqueued, false if the
            queue is closed.
    */
    Boolean enqueue(ElemType *element);

    /** Dequeue an element from the front of the queue. Return null immediately
        if queue is empty or closed.
        @return A pointer to the element that was dequeued, or null if the
            queue is empty or closed.
    */
    ElemType *dequeue();

    /** Dequeue an element from the front of the queue (wait if the queue is
        empty).
        @return A pointer to the element that was dequeued, or null if the
            queue is closed (either before or while waiting for an element).
    */
    ElemType *dequeue_wait();

    /** Discard all the elements on the list. The size becomes zero afterwards.
    */
    void clear();

    /** Return number of element in queue.
    */
    Uint32 count() const { return _rep.size(); }

    /** Return true is queue is empty (has zero elements).
    */
    Boolean is_empty() const { return _rep.size() == 0; }

    /** Return true if the queue has been closed (in which case no new
        elements may be enqueued).
    */
    Boolean is_closed() const { return _closed.get(); }

private:

    Mutex _mutex;
    Condition _not_empty;
    AtomicInt _closed;
    typedef List<ElemType,NullLock> Rep;
    Rep _rep;
};

template<class ElemType>
AsyncQueue<ElemType>::AsyncQueue() :
    _mutex(Mutex::NON_RECURSIVE)
{
}

template<class ElemType>
AsyncQueue<ElemType>::~AsyncQueue()
{
}

template<class ElemType>
void AsyncQueue<ElemType>::close()
{
    AutoMutex auto_mutex(_mutex);

    if (!is_closed())
    {
        _closed++;
        _not_empty.signal();
    }
}

template<class ElemType>
Boolean AsyncQueue<ElemType>::enqueue(ElemType *element)
{
    if (element)
    {
        AutoMutex auto_mutex(_mutex);

        if (is_closed())
        {
            return false;
        }

        _rep.insert_back(element);
        _not_empty.signal();
    }

    return true;
}

template<class ElemType>
void AsyncQueue<ElemType>::clear()
{
    AutoMutex auto_mutex(_mutex);
    _rep.clear();
}

template<class ElemType>
ElemType* AsyncQueue<ElemType>::dequeue()
{
    AutoMutex auto_mutex(_mutex);

    if (is_closed())
    {
        return 0;
    }

    return _rep.remove_front();
}

template<class ElemType>
ElemType* AsyncQueue<ElemType>::dequeue_wait()
{
    AutoMutex auto_mutex(_mutex);

    while (is_empty())
    {
        if (is_closed())
        {
            return 0;
        }

        _not_empty.wait(_mutex);
    }

    if (is_closed())
    {
        return 0;
    }

    ElemType* p = _rep.remove_front();
    PEGASUS_DEBUG_ASSERT(p != 0);

    return p;
}

PEGASUS_NAMESPACE_END

#endif /* Pegasus_AsyncQueue_h */
