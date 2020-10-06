/*!LICENSE
 *
 * Licensed to The Open Group (TOG) under one or more contributor license
 * agreements.  Refer to the OpenPegasusNOTICE.txt file distributed with
 * this work for additional information regarding copyright ownership.
 * Each contributor licenses this file to you under the OpenPegasus Open
 * Source License; you may not use this file except in compliance with the
 * License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

configURL = "root%2FPG_InterOp/enum?class=cim_indicationsubscription";
currentElement = null;

function insertindicationRow(info, handler, filter, state, ref) {
    var t_rows = document.getElementById('indicationTable').rows;
	

    var x=1;

    for (x=1; x < t_rows.length; x++) {
        var celValue = t_rows[x].cells[0].textContent;
        if (name < celValue) {
            break;
        }
    }

	
    var t_row = document.getElementById('indicationTable').insertRow(x);
    t_row.id = "row_" + ref;
    var t_info =t_row.insertCell(0);
    var t_handler = t_row.insertCell(1);
    var t_filter =t_row.insertCell(2);
    var t_state = t_row.insertCell(3);
	
	
	
    refArray[name] = ref;

    t_info.innerHTML=info;
    t_handler.innerHTML=handler;
    t_filter.innerHTML=filter;
    t_state.innerHTML = state;
}


/**
 * Writes received content on page.
 */
function writeContent(data) {
    //define a global variable to store property names and corresponding $ref values 
    refArray = new Array();
	
    var processList = JSON.parse(data);
    var indicationList=processList.instances;
    var i=0;
    for (i=0; i < indicationList.length; i++) {
        insertindicationRow(decodeURIComponent(indicationList[i].properties.SubscriptionInfo),
        		decodeURIComponent(indicationList[i].properties.Handler),
        		decodeURIComponent(indicationList[i].properties.Filter),
        		decodeURIComponent(indicationList[i].properties.SubscriptionState));
    }
    //change background color for every second row to improve readability
    var t_rows = document.getElementById('indicationTable').rows;
    var x;
    for (x=0; x < t_rows.length; x+=2) {
        document.getElementById('indicationTable').rows[x].style.background = "#EEEEEE";
        document.getElementById('indicationTable').rows[x+1].style.background = "#E0E0E0";		
    }
	
	
}

/**function to delete all rows of a table except the head row
 * afterwards a new request is send to get the lastet data and the table can be rebuilt
 */
function reloadTable(table)
{
    var rows = table.rows;
    var rowCount = rows.length;
    for (var i = rowCount-1; i > 0; i--)
        {
            table.deleteRow(i);
        }
   //default case, change it to redraw another table
   genericRequest('root%2FPG_InterOp/enum?class=CIM_IndicationSubscription', writeContent, true);         
    
}


