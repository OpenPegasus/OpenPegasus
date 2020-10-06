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


#include "CMPI_Version.h"

#include <Pegasus/Common/Stack.h>
#include <Pegasus/WQL/WQLParser.h>
#include <Pegasus/WQL/WQLSelectStatementRep.h>
#include <Pegasus/Common/Tracer.h>

#include "CMPI_Wql2Dnf.h"

PEGASUS_USING_STD;
PEGASUS_NAMESPACE_BEGIN

//
// Terminal element methods
//
void term_el_WQL::negate(void)
{
    switch( op )
    {
        case WQL_EQ: op = WQL_NE;
            break;
        case WQL_NE: op = WQL_EQ;
            break;
        case WQL_LT: op = WQL_GE;
            break;
        case WQL_LE: op = WQL_GT;
            break;
        case WQL_GT: op = WQL_LE;
            break;
        case WQL_GE: op = WQL_LT;
            break;
        default:
            break;
    }
};

static bool operator==(const WQLOperand& x, const WQLOperand& y)
{
    if( x.getType()==y.getType() )
    {
        switch( x.getType() )
        {
            case WQLOperand::PROPERTY_NAME:
                return x.getPropertyName()==y.getPropertyName();
            case WQLOperand::INTEGER_VALUE:
                return x.getIntegerValue()==y.getIntegerValue();
            case WQLOperand::DOUBLE_VALUE:
                return x.getDoubleValue()==y.getDoubleValue();
            case WQLOperand::BOOLEAN_VALUE:
                return x.getBooleanValue()==y.getBooleanValue();
            case WQLOperand::STRING_VALUE:
                return x.getStringValue()==y.getStringValue();
            case WQLOperand::NULL_VALUE:
                return true;
        }
    }
    return false;
}

static bool operator==(const term_el_WQL& x, const term_el_WQL& y)
{
    return x.op == y.op &&
    x.opn1 == y.opn1 &&
    x.opn2 == y.opn2;
}

static void addIfNotExists(TableauRow_WQL &tr, const term_el_WQL& el)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_Wql2Dnf:addIfNotExists()");
    for( int i=0,m=tr.size(); i<m; i++ )
    {
        if( tr[i]==el )
        {
            PEG_METHOD_EXIT();
            return;
        }
    }
    tr.append(el);
    PEG_METHOD_EXIT();
}

CMPI_Wql2Dnf::CMPI_Wql2Dnf(const String &condition, const String &pref)
{
    WQLSelectStatement wqs;
    WQLParser::parse(pref+condition,wqs);
    eval_heap.reserveCapacity(16);
    terminal_heap.reserveCapacity(16);
    _tableau.clear();
    compile(&wqs);
}

CMPI_Wql2Dnf::CMPI_Wql2Dnf()
{
    eval_heap.reserveCapacity(16);
    terminal_heap.reserveCapacity(16);
    _tableau.clear();
}

CMPI_Wql2Dnf::~CMPI_Wql2Dnf()
{
}

void CMPI_Wql2Dnf::compile(const WQLSelectStatement * wqs)
{
    PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPI_Wql2Dnf::compile()");
    if( !wqs->hasWhereClause() )
    {
        PEG_METHOD_EXIT();
        return;
    }
    _tableau.clear();

    _buildEvalHeap(wqs);
    _pushNOTDown();
    _factoring();

    Array<CMPI_stack_el> disj;
    _gatherDisj(disj);
    if( disj.size() == 0 )
    {
        if( terminal_heap.size() > 0 )
        {
            // point to the remaining terminal element
            disj.append(CMPI_stack_el(0,true));
        }
    }

    for( Uint32 i=0, n =disj.size(); i< n; i++ )
    {
        TableauRow_WQL tr;
        Array<CMPI_stack_el> conj;

        if( !disj[i].is_terminal )
        {
            _gatherConj(conj, disj[i]);
            for( Uint32 j=0, m = conj.size(); j < m; j++ )
            {
                addIfNotExists(tr,terminal_heap[conj[j].opn]);
//               tr.append(terminal_heap[conj[j].opn]);
            }
        }
        else
        {
            addIfNotExists(tr,terminal_heap[disj[i].opn]);
//          tr.append(terminal_heap[disj[i].opn]);
        }
        _tableau.append(tr);
    }

    eval_heap.clear();

    _populateTableau();
    PEG_METHOD_EXIT();
}

