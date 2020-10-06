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


#include "Cql2Dnf.h"
#include <Pegasus/Common/Stack.h>
#include <Pegasus/Common/Tracer.h>
#include <Pegasus/Common/PegasusAssert.h>

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

#define PEGASUS_ARRAY_T term_el
# include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T eval_el
# include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

#define PEGASUS_ARRAY_T stack_el
# include <Pegasus/Common/ArrayImpl.h>
#undef PEGASUS_ARRAY_T

//
// Terminal element methods
//

void term_el::negate()
{
   NOT = true;
}

//
// Evaluation heap element methods
//
stack_el eval_el::getFirst()
{
   return stack_el(opn1, is_terminal1);
}

stack_el eval_el::getSecond()
{
   return stack_el(opn2, is_terminal2);
}

void eval_el::setFirst(const stack_el &s)
{
     opn1 = s.opn;
     is_terminal1 = s.is_terminal;
}

void eval_el::setSecond(const stack_el &s)
{
    opn2 = s.opn;
    is_terminal2 = s.is_terminal;
}

void eval_el::assign_unary_to_first(const eval_el & assignee)
{
    opn1 = assignee.opn1;
    is_terminal1 = assignee.is_terminal1;
}

void eval_el::assign_unary_to_second(const eval_el & assignee)
{
    opn2 = assignee.opn1;
    is_terminal2 = assignee.is_terminal1;
}

// Ordering operators, so that op1 > op2 for all non-terminals
// and terminals appear in the second operand first
void eval_el::order(void)
{
    int k;
    if ((!is_terminal1) && (!is_terminal2))
    {
        if ((k = opn2) > opn1)
        {
            opn2 = opn1;
            opn1 =  k;
        }
    }
    else
    {
        if ((is_terminal1) && (!is_terminal2))
        {
            if ((k = opn2) > opn1)
            {
                opn2 = opn1;
                opn1 =  k;
                is_terminal1 = false;
                is_terminal2 = true;
            }
        }
    }
}
/*
static bool operator==(const term_el& x, const term_el& y)
{
    return x._simplePredicate == y._simplePredicate;
}
*/
//
// CQL Compiler methods
//
/*
Cql2Dnf::Cql2Dnf()
{
    eval_heap.reserveCapacity(16);
    terminal_heap.reserveCapacity(16);
}

Cql2Dnf::Cql2Dnf(CQLSelectStatement & cqs)
{
    eval_heap.reserveCapacity(16);
    terminal_heap.reserveCapacity(16);
    compile(&cqs);
}

Cql2Dnf::Cql2Dnf(CQLSelectStatement * cqs)
{
    eval_heap.reserveCapacity(16);
    terminal_heap.reserveCapacity(16);
    compile(cqs);
}
*/
Cql2Dnf::Cql2Dnf(CQLPredicate& topLevel){
    eval_heap.reserveCapacity(16);
    terminal_heap.reserveCapacity(16);
    compile(topLevel);
}

Cql2Dnf::~Cql2Dnf() {}
/*
void Cql2Dnf::compile(CQLSelectStatement * cqs){
    CQLPredicate topLevel = cqs->getPredicate();
    compile(topLevel);
}
*/
void Cql2Dnf::compile(CQLPredicate& topLevel)
{
    PEG_METHOD_ENTER(TRC_CQL, "Cql2Dnf::compile");

    _strip_ops_operands(topLevel);
    _buildEvalHeap();
    _pushNOTDown();
    _factoring();
    _construct();
    eval_heap.clear();

    PEG_METHOD_EXIT();
}

