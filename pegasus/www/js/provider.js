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

configURL = "root%2FPG_InterOp/enum?class=PG_ProviderModule";
currentElement = null;

function insertProviderRow(name, location, vendor, version, interfacetype, state, ref) {
    var t_rows = document.getElementById('providerTable').rows;
	

    var x=1;

    for (x=1; x < t_rows.length; x++) {
        var celValue = t_rows[x].cells[0].textContent;
        if (name < celValue) {
            break;
        }
    }

	
    var t_row = document.getElementById('providerTable').insertRow(x);
    t_row.id = "row_" + ref;
    var t_name =t_row.insertCell(0);
    var t_location = t_row.insertCell(1);
    var t_vendor =t_row.insertCell(2);
    var t_version =t_row.insertCell(3);
    var t_interfacetype =t_row.insertCell(4);
    var t_state = t_row.insertCell(5);
	
	
	
    refArray[name] = ref;

    t_name.innerHTML=name;
    t_location.innerHTML=location;
    t_vendor.innerHTML=vendor;
    t_version.innerHTML = version;
    t_interfacetype.innerHTML = interfacetype; 
    t_state.innerHTML = state;
}


/**
 * Writes received content on page.
 */
function writeContent(data) {
    //define a global variable to store property names and corresponding $ref values 
    refArray = new Array();
	
    var responseArray = JSON.parse(data);
    var providerList=responseArray.instances;
    var i=0;
    for (i=0; i < providerList.length; i++) {
        insertProviderRow(decodeURIComponent(providerList[i].properties.Name),
        		          decodeURIComponent(providerList[i].properties.Location),
        		          decodeURIComponent(providerList[i].properties.Vendor),
        		          decodeURIComponent(providerList[i].properties.Version),
        		          decodeURIComponent(providerList[i].properties.InterfaceType),
        		          decodeURIComponent(providerList[i].properties.OperationalStatus));
    }
    //change background color for every second row to improve readability
    var t_rows = document.getElementById('providerTable').rows;
    var x;
    for (x=0; x < t_rows.length; x+=2) {
        document.getElementById('providerTable').rows[x].style.background = "#EEEEEE";
        //document.getElementById('providerTable').rows[x+1].style.background = "#E0E0E0";		
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
   genericRequest('root%2FPG_InterOp/enum?class=PG_ProviderModule', writeContent, true);         
    
}