String WQL2String(const WQLOperand &o)
{
    switch( o.getType() )
    {
        case WQLOperand::PROPERTY_NAME:
            return o.getPropertyName();
        case WQLOperand::STRING_VALUE:
            return o.getStringValue();
        case WQLOperand::INTEGER_VALUE:
            return Formatter::format("$0",o.getIntegerValue());
        case WQLOperand::DOUBLE_VALUE:
            return Formatter::format("$0",o.getDoubleValue());
        case WQLOperand::BOOLEAN_VALUE:
            return Formatter::format("$0",o.getBooleanValue());
        default: ;
    }
    return "NULL_VALUE";
}

CMPIPredOp WQL2PredOp(const WQLOperation &op)
{
    PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE,"CMPI_Wql2Dnf:WQL2PredOp()");
    static CMPIPredOp ops[]={(CMPIPredOp)0,(CMPIPredOp)0,(CMPIPredOp)0,
        CMPI_PredOp_Equals,
        CMPI_PredOp_NotEquals,
        CMPI_PredOp_LessThan,
        CMPI_PredOp_LessThanOrEquals,
        CMPI_PredOp_GreaterThan,
        CMPI_PredOp_GreaterThanOrEquals,
        (CMPIPredOp)0,(CMPIPredOp)0,(CMPIPredOp)0,
        (CMPIPredOp)0,(CMPIPredOp)0,(CMPIPredOp)0};
    PEG_METHOD_EXIT();
    return ops[(int)op];
}

CMPI_QueryOperand::Type WQL2Type(WQLOperand::Type typ)
{
    switch( typ )
    {
        case WQLOperand::PROPERTY_NAME:
            return CMPI_QueryOperand::PROPERTY_TYPE;
        case WQLOperand::STRING_VALUE:
            return CMPI_QueryOperand::STRING_TYPE;
        case WQLOperand::INTEGER_VALUE:
            return CMPI_QueryOperand::UINT64_TYPE;
        case WQLOperand::DOUBLE_VALUE:
            return CMPI_QueryOperand::REAL_TYPE;
        case WQLOperand::BOOLEAN_VALUE:
            return CMPI_QueryOperand::BOOLEAN_TYPE;
        case WQLOperand::NULL_VALUE:
            return CMPI_QueryOperand::NULL_TYPE;
    }
    return CMPI_QueryOperand::NULL_TYPE;
}


