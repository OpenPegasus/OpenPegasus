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

getConfigURL = "root%2FPG_Internal/enum?class=PG_ConfigSetting";
updateConfigURL="root%2FPG_Internal/PG_ConfigSetting/";
currentElement = null;

function insertConfigRow(name, current, defaultvalue, planned, dynamic, ref) {
    var t_rows = document.getElementById('configTable').rows;
	

    var x=1;

    for (x=1; x < t_rows.length; x++) {
        var celValue = t_rows[x].cells[0].textContent;
        if (name < celValue) {
            break;
        }
    }

	
    var t_row = document.getElementById('configTable').insertRow(x);
    t_row.id = "row_" + ref;
    var t_name =t_row.insertCell(0);
    var t_current = t_row.insertCell(1);
    var t_dynamic =t_row.insertCell(2);
    var t_planned =t_row.insertCell(3);
    var t_defaultvalue =t_row.insertCell(4);
    var t_actions = t_row.insertCell(5);
	
	
	
    t_name.innerHTML=name;
    t_current.innerHTML=current;
    t_defaultvalue.innerHTML=defaultvalue;
    t_planned.innerHTML = planned;
    //store ref value
    refArray[name] = ref;
    //set dynamic property icons
    if (dynamic == true)
    {
        t_dynamic.innerHTML = "<img src='images/green_circle.gif' alt='true' />"; 
    }
    else
    {
        t_dynamic.innerHTML = "<img src='images/red_circle.gif' alt='false' />";
    }
    //set the property value name as ID, starting with "edit_"
    t_actions.innerHTML = "<table style='text-align: left; width: 100%;' border='0' cellpadding='2' cellspacing='2'>" +
    "<tbody><tr><td style='vertical-align: top;'><a href='#' id='reset_"+name+"' onclick='showResetOverlay(this)'><img style='width: 12px; height: 12px;' alt='reset' src='images/reset.png'"+
    "title='reset default value'></a></td>"+
    "<td style='vertical-align: top;'><a href='#' id='edit_"+name+"' onclick='showEditOverlay(this)'><img style='width: 12px; "+
    "height: 12px;' alt='edit' src='images/edit.png' title='edit config setting'></a>"+
    "</td></tr></tbody></table>";
	
}


/**
 * Writes received content on page.
 */