void Cql2Dnf::_buildEvalHeap()
{
    PEG_METHOD_ENTER(TRC_CQL, "Cql2Dnf::_buildEvalHeap");

    Stack<stack_el> stack;

    // Counter for Operands
    Uint32 j = 0;

    for (Uint32 i = 0, n = _operations.size(); i < n; i++)
    {
        OperationType op = _operations[i];

        switch (op)
        {
            case CQL_OR:
            case CQL_AND:
            {
                PEGASUS_ASSERT(stack.size() >= 2);

                stack_el op1 = stack.top();
                stack.pop();

                stack_el op2 = stack.top();

                // generate Eval expression
                eval_heap.append(eval_el(0, op , op1.opn, op1.is_terminal,
                                 op2.opn , op2.is_terminal));

                stack.top() = stack_el(eval_heap.size()-1, false);

                break;
            }

            case CQL_NOT:
            {
                PEGASUS_ASSERT(stack.size() >= 1);

                stack_el op1 = stack.top();

                // generate Eval expression
                eval_heap.append(eval_el(0, op , op1.opn, op1.is_terminal,
                                 -1, true));

                stack.top() = stack_el(eval_heap.size()-1, false);

                break;
            }

            case CQL_EQ:
            case CQL_NE:
            case CQL_LT:
            case CQL_LE:
            case CQL_GT:
            case CQL_GE:
            case CQL_ISA:
            case CQL_LIKE:
            {
                PEGASUS_ASSERT(_operands.size() >= 2);

                CQLExpression lhs = _operands[j++];

                CQLExpression rhs = _operands[j++];

                CQLSimplePredicate sp(lhs,rhs,_convertOpType(op));
                terminal_heap.push(term_el(false, sp));

                stack.push(stack_el(terminal_heap.size()-1, true));

                break;
            }

            case CQL_IS_NULL:
            {
                PEGASUS_ASSERT(_operands.size() >= 1);
                CQLExpression expression = _operands[j++];
                CQLSimplePredicate dummy(expression,IS_NULL);
                terminal_heap.push(term_el(false, dummy));

                stack.push(stack_el(terminal_heap.size()-1, true));

                break;
            }

            case CQL_IS_NOT_NULL:
            {
                PEGASUS_ASSERT(_operands.size() >= 1);
                CQLExpression expression = _operands[j++];
                CQLSimplePredicate dummy(expression,IS_NOT_NULL);
                terminal_heap.push(term_el(false, dummy));

                stack.push(stack_el(terminal_heap.size()-1, true));

                break;
            }
            case CQL_NOOP:
            default: break;
        }
    }

    PEGASUS_ASSERT(stack.size() == 1);

    PEG_METHOD_EXIT();
}

void Cql2Dnf::_pushNOTDown()
{
    PEG_METHOD_ENTER(TRC_CQL, "Cql2Dnf::_pushNOTDown");

    for (int i=eval_heap.size()-1; i >= 0; i--)
    {
        Boolean _found = false;

        // Order all operators, so that op1 > op2 for non-terminals
        // and terminals appear as second operand

        eval_heap[i].order();

        // First solve the unary NOT operator

        if (eval_heap[i].op == CQL_NOT)
       {
            // This serves as the equivalent of an empty operator
            eval_heap[i].op = CQL_NOOP;

            // Substitute this expression in all higher order eval statements
            // so that this node becomes disconnected from the tree

            for (int j=eval_heap.size()-1; j > i;j--)
            {
               // Test first operand
               if ((!eval_heap[j].is_terminal1) && (eval_heap[j].opn1 == i))
                   eval_heap[j].assign_unary_to_first(eval_heap[i]);

               // Test second operand
               if ((!eval_heap[j].is_terminal2) && (eval_heap[j].opn2 == i))
                   eval_heap[j].assign_unary_to_second(eval_heap[i]);
            }

            // Test: Double NOT created by moving down

            if (eval_heap[i].mark)
               eval_heap[i].mark = false;
            else
               _found = true;
            // else indicate a pending NOT to be pushed down further
        }

        // Simple NOT created by moving down

        if (eval_heap[i].mark)
        {
            // Remove the mark, indicate a pending NOT to be pushed down
            // further and switch operators (AND / OR)

            eval_heap[i].mark=false;
            if (eval_heap[i].op == CQL_OR)
                eval_heap[i].op = CQL_AND;
            else if (eval_heap[i].op == CQL_AND) eval_heap[i].op = CQL_OR;
                // NOT operator is already ruled out
                _found = true;
        }

        // Push a pending NOT further down
        if (_found)
        {
             // First operand

             int j = eval_heap[i].opn1;
             if (eval_heap[i].is_terminal1)
                 // Flip NOT mark
                 terminal_heap[j].negate();
             else
                 eval_heap[j].mark = !(eval_heap[j].mark);

             //Second operand (if it exists)

             if ((j = eval_heap[i].opn2) >= 0)
             {
                 if (eval_heap[i].is_terminal2)
                     // Flip NOT mark
                     terminal_heap[j].negate();
                 else
                     eval_heap[j].mark = !(eval_heap[j].mark);
             }
        }
    }
    PEG_METHOD_EXIT();
}

