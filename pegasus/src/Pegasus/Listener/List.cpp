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
// Author: Dong Xiang, EMC Corporation (xiang_dong@emc.com)
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////

#include "List.h"

#include <Pegasus/Common/Exception.h>


PEGASUS_NAMESPACE_BEGIN

/////////////////////////////////////////////////////////////////////////////
// ListNode
/////////////////////////////////////////////////////////////////////////////
class ListNode
{
public:
    ListNode(void* element);
    ~ListNode();

    ListNode* getNext() const;
    void setNext(ListNode* next);

    ListNode* getPrevious() const;
    void setPrevious(ListNode* previous);

    void* getElement() const;

private:
    void*           _element;
    ListNode*   _next;
    ListNode* _previous;
};

ListNode::ListNode(void* element)
:_element(element),_next(NULL),_previous(NULL)
{
}
ListNode::~ListNode()
{
}

ListNode* ListNode::getNext() const
{
    return _next;
}
void ListNode::setNext(ListNode* next)
{
    _next = next;
}

ListNode* ListNode::getPrevious() const
{
    return _previous;
}
void ListNode::setPrevious(ListNode* previous)
{
    _previous = previous;
}

void* ListNode::getElement() const
{
    return _element;
}

/////////////////////////////////////////////////////////////////////////////
// PtrListIterator
/////////////////////////////////////////////////////////////////////////////
class PtrListIterator : public Iterator
{
public:
    PtrListIterator(ListNode* first);
  ~PtrListIterator();

    Boolean hasNext();
    void*       next();
    void        remove();

private:
    ListNode* _lead;
};

PtrListIterator::PtrListIterator(ListNode* lead)
:_lead(lead)
{
}
PtrListIterator::~PtrListIterator()
{
}
Boolean PtrListIterator::hasNext()
{
    return (_lead!=NULL)?true:false;
}
void* PtrListIterator::next()
{
    if(_lead==NULL)
        throw IndexOutOfBoundsException();

    void* element = _lead->getElement();
    _lead = _lead->getNext();

    return element;
}
void PtrListIterator::remove()
{
    throw Exception("Not Supported feature");
}


/////////////////////////////////////////////////////////////////////////////
// PtrListRep
/////////////////////////////////////////////////////////////////////////////
class PtrListRep
{
public:
    PtrListRep();
  ~PtrListRep();

    void add(void* element);
    void remove(void* element);

    Iterator* iterator();

private:
    ListNode* _first;
    ListNode* _last;
};

PtrListRep::PtrListRep()
:_first(NULL),_last(NULL)
{
}
PtrListRep::~PtrListRep()
{
    // reemove all the nodes
    while(_last!=NULL)
    {
        ListNode* n = _last->getPrevious();
        delete _last;
        _last = n;
    }
}
void PtrListRep::add(void* element)
{
    ListNode* n = new ListNode(element);

    if(_last==NULL)
    { // this is the very first one
        _first = _last = n;
    }
    else { // append to the end
        _last->setNext(n);
        n->setPrevious(_last);
        _last = n;
    }
}
void PtrListRep::remove(void* element)
{
    if(element!=NULL && _first!=NULL)
    {
        for(ListNode* n=_first; n!=NULL; n=n->getNext())
        {
            void* el = n->getElement();
            if(el==element)
            { // remove the node
                ListNode* prev = n->getPrevious();
                ListNode* next = n->getNext();

                if(prev!=NULL)
                    prev->setNext(next);
                else // the node is the very first
                    _first = next;

                if(next!=NULL)
                    next->setPrevious(prev);
                else // the node is the last
                    _last = prev;

                delete n;
                break;
            }
        }
    }
}
Iterator* PtrListRep::iterator()
{
    return new PtrListIterator(_first);
}

/////////////////////////////////////////////////////////////////////////////
// List
/////////////////////////////////////////////////////////////////////////////
PtrList::PtrList()
:_rep(new PtrListRep())
{
}
PtrList::~PtrList()
{
    if(_rep!=NULL)
        delete static_cast<PtrListRep*>(_rep);
    _rep=NULL;
}

void PtrList::add(void* element)
{
    static_cast<PtrListRep*>(_rep)->add(element);
}

void PtrList::remove(void* element)
{
    static_cast<PtrListRep*>(_rep)->remove(element);
}

Iterator* PtrList::iterator()
{
    return static_cast<PtrListRep*>(_rep)->iterator();
}


PEGASUS_NAMESPACE_END