function writeContent(data) {
    //define a global variable to store property names and corresponding $ref values 
    refArray = new Array();
	
    var processList = JSON.parse(data);
    var instanceList=processList.instances;
    var i=0;
    for (i=0; i < instanceList.length; i++) {
        insertConfigRow(decodeURIComponent(instanceList[i].properties.PropertyName),
        		        decodeURIComponent(instanceList[i].properties.CurrentValue),
        		        decodeURIComponent(instanceList[i].properties.DefaultValue),
        		        decodeURIComponent(instanceList[i].properties.PlannedValue),
        		        instanceList[i].properties.DynamicProperty,
        		        decodeURIComponent(instanceList[i].properties.PropertyName));
    }
    //change background color for every second row to improve readability
    var t_rows = document.getElementById('configTable').rows;
    var x;
    for (x=1; x < t_rows.length; x+=2) {
        document.getElementById('configTable').rows[x].style.background = "#CACDCE";		
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
    enableApplyButton();
    //default case, change it to redraw another table
    genericRequestGet('root%2FPG_Internal/enum?class=PG_ConfigSetting', writeContent, true);         
    
}

/**
 *this functions replaces the content of the central div, to show some other page here.
 *this is just an approach to to use static headers and footers as the cim server does not support server side scripting.
 *if there is a better solution for that issue, feel free to change that.
 *
 *deprecated
 */
function replContDiv(urlAdd)
{
    try {
        var req = new XMLHttpRequest();
    } catch(e) {
        alert('No support for XMLHTTPRequests');
        return;
    }
    
    var url = host + urlAdd;
	var async = false;
    req.open('GET', url, async);

    // add progress listener (differs from version to version...)
    req.onreadystatechange = function () {
        // state complete is of interest, only
        if (req.readyState == 4) {
            if (req.status == 200) {
                document.getElementById("content").innerHTML = req.responseText;
				if (urlAdd == "config.html")
				{
					genericRequestGet(geyConfigURL, writeContent, false);
				}
            //dump(req.responseText);
            } else {
                dump("Error loading page\n");
            }
        }
    };

    // send request
    req.send();
    
    
    
}




/**
*Function to show an overlay for editing config settings
*/
function showEditOverlay(element) {
    //uncheck both checkboxes
    document.change_setting.checkbox_planned.checked = false;
    document.change_setting.checkbox_current.checked = false;
    currentElement = element;
    var propertyName = element.id;   
    propertyName = propertyName.replace("edit_","");  
    var title = document.getElementById("settings_title");
    title.innerHTML="Setting being changed: <b>" + propertyName + "</b>";	
    // get the type of the property
    var typeOfProperty = document.getElementById("row_"+propertyName).cells[4].innerHTML;
    var dynamic = document.getElementById("row_"+propertyName).cells[2].innerHTML;
    //if cell contains "red" (= graphics red_circle) grey out the checkbox
    if (dynamic.indexOf("red") != -1)
    {
        document.change_setting.checkbox_current.disabled = true;
    }
    else
    {
        document.change_setting.checkbox_current.disabled = false;
    }
	// show dropdown menu instead of text field if value is boolean
	if (typeOfProperty == "false" || typeOfProperty == "true") 
	{
		document.getElementById("input_area").innerHTML = "<select name='input_value' ><option value='true'>true</option><option value='false'>false</option></select>";
	} 
	else 
	{
        //get the current value from the table
        var currentValue = document.getElementById("row_"+propertyName).cells[1].textContent;
		document.getElementById("input_area").innerHTML = "<input id='input_field' name='input_value' type='text' style='width:90%' onkeypress='enableApplyButton()' value='"+currentValue+"'>";   
    }
    el = document.getElementById("overlay");
    el.style.visibility = "visible";
    
    //select content
    var inputField = document.getElementById("input_field");
    if (inputField != null)
    {
        inputField.select();            
    } 
}

/**
*Function to show an overlay for resetting config settings
*/
function showResetOverlay(element) {    
    //uncheck both checkboxes
    document.reset_setting.checkbox_planned_reset.checked = false;
    document.reset_setting.checkbox_current_reset.checked = false;
    currentElement = element;
    var propertyName = element.id;   
    propertyName = propertyName.replace("reset_","");  
    var title = document.getElementById("reset_title");
    title.innerHTML="Setting being resetted: <b>" + propertyName + "</b>";	
    var dynamic = document.getElementById("row_"+propertyName).cells[2].innerHTML;
    
    //if cell contains "red" (= graphics red_circle) grey out the checkbox
    if (dynamic.indexOf("red") != -1)
    {
        document.reset_setting.checkbox_current_reset.disabled = true;
    }
    else
    {
        document.reset_setting.checkbox_current_reset.disabled = false;
    }
    el = document.getElementById("overlay_reset");
    el.style.visibility = "visible";
}

/**
*Function to hide the overlay window
*/
function hide_overlay(element, overlayID) {
    el = document.getElementById(overlayID);
    el.style.visibility = "hidden";
    currentElement = null;
    
    //remove possible text content of any status bars
    document.getElementById("settings_errorbar").textContent = "";
    document.getElementById("reset_errorbar").textContent = "";
 
}

/**
 * function to actually set the value of the property entered in the form
 */
function processFormData(valueCurrent, valuePlanned, valueToSet){
    //disable apply button
    document.change_setting.apply.disabled = true;
    //check if both checkboxes are unchecked and tell the user to check at least one
    if (!valueCurrent.checked && !valuePlanned.checked)
    {
         var temp = document.getElementById("settings_errorbar");
         temp.innerHTML = "<FONT COLOR='#DE320B'> check at least one checkbox </FONT>";
         return;
     }
    
    var propertyValue = valueToSet.value;
    
    //Create requestURL
    var ref = refArray[currentElement.id.replace("edit_","")];
    var methodURL = ref + "/UpdatePropertyValue";
    var requestURL=updateConfigURL+methodURL;
    
    //Create the request content for Post 
    var requestContent=
        {
    		"kind": " methodrequest",
    		"self": requestURL,
    		"method": "UpdatePropertyValue",
    		"parameters":
    		    {
    	         "PropertyValue":propertyValue,
    	         "resetvalue":false,
    	         "setcurrentvalue":valueCurrent.checked,
    	         "setPlannedValue":valuePlanned.checked
    		    }
    	}
    	
    genericRequestPost(requestURL,requestContent, processRequestResult, true);
    //hide_overlay(null);  
    
}

/**
 * function to reset current or planned value to default values
 */
function processReset(valueCurrent, valuePlanned){
    var ref = refArray[currentElement.id.replace("reset_","")];
    var methodURL = ref + "/UpdatePropertyValue";
    var requestURL=updateConfigURL+methodURL;
    
    var requestContent=
	{
		"kind": " methodrequest",
		"self": requestURL,
		"method": "UpdatePropertyValue",
		"parameters": 
		{
	        "resetvalue":true,
	        "setcurrentvalue":valueCurrent.checked,
	        "setPlannedValue":valuePlanned.checked
		}
	}
    
    genericRequestPost(requestURL,requestContent, processRequestResult, true);
    //hide_overlay(null);

    
    
}

function processRequestResult(response){
    //show info on correct error bar depending on currentElement
	
    var barToShow;
    if (currentElement.id.indexOf("edit") != -1)
        {
            barToShow = "settings_errorbar";
        }
    else if ((currentElement.id.indexOf("reset") != -1))
        {
            barToShow = "reset_errorbar";
        }
    else
        {
            return;
        }
    var result = JSON.parse(response);
    //for positive responses the property 'ReturnValue' is contained, but double check that here.
    if ('returnvalue' in result)
        {
            if (result.returnvalue == true)
                {
                    var temp = document.getElementById(barToShow);
                    temp.title = "";
                    temp.innerHTML = "<FONT COLOR='#1BE042'>successful</FONT>";
                    //redraw the table
                    reloadTable(document.getElementById('configTable'));                  
                                 
                }
           
        }
    
}

function processRequestError(response){
    //show info on correct error bar depending on currentElement
    var barToShow;
    if (currentElement.id.indexOf("edit") != -1)
        {
            barToShow = "settings_errorbar";
        }
    else if ((currentElement.id.indexOf("reset") != -1))
        {
            barToShow = "reset_errorbar";
        }
    else
        {
            return;
        }
    var result = JSON.parse(response);
    if ('message' in result)
        {
            if (result.message != "")
                {
                     var temp = document.getElementById(barToShow);
                     temp.innerHTML = "<FONT COLOR='#DE320B'> failure (hover here for details) </FONT>";
                     temp.title = result.details;
                }
           
        }
    
}

function enableApplyButton()
{
    document.change_setting.apply.disabled = false;
}