void Cql2Dnf::_factoring(void)
{
    PEG_METHOD_ENTER(TRC_CQL, "Cql2Dnf::_factoring");

    int i = 0,n = eval_heap.size();
    //for (int i=eval_heap.size()-1; i >= 0; i--)
    while (i < n)
    {
        int _found = 0;
        int index = 0;

        // look for expressions (A | B) & C  ---> A & C | A & B
        if (eval_heap[i].op == CQL_AND)
        {
            if (!eval_heap[i].is_terminal1)
            {
                index = eval_heap[i].opn1; // remember the index
                if (eval_heap[index].op == CQL_OR)
                    _found = 1;
            }

            if ((_found == 0) && (!eval_heap[i].is_terminal2))
            {
                index = eval_heap[i].opn2; // remember the index
                if (eval_heap[index].op == CQL_OR)
                    _found = 2;
            }

            if (_found != 0)
            {
                 //int u1,u1_t,u2,u2_t,u3,u3_t;
                 stack_el s;

                 if (_found == 1)
                     s = eval_heap[i].getSecond();
                 else
                     s = eval_heap[i].getFirst();

                 // insert two new expression before entry i
                 eval_el evl;

                 evl = eval_el(false, CQL_OR, i+1, false, i, false);
                 if ((Uint32 )i < eval_heap.size()-1)
                     eval_heap.insert(i+1, evl);
                 else
                     eval_heap.append(evl);
                 eval_heap.insert(i+1, evl);

                 for (int j=eval_heap.size()-1; j > i + 2; j--)
                 {
                     //eval_heap[j] = eval_heap[j-2];

                     // adjust pointers

                     if ((!eval_heap[j].is_terminal1)&&
                         (eval_heap[j].opn1 >= i))
                         eval_heap[j].opn1 += 2;
                     if ((!eval_heap[j].is_terminal2)&&
                         (eval_heap[j].opn2 >= i))
                         eval_heap[j].opn2 += 2;
                 }

                 n+=2; // increase size of array

                 // generate the new expressions : new OR expression

                 // first new AND expression
                 eval_heap[i+1].mark = false;
                 eval_heap[i+1].op = CQL_AND;
                 eval_heap[i+1].setFirst(s);
                 eval_heap[i+1].setSecond( eval_heap[index].getFirst());
                 eval_heap[i+1].order();


                 // second new AND expression
                 eval_heap[i].mark = false;
                 eval_heap[i].op = CQL_AND;
                 eval_heap[i].setFirst(s);
                 eval_heap[i].setSecond( eval_heap[index].getSecond());
                 eval_heap[i].order();

                 // mark the indexed expression as inactive
                 //eval_heap[index].op = WQL_IS_TRUE; possible disconnects
                 i--;

            } /* endif _found > 0 */

        } /* endif found AND operator */

        i++; // increase pointer
    }

    PEG_METHOD_EXIT();
}

void Cql2Dnf::_strip_ops_operands(CQLPredicate& topLevel)
{
    PEG_METHOD_ENTER(TRC_CQL, "Cql2Dnf::_strip_ops_operands");
    //
    // depth first search for all operations and operands
    // extract operations and operands and store in respective arrays
    // for later processing
    //
    _destruct(topLevel);
    if(topLevel.getInverted())
    {
        _operations.append(CQL_NOT);
    }
    PEG_METHOD_EXIT();
}

OperationType Cql2Dnf::_convertOpType(ExpressionOpType op){
    switch(op){
        case EQ: return CQL_EQ;
        case NE: return CQL_NE;
        case GT: return CQL_GT;
        case LT: return CQL_LT;
        case GE: return CQL_GE;
        case LE: return CQL_LE;
        case IS_NULL: return CQL_IS_NULL;
        case IS_NOT_NULL: return CQL_IS_NOT_NULL;
        case ISA: return CQL_ISA;
        case LIKE: return CQL_LIKE;
        default: return CQL_NOOP;
    }
}