void CMPI_Wql2Dnf::_populateTableau(void)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_Wql2Dnf::_populateTableau()");
    for( Uint32 i=0,n = _tableau.size(); i < n; i++ )
    {
        TableauRow_WQL tr_wql = _tableau[i];

        CMPI_TableauRow tr;
        for( Uint32 j=0,m = tr_wql.size(); j < m; j++ )
        {
            term_el_WQL t = tr_wql[j];

            CMPI_QueryOperand lhs(WQL2String(t.opn1),
            WQL2Type(t.opn1.getType()));
            CMPI_QueryOperand rhs(WQL2String(t.opn2),
            WQL2Type(t.opn2.getType()));

            tr.append(CMPI_term_el(t.mark, WQL2PredOp(t.op), lhs, rhs));

        }
        _CMPI_tableau.append (tr);
    }
    PEG_METHOD_EXIT();
}
void CMPI_Wql2Dnf::_buildEvalHeap(const WQLSelectStatement * wqs)
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_Wql2Dnf::_buildEvalHeap()");
    //WQLSelectStatement* that = (WQLSelectStatement*)wqs;

    WQLSelectStatementRep* wqsrep = wqs->_rep;

    WQLOperand dummy;
    dummy.clear();
    Stack<CMPI_stack_el> stack;

    // Counter for Operands

    Uint32 j = 0;

    //cerr << "Build eval heap\n";

    for( Uint32 i = 0, n = wqsrep->_operations.size(); i < n; i++ )
    {
        WQLOperation op = wqsrep->_operations[i];

        switch( op )
        {
            case WQL_OR:
            case WQL_AND:
                {
                    PEGASUS_ASSERT(stack.size() >= 2);

                    CMPI_stack_el op1 = stack.top();
                    stack.pop();

                    CMPI_stack_el op2 = stack.top();

                    // generate Eval expression
                    eval_heap.append(CMPI_eval_el(
                                         0, op , op1.opn,
                                         op1.is_terminal,op2.opn ,
                                         op2.is_terminal));

                    stack.top() = CMPI_stack_el(eval_heap.size()-1, false);

                    break;
                }

            case WQL_NOT:
            case WQL_IS_FALSE:
            case WQL_IS_NOT_TRUE:
                {
                    PEGASUS_ASSERT(stack.size() >= 1);

                    CMPI_stack_el op1 = stack.top();

                    // generate Eval expression
                    eval_heap.append(CMPI_eval_el(
                                         0, op , op1.opn,
                                         op1.is_terminal,-1, true));

                    stack.top() = CMPI_stack_el(eval_heap.size()-1, false);

                    break;
                }

            case WQL_EQ:
            case WQL_NE:
            case WQL_LT:
            case WQL_LE:
            case WQL_GT:
            case WQL_GE:
                {
                    PEGASUS_ASSERT(wqsrep->_operands.size() >= 2);

                    WQLOperand lhs = wqsrep->_operands[j++];

                    WQLOperand rhs = wqsrep->_operands[j++];

                    terminal_heap.push(term_el_WQL(false, op, lhs, rhs));

                    stack.push(CMPI_stack_el(terminal_heap.size()-1, true));

                    break;
                }

            case WQL_IS_TRUE:
            case WQL_IS_NOT_FALSE:
                {
                    PEGASUS_ASSERT(stack.size() >= 1);
                    break;
                }

            case WQL_IS_NULL:
                {
                    PEGASUS_ASSERT(wqsrep->_operands.size() >= 1);
                    WQLOperand operand = wqsrep->_operands[j++];

                    terminal_heap.push(
                        term_el_WQL(false, WQL_EQ, operand, dummy));

                    stack.push(CMPI_stack_el(terminal_heap.size()-1, true));

                    break;
                }

            case WQL_IS_NOT_NULL:
                {
                    PEGASUS_ASSERT(wqsrep->_operands.size() >= 1);
                    WQLOperand operand = wqsrep->_operands[j++];

                    terminal_heap.push(
                        term_el_WQL(false, WQL_NE, operand, dummy));

                    stack.push(CMPI_stack_el(terminal_heap.size()-1, true));

                    break;
                }
        }
    }

    PEGASUS_ASSERT(stack.size() == 1);
    PEG_METHOD_EXIT();
}

void CMPI_Wql2Dnf::_pushNOTDown()
{
    PEG_METHOD_ENTER(
        TRC_CMPIPROVIDERINTERFACE,
        "CMPI_Wql2Dnf::_pushNOTDown()");
    for( int i=eval_heap.size()-1; i >= 0; i-- )
    {
        Boolean _found = false;

        // Order all operators, so that op1 > op2 for non-terminals
        // and terminals appear as second operand

        eval_heap[i].order();

        // First solve the unary NOT operator

        if( eval_heap[i].op == WQL_NOT ||
        eval_heap[i].op == WQL_IS_FALSE ||
        eval_heap[i].op == WQL_IS_NOT_TRUE )
        {
            // This serves as the equivalent of an empty operator
            eval_heap[i].op = WQL_IS_TRUE;

            // Substitute this expression in all higher order eval statements
            // so that this node becomes disconnected from the tree

            for( int j=eval_heap.size()-1; j > i;j-- )
            {
                // Test first operand
                if( (!eval_heap[j].is_terminal1) && (eval_heap[j].opn1 == i) )
                {

                    eval_heap[j].assign_unary_to_first(eval_heap[i]);
                }

                // Test second operand
                if( (!eval_heap[j].is_terminal2) && (eval_heap[j].opn2 == i) )
                {

                    eval_heap[j].assign_unary_to_second(eval_heap[i]);
                }
            }

            // Test: Double NOT created by moving down

            if( eval_heap[i].mark )
            {
                eval_heap[i].mark = false;
            }
            else
            {
                _found = true;
            }
            // else indicate a pending NOT to be pushed down further
        }

        // Simple NOT created by moving down

        if( eval_heap[i].mark )
        {
            // Remove the mark, indicate a pending NOT to be pushed down
            // further and switch operators (AND / OR)

            eval_heap[i].mark=false;
            if( eval_heap[i].op == WQL_OR )
            {
                eval_heap[i].op = WQL_AND;
            }
            else
            {
                if( eval_heap[i].op == WQL_AND )
                {
                    eval_heap[i].op = WQL_OR;
                }
            }

            // NOT operator is already ruled out
            _found = true;
        }

        // Push a pending NOT further down
        if( _found )
        {
            // First operand

            int j = eval_heap[i].opn1;
            if( eval_heap[i].is_terminal1 )
            {
                // Flip NOT mark
                terminal_heap[j].negate();
            }
            else
            {
                eval_heap[j].mark = !(eval_heap[j].mark);
            }

            //Second operand (if it exists)

            if( (j = eval_heap[i].opn2) >= 0 )
            {
                if( eval_heap[i].is_terminal2 )
                {
                    // Flip NOT mark
                    terminal_heap[j].negate();
                }
                else
                {
                    eval_heap[j].mark = !(eval_heap[j].mark);
                }
            }
        }
    }
    PEG_METHOD_EXIT();
}

