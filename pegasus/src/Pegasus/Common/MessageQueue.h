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
//%////////////////////////////////////////////////////////////////////////////

#ifndef Pegasus_MessageQueue_h
#define Pegasus_MessageQueue_h

#include <Pegasus/Common/Config.h>
#include <Pegasus/Common/Message.h>
#include <Pegasus/Common/Mutex.h>
#include <Pegasus/Common/List.h>
#include <Pegasus/Common/Linkage.h>

PEGASUS_NAMESPACE_BEGIN

/** The MessageQueue class represents a queue abstraction and is used by
    modules to exchange messages. Methods are provided for enqueuing,
    dequeuing, removing, iterating messages. Some methods are virtual and
    may be overriden by subclasses to modify the behavior.

    <h1>A Word about using the find() Methods</h1>

    There are two find() methods. One that takes a queue id and one that
    takes a name. The time complexity of the former is O(1); whereas, the
    time complexity of the latter is O(n). Therefore, use the queue id form
    since it is more efficient.
*/
class PEGASUS_COMMON_LINKAGE MessageQueue
{
public:

    /** This constructor places this object on a queue table which is
    maintained by this class. Each message queue has a queue-id (which
    may be obtained by calling getQueueId()). The queue-id may be passed
    to lookupQueue() to obtain a pointer to the corresponding queue).
    */
    MessageQueue(const char *name);

    /** Removes this queue from the queue table. */
    virtual ~MessageQueue();

    /** Enques a message (places it at the back of the queue).
    @param message pointer to message to be enqueued.
    @exception  NullPointer exception if message parameter is null.
    */
    virtual void enqueue(Message* message);

    /** Dequeues a message (removes it from the front of the queue).
    @return pointer to message or zero if queue is empty.
    */
    virtual Message* dequeue();

    /**
        This function will indicate whether the MessageQueue is active or not. 
        By default this function will return true and do nothing else.
    */
    virtual Boolean isActive();

    /** Returns true if there are no messages on the queue. */
    Boolean isEmpty() const { return  (Boolean) _messageList.is_empty(); }

    /** Returns the number of messages on the queue. */
    Uint32 getCount() const {  return (Uint32) _messageList.size(); }

    /** Retrieve the queue id for this queue. */
    Uint32 getQueueId() const throw() { return _queueId; }

    /** Provide a string name for this queue to be used by the print method.
     */
    const char* getQueueName() const;

    /** This method is called after a message has been enqueued. This default
    implementation does nothing. Derived classes may override this to
    take some action each time a message is enqueued (for example, this
    method could handle the incoming message in the thread of the caller
    of enqueue()).
    */
    virtual void handleEnqueue() ;

    /** Lookup a message queue from a queue id. Note this is an O(1) operation.
     */
    static MessageQueue* lookup(Uint32 queueId);

    /** Lookup a message given a queue name. NOte this is an O(N) operation.
     */
    static MessageQueue* lookup(const char *name);

    /** Get the next available queue id. It always returns a non-zero
    queue id an monotonically increases and finally wraps (to one)
    after reaching the maximum unsigned 32 bit integer.
    */
    static Uint32 getNextQueueId();

    /** Put the queue id into the stack.
    */
    static void putQueueId(Uint32 queueId);

protected:
    Uint32 _queueId;
    char *_name;

private:
    List<Message, Mutex> _messageList;
};

PEGASUS_NAMESPACE_END

#endif /* Pegasus_MessageQueue_h */