ExpressionOpType Cql2Dnf::_convertOpType(OperationType op)
{
    switch(op)
    {
        case CQL_EQ: return EQ;
        case CQL_NE: return NE;
        case CQL_GT: return GT;
        case CQL_LT: return LT;
        case CQL_GE: return GE;
        case CQL_LE: return LE;
        case CQL_IS_NULL: return IS_NULL;
        case CQL_IS_NOT_NULL: return IS_NOT_NULL;
        case CQL_ISA: return ISA;
        case CQL_LIKE: return LIKE;
        default: break; // should never get here
    }
    PEGASUS_UNREACHABLE(PEGASUS_ASSERT(0);)
    return EQ;
}

void Cql2Dnf::_destruct(CQLPredicate& _p){
    if(_p.isSimple()){
        CQLSimplePredicate _sp = _p.getSimplePredicate();
        _operations.append(_convertOpType(_sp.getOperation()));
        _operands.append(_sp.getLeftExpression());
        if((_operations[_operations.size()-1] != CQL_IS_NULL)
            && (_operations[_operations.size()-1] != CQL_IS_NOT_NULL))
        {
            _operands.append(_sp.getRightExpression());
        }
    }
    else{
        Array<CQLPredicate> _preds = _p.getPredicates();
        Array<BooleanOpType> _boolops = _p.getOperators();
        for(Uint32 i=0;i<_preds.size();i++)
        {
            _destruct(_preds[i]);
            if(_preds[i].getInverted())
            {
                _operations.append(CQL_NOT);
            }
            if(i > 0)
            {
                if(_boolops[i-1] == AND)
                {
                    _operations.append(CQL_AND);
                }
                if(_boolops[i-1] == OR)
                {
                    _operations.append(CQL_OR);
                }
            }
        }
    }
}