void CMPI_Wql2Dnf::_factoring(void)
{
    PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPI_Wql2Dnf::_factoring()");
    int i = 0,n = eval_heap.size();
    //for (int i=eval_heap.size()-1; i >= 0; i--)
    while( i < n )
    {
        int _found = 0;
        int index = 0;

        // look for expressions (A | B) & C  ---> A & C | A & B
        if( eval_heap[i].op == WQL_AND )
        {
            if( !eval_heap[i].is_terminal1 )
            {
                index = eval_heap[i].opn1; // remember the index
                if( eval_heap[index].op == WQL_OR )
                {
                    _found = 1;
                }
            }
            if( (_found == 0) && (!eval_heap[i].is_terminal2) )
            {
                    index = eval_heap[i].opn2; // remember the index
                    if( eval_heap[index].op == WQL_OR )
                    {
                        _found = 2;
                    }
            }

            if( _found != 0 )
            {
                //int u1,u1_t,u2,u2_t,u3,u3_t;
                CMPI_stack_el s;

                if( _found == 1 )
                {
                    s = eval_heap[i].getSecond();
                }
                else
                {
                    s = eval_heap[i].getFirst();
                }

                // insert two new expression before entry i
                CMPI_eval_el evl;

                evl = CMPI_eval_el(false, WQL_OR, i+1, false, i, false);
                if( (Uint32 )i < eval_heap.size()-1 )
                {
                    eval_heap.insert(i+1, evl);
                }
                else
                {
                     eval_heap.append(evl);
                }
                eval_heap.insert(i+1, evl);

                for( int j=eval_heap.size()-1; j > i + 2; j-- )
                {
                    //eval_heap[j] = eval_heap[j-2];

                    // adjust pointers

                    if( (!eval_heap[j].is_terminal1)&&
                        (eval_heap[j].opn1 >= i) )
                    {
                        eval_heap[j].opn1 += 2;
                    }
                    if( (!eval_heap[j].is_terminal2)&&
                        (eval_heap[j].opn2 >= i) )
                    {
                        eval_heap[j].opn2 += 2;
                    }
                }

                n+=2; // increase size of array

                // generate the new expressions : new OR expression

                // first new AND expression
                eval_heap[i+1].mark = false;
                eval_heap[i+1].op = WQL_AND;
                eval_heap[i+1].setFirst(s);
                eval_heap[i+1].setSecond( eval_heap[index].getFirst());
                eval_heap[i+1].order();

                // second new AND expression
                eval_heap[i].mark = false;
                eval_heap[i].op = WQL_AND;
                eval_heap[i].setFirst(s);
                eval_heap[i].setSecond( eval_heap[index].getSecond());
                eval_heap[i].order();

                // mark the indexed expression as inactive
                //eval_heap[index].op = WQL_IS_TRUE; possible disconnects
                i--;

            } /* endif _found > 0 */

        } /* endif found AND operator */

        i++; // increase pointer
    } // end of while loop

    PEG_METHOD_EXIT();
}
    void CMPI_Wql2Dnf::_gatherDisj(Array<CMPI_stack_el>& stk)
    {
        _gather(stk, CMPI_stack_el(0,true), true);
    }

    void CMPI_Wql2Dnf::_gatherConj(
        Array<CMPI_stack_el>& stk,
        CMPI_stack_el sel)
    {
        _gather(stk, sel, false);
    }

    void CMPI_Wql2Dnf::_gather(
        Array<CMPI_stack_el>& stk,
        CMPI_stack_el sel,
        Boolean or_flag)
    {
        PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPI_Wql2Dnf::_gather()");
        Uint32 i = 0;

        stk.clear();
        stk.reserveCapacity(16);

        if( (i = eval_heap.size()) == 0 )
        {
            PEG_METHOD_EXIT();
            return;
        }

        while( eval_heap[i-1].op == WQL_IS_TRUE )
        {
            eval_heap.remove(i-1);
            i--;
            if( i == 0 )
            {
               PEG_METHOD_EXIT();
               return;
            }
        }
        //if (i == 0) return;

        if( or_flag )
        {
            stk.append(CMPI_stack_el(i-1,false));
        }
        else
        {
            if( sel.is_terminal )
            {
                PEG_METHOD_EXIT();
                return;
            }
            stk.append(sel);
        }

        i = 0;

        while( i<stk.size() )
        {
            int k = stk[i].opn;

            if( (k < 0) || (stk[i].is_terminal) )
            {
                i++;
            }
            else
            {
                if ( ((eval_heap[k].op != WQL_OR) && (or_flag)) ||
                     ((eval_heap[k].op != WQL_AND) && (!or_flag)) )
                {
                    i++;
                }
                else
                {
                    // replace the element with disjunction
                    stk[i] = eval_heap[k].getSecond();
                    stk.insert(i, eval_heap[k].getFirst());
                    if (or_flag)
                    {
                      eval_heap[k].op = WQL_IS_TRUE;
                    }
                }
            }
        }
        PEG_METHOD_EXIT();
    }

