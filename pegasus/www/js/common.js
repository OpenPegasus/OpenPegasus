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

//check if it is http or https
httpTag =document.location.protocol; 
//base url which is identical for all requests
baseURL = httpTag+"//"+window.location.hostname+":"+window.location.port+"/cimrs/";
host = httpTag+"//"+window.location.hostname+":"+window.location.port+"/";

function genericRequestGet(urlAdd, funcToCall, synchronous){
    /*
	 * based on api example from: 
	 * https://developer.mozilla.org/En/XMLHttpRequest/Using_XMLHttpRequest#Example.3a_Asynchronous_request
	 */
        
        
    if (funcToCall == "writeContent")
        funcToCall = writeContent;
    
    try {
        var req = new XMLHttpRequest();
    } catch(e) {
        alert('No support for XMLHTTPRequests');
        return;
    }
    var url = baseURL + urlAdd;
    
    var async = synchronous;
    req.open('GET', url, async);

    // add progress listener (differs from version to version...)
    req.onreadystatechange = function () {
        // state complete is of interest, only
        if (req.readyState == 4) {
            if (req.status == 200) {
                //call of the passed function (as parameter)
                funcToCall(req.responseText);
            //dump(req.responseText);
            } else {
                //if return code is another than 200 process error
                processRequestError(req.responseText);
                
            }
        }
    };

    // send request
    req.send();
        
}


function genericRequestPost(requestURL, requestContent,funcToCall, synchronous){
    /*
	 * based on api example from: 
	 * https://developer.mozilla.org/En/XMLHttpRequest/Using_XMLHttpRequest#Example.3a_Asynchronous_request
	 */
 
    try {
        var req = new XMLHttpRequest();
    } catch(e) {
        alert('No support for XMLHTTPRequests');
        return;
    }
    var url = baseURL + requestURL;
    
    var async = synchronous;
    req.open('POST', url, async);
    req.setRequestHeader('Content-Type', 'application/json');
    
    // add progress listener (differs from version to version...)
    req.onreadystatechange = function () {
        // state complete is of interest, only
        if (req.readyState == 4) {
            if (req.status == 200) {
                //call of the passed function (as parameter)
                funcToCall(req.responseText);
            //dump(req.responseText);
            } else {
                //if return code is another than 200 process error
                processRequestError(req.responseText);
                
            }
        }
    };

    // send request
    req.send(JSON.stringify(requestContent));
        
}