void Cql2Dnf::_construct(){

    PEG_METHOD_ENTER(TRC_CQL, "Cql2Dnf::_construct");
    //
    // Each eval_el on the eval heap contains all the information needed to
    // make a CQLPredicate.
    // We will build a CQLPredicate for every element in the eval heap.
    // So there is a 1 to 1 correspondence
    // between elements in the eval heap and elements in the CQLPredicate
    // array used below.
    // The first eval_el on the eval heap will always contain at least one
    // terminal if the operation is a NOT
    // or two terminals if the operation is AND or OR.  We are guaranteed to
    // build a CQLPredicate from the first position in the eval_heap array.
    //
    // The key to the algorithm is the isterminalX flag.
    // When set to true, we go to the term_heap and get the CQLSimplePredicate.
    // When set to false, we go to the _preds array below
    // and get the CQLPredicate.  Since there is a 1 - 1 correspondence,
    // as explained above, the index
    // referred to by eval.opn1 or eval.opn2 is valid into the _preds array.
    //
    // For ANDs and ORs, we need two operands, as explained above,
    // we get those operands from either the term_heap or the _preds array.
    // For NOTs, we need only 1 operand, and that
    // comes from either the term_heap or the _preds array.
    //
    // When finished, the last element in the _preds array contains the
    // top level CQLPredicate (the rebuilt tree)
    //
    // Example:  a=b^(!c=d v e=f)
    // If the current eval_heap looks like:
    //  0,NOT,1,True,-1,True [index = 0]
    //  0,OR,2,True,0,False  [index = 1]
    //  0,AND,1,False,0,True [index = 2]
    //
    // And the current term_heap looks like:
    //  CQLSimplePredicate(a=b) [index = 0]
    //  CQLSimplePredicate(c=d) [index = 1]
    //      CQLSimplePredicate(e=f) [index = 0]
    //
    // The _preds array at the end would look like:
    //  CQLPredicate(!c==d)        [index = 0]
    //  CQLPredicate(e==f v !c==d) [index = 1]
    //  CQLPredicate((e==f v !c==d) ^ a==b) [index = 2]  (the rebuilt tree)
    //

    if(eval_heap.size() > 0){
       Array<CQLPredicate> _preds;
       CQLPredicate pred;
       for(Uint32 i=0;i<eval_heap.size();i++){
        eval_el eval = eval_heap[i];
        if(eval.is_terminal1 && eval.is_terminal2)
        {
            switch(eval.op)
            {
                case CQL_NOT:
                {
                    _preds.append(
                        CQLPredicate(terminal_heap[eval.opn1]._simplePredicate,
                        true));
                    break;
                }
                case CQL_NOOP:
                {
                    CQLPredicate p(terminal_heap[eval.opn1]._simplePredicate,
                            false);
                    if(terminal_heap[eval.opn1].NOT == true)
                       p.setInverted(true);
                    _preds.append(p);
                    break;
                }
                case CQL_AND:
                {
                    CQLPredicate p;
                    CQLPredicate p1(terminal_heap[eval.opn2]._simplePredicate,
                            false);
                    if(terminal_heap[eval.opn2].NOT == true)
                       p1.setInverted(true);
                    p.appendPredicate(p1);
                    CQLPredicate p2(terminal_heap[eval.opn1]._simplePredicate,
                            false);
                    if(terminal_heap[eval.opn1].NOT == true)
                       p2.setInverted(true);
                    p.appendPredicate(p2,AND);
                    _preds.append(p);
                    break;
                }
                case CQL_OR:
                {
                   CQLPredicate p;
                   CQLPredicate p1(terminal_heap[eval.opn2]._simplePredicate,
                       false);
                   if(terminal_heap[eval.opn2].NOT == true)
                       p1.setInverted(true);

                   p.appendPredicate(p1);
                   CQLPredicate p2(terminal_heap[eval.opn1]._simplePredicate,
                           false);
                   if(terminal_heap[eval.opn1].NOT == true)
                      p2.setInverted(true);
                   p.appendPredicate(p2,OR);
                   _preds.append(p);
                   break;
                }
                case CQL_EQ:
                case CQL_NE:
                case CQL_GT:
                case CQL_LT:
                case CQL_GE:
                case CQL_LE:
                case CQL_ISA:
                case CQL_LIKE:
                case CQL_IS_NULL:
                case CQL_IS_NOT_NULL:
                    break;
            }
        }else if(eval.is_terminal1 && !eval.is_terminal2){
            switch(eval.op)
            {
                case CQL_NOT:
                {
                   _preds.append(
                       CQLPredicate(terminal_heap[eval.opn1]._simplePredicate,
                           true));
                   break;
                }
                case CQL_NOOP:
                {
                   CQLPredicate p(terminal_heap[eval.opn1]._simplePredicate,
                                  false);
                   if(terminal_heap[eval.opn1].NOT == true)
                      p.setInverted(true);
                   _preds.append(p);
                   break;
                }
                case CQL_AND:
                    {
                       CQLPredicate p;
                       CQLPredicate p1(
                           terminal_heap[eval.opn1]._simplePredicate,
                           false);
                       if(terminal_heap[eval.opn1].NOT == true)
                       p1.setInverted(true);
                       p = _preds[eval.opn2];
                       p.appendPredicate(p1,AND);
                       _preds.append(p);
                       break;
                    }
                case CQL_OR:
                {
                  CQLPredicate p;
                  CQLPredicate p1(terminal_heap[eval.opn1]._simplePredicate,
                                  false);
                      if(terminal_heap[eval.opn1].NOT == true)
                     p1.setInverted(true);
                  p = _preds[eval.opn2];
                  p.appendPredicate(p1,OR);
                  _preds.append(p);
                  break;
                }
                case CQL_EQ:
                case CQL_NE:
                case CQL_GT:
                case CQL_LT:
                case CQL_GE:
                case CQL_LE:
                case CQL_ISA:
                case CQL_LIKE:
                case CQL_IS_NULL:
                case CQL_IS_NOT_NULL:
                    break;

            }
        }
        else if(!eval.is_terminal1 && eval.is_terminal2)
        {
         switch(eval.op)
         {
            case CQL_NOT:
            {
               CQLPredicate p = _preds[eval.opn1];
               p.setInverted(true);
               _preds.append(p);
               break;
            }
            case CQL_NOOP:
            {
               _preds.append(_preds[eval.opn1]);
               break;
            }
            case CQL_AND:
            {
               CQLPredicate p;
               CQLPredicate p1(terminal_heap[eval.opn2]._simplePredicate,
                       false);
               if(terminal_heap[eval.opn2].NOT == true)
               p1.setInverted(true);
               p = _preds[eval.opn1];
               p.appendPredicate(p1,AND);
               _preds.append(p);
               break;
            }
            case CQL_OR:
            {
               CQLPredicate p;
               CQLPredicate p1(terminal_heap[eval.opn2]._simplePredicate,
                       false);
               if(terminal_heap[eval.opn2].NOT == true)
               p1.setInverted(true);
               p = _preds[eval.opn1];
               p.appendPredicate(p1,OR);
               _preds.append(p);
               break;
            }
            case CQL_EQ:
            case CQL_NE:
            case CQL_GT:
            case CQL_LT:
            case CQL_GE:
            case CQL_LE:
            case CQL_ISA:
            case CQL_LIKE:
            case CQL_IS_NULL:
            case CQL_IS_NOT_NULL: break;

         }

         }
         else  // !eval.is_terminal1 && !eval.is_terminal2
         {
            switch(eval.op)
            {
                case CQL_NOT:
                {
                    CQLPredicate p = _preds[eval.opn1];
                    p.setInverted(true);
                    _preds.append(p);
                    break;
                }
               case CQL_NOOP:
                {
                    _preds.append(_preds[eval.opn1]);
                    break;
                }
                case CQL_AND:
                {
                   CQLPredicate p = _preds[eval.opn2];
                   _flattenANDappend(p,AND,_preds[eval.opn1]);
                   _preds.append(p);
                   break;
                }
                case CQL_OR:
                {
                   CQLPredicate p = _preds[eval.opn2];
                   _flattenANDappend(p,OR,_preds[eval.opn1]);
                   _preds.append(p);
                   break;
                }
                case CQL_EQ:
                case CQL_NE:
                case CQL_GT:
                case CQL_LT:
                case CQL_GE:
                case CQL_LE:
                case CQL_ISA:
                case CQL_LIKE:
                case CQL_IS_NULL:
                case CQL_IS_NOT_NULL:
                    break;
            }

        }
       } // end for(...)

       _dnfPredicate = _preds[_preds.size()-1];

    } // end if
    else
    { // we just have a CQLSimplePredicate on the terminal_heap
        PEGASUS_ASSERT(terminal_heap.size() == 1);
        _dnfPredicate = CQLPredicate(terminal_heap[0]._simplePredicate,false);
    }

    PEG_METHOD_EXIT();
}