//=============================================================================
//
// class CMPI_stack_el
//
//=============================================================================

//=============================================================================
//
// class CMPI_eval_el
//
//=============================================================================

    CMPI_stack_el CMPI_eval_el::getFirst()
    {
        return CMPI_stack_el(opn1, is_terminal1);
    }

    CMPI_stack_el CMPI_eval_el::getSecond()
    {
        return CMPI_stack_el(opn2, is_terminal2);
    }

    void CMPI_eval_el::setFirst(const CMPI_stack_el s)
    {
        opn1 = s.opn;
        is_terminal1 = s.is_terminal;
    }

    void CMPI_eval_el::setSecond(const CMPI_stack_el s)
    {
        opn2 = s.opn;
        is_terminal2 = s.is_terminal;
    }

    void CMPI_eval_el::assign_unary_to_first(const CMPI_eval_el & assignee)
    {
        opn1 = assignee.opn1;
        is_terminal1 = assignee.is_terminal1;
    }

    void CMPI_eval_el::assign_unary_to_second(const CMPI_eval_el & assignee)
    {
        opn2 = assignee.opn1;
        is_terminal2 = assignee.is_terminal1;
    }

// Ordering operators, so that op1 > op2 for all non-terminals
// and terminals appear in the second operand first
void CMPI_eval_el::order(void)
{
    PEG_METHOD_ENTER(TRC_CMPIPROVIDERINTERFACE, "CMPI_eval_el::order()");
    int k;
    if( (!is_terminal1) && (!is_terminal2) )
    {
        if( (k = opn2) > opn1 )
        {
            opn2 = opn1;
            opn1 =  k;
        }
    }
    else if ((is_terminal1) && (!is_terminal2))
    {
        if ((k = opn2) > opn1)
        {
            opn2 = opn1;
            opn1 =  k;
            is_terminal1 = false;
            is_terminal2 = true;
        }
    }
       PEG_METHOD_EXIT();
}

PEGASUS_NAMESPACE_END