CQLPredicate Cql2Dnf::getDnfPredicate()
{
    return _dnfPredicate;
}

CQLPredicate Cql2Dnf::_flattenANDappend(CQLPredicate& topLevel,
        BooleanOpType op, CQLPredicate& p)
{

    PEG_METHOD_ENTER(TRC_CQL, "Cql2Dnf::_flattenANDappend");
    //
    // this is to prevent appending complex predicates to the top level
    // predicate.
    // the final DNFed predicate must only have simple predicates inside
    // its predicate array
    //
    // example:
    // say P(top level) = A AND B
    // say P1 = C AND D
    // say we need to OR them together
    // we cant call P.appendPredicate(P1,OR) because this creates one more
    // complex predicate layer.
    // instead we would:
    // -> get P1s predicates (which should all be simple)
    // -> append its first predicate to P along with the operator passed into
    // us
    // -> at this point we have P = A AND B OR C
    // -> then go through P1s remaining predicates and append them and P1s
    // operators to P
    // -> when finished, we have P = A AND B OR C AND D INSTEAD of having
    // P = A AND B OR P1 where P1 is a complex predicate
    //

    if(!p.isSimple())
    {
        Array<CQLPredicate> preds = p.getPredicates();
        Array<BooleanOpType> ops = p.getOperators();
        for(Uint32 i=0;i<preds.size();i++)
        {
            if(i==0) topLevel.appendPredicate(preds[i],op);
            else topLevel.appendPredicate(preds[i],ops[i-1]);
        }
    }
    else
    {
        topLevel.appendPredicate(p,op);
    }

    PEG_METHOD_EXIT();
    return topLevel;
}

PEGASUS_NAMESPACE_END
