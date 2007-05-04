<?php

/*

salesforce.com Partner PHP client 

Copyright (c) 2005 Ryan Choi

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you have any questions or comments, please email:

Ryan Choi
rchoi21@hotmail.com
http://www.ryankicks.com

*/
    
/**
 * salesforce client for use with a modified nuSOAP PHP library.
 */

require_once('nusoap.php');

/**
 * 
 * salesforce
 * 
 * @author Ryan Choi <rchoi21@hotmail.com>
 * @version
 * @access public
 */
class salesforce {

    var $partnerNs = 'urn:partner.soap.sforce.com';

    var $client; 
    var $result;

    var $url;
    var $session;

    /**
     * constructor for salesforce client.
     * 
     * @param string
     *            $wsdl local reference to partner WSDL file.
     * @access public
     */
    function salesforce($wsdl) {

        $this->client = new nusoapclient($wsdl, true);

    }

    /**
     * login with username and password. will set SessionId header upon
     * successful login.
     * 
     * @param string
     *            $username username for login.
     * @param string
     *            $password password for login.
     * @return mixed LoginResult complex type. (See WSDL.)
     * @access public
     */
    function login($username, $password){

        // Doc/lit parameters get wrapped
        $param = array('username' => $username, 'password' => $password);
        $this->result = $this->client->call('login', array('parameters' => $param), '', '', false, true);

        if ($this->client->getError() || $this->client->fault) {

            return false;

        } else {

            $wrapper = $this->result['result'];
            $url = $wrapper['serverUrl'];
            $session = $wrapper['sessionId'];

	    // setup the client's URL with the response from the login 
	    $this->setURL($url);

	    // setup the client's session with the response from the login 
	    $this->setSessionId($session);

            return $this->result['result'];

        }
    }

    /**
     * set session of client. called in login, and accessible for manual
     * setting if session already available. 
     * 
     * @param string
     *            $session session string 
     * @return none
     * @access public
     */
    function setSessionId($session){

        $element = new soapval('sessionId', null, $session);
        $element = array($element);
        $this->setHeader('SessionHeader', $element);

    }

    /**
     * set URL of client. all requests are done insecure.
     * 
     * @param string
     *            $url URL of API.
     * @return none
     * @access public
     */
    function setURL($url){
        $this->client->forceEndpoint = str_replace("https", "http", $url);
    }
    
    /**
     * set header on client.
     * 
     * @param string
     *            $headerName name of header
     * @param array
     *            $headerValue array of soapvals of values
     * @access public
     */
    function setHeader($headerName, $headerValue){

        $header = new soapval($headerName, null, $headerValue, $this->partnerNs);
        $headers = null;

        if ($this->client->requestHeaders == null){
            $headers = array($header);
        } else {
            $headers = $this->client->requestHeaders;
            $count = 0;
            foreach ($headers as $hdr) {
                $existingHdrName = $hdr->name;
                if ($existingHdrName == $headerName){
                    break;
                }
                $count++;
            }
            array_splice($headers, $count, 1, array($header));
        }
        
        $this->client->setHeaders($headers);

    }

    /**
     * create sObjects.
     * 
     * @param mixed
     *            $values either a single sObject or an array of sObjects
     * @return mixed either a single SaveResult complex type or an array of
     *         SaveResult complex types. (See WSDL.)
     * @access public
     */
    function create($sObjects){
        $param = array('sObjects' => $sObjects);
        $this->result = $this->client->call('create', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }

    /**
     * update sObjects.
     * 
     * @param mixed
     *            $values either a single sObject or an array of sObjects
     * @return mixed either a single SaveResult complex type or an array of
     *         SaveResult complex types. (See WSDL.)
     * @access public
     */
    function update($sObjects){
        $param = array('sObjects' => $sObjects);
        $this->result = $this->client->call('update', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    } 

    /**
     * delete sObjects.
     * 
     * @param mixed
     *            $values either a single id (string) or an array of ids (array
     *            of strings)
     * @return mixed either a single DeleteResult complex type or an array of
     *         DeleteResult complex types. (See WSDL.)
     * @access public
     */
    function delete($ids){
        $param = array('ids' => $ids);
        $this->result = $this->client->call('delete', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }

    /**
     * perform query using SOQL command.
     * 
     * @param string
     *            $queryString SOQL query request
     * @return mixed QueryResult complex type. (See WSDL.)
     * @access public
     */
    function query($queryString){
        $param = array('queryString' => $queryString);
        $this->result = $this->client->call('query', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }
    
    /**
     * queries more query records based on query locator.
     * 
     * @param string
     *            $queryLocator string representing query locator
     * @return mixed QueryResult complex type. (See WSDL.)
     * @access public
     */
    function queryMore($queryLocator){
        $param = array('queryLocator' => $queryLocator);
        $this->result = $this->client->call('queryMore', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }

    /**
     * perform retrieve
     * 
     * @param string
     *            $fieldList fields of sObject to return
     *            $sObjectType type of object to retrieve
     *            $ids ids of sObjects to retrieve
     * @return mixed array of sObjects. (See WSDL.)
     * @access public
     */
    function retrieve($fieldList, $sObjectType, $ids){
        $param = array(
		    'fieldList' => $fieldList, 
		    'sObjectType' => $sObjectType, 
		    'ids' => $ids
                 );
        $this->result = $this->client->call('retrieve', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }

    /**
     * get sObjects updated during a specified interval
     * 
     * @param string
     *            $sObjectType type of sObject
     *            $startDate 
     *            $endDate
     * @return mixed GetUpdatedResult complex type. (See WSDL.)
     * @access public
     */
    function getUpdated($sObjectType, $startDate, $endDate){
        $param = array(
		    'sObjectType' => $sObjectType, 
		    'startDate' => $startDate,
		    'endDate' => $endDate
                 );
        $this->result = $this->client->call('getUpdated', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }

    /**
     * get ids of sObjects deleted during a specified interval
     * 
     * @param string
     *            $sObjectType type of sObject
     *            $startDate 
     *            $endDate
     * @return mixed GetDeletedResult complex type. (See WSDL.)
     * @access public
     */
    function getDeleted($sObjectType, $startDate, $endDate){
        $param = array(
		    'sObjectType' => $sObjectType, 
		    'startDate' => $startDate,
		    'endDate' => $endDate
                 );
        $this->result = $this->client->call('getDeleted', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }


    /**
     * perform lead converts.
     * 
     * @param mixed $leadConverts either a single LeadConvert object or an array of LeadConvert objects
     * @return mixed LeadConverResult complex types. (See WSDL.)
     * @access public
     */
    function convertLead($leadConverts){
        $param = array('leadConverts' => $leadConverts);
        $this->result = $this->client->call('convertLead', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }
    
    /**
     * returns current time of salesforce server
     * 
     * @return timestamp time at sforce server.
     * @access public
     */
    function getServerTimestamp(){
        $param = array('' => '');
        $this->result = $this->client->call('getServerTimestamp', array('parameters' => $param), '', '', false, true);
        return $this->result['result']['timestamp'];
    }
    
    /**
     * returns global description of API settings
     * 
     * @return mixed DescribeGlobalResult complex type. (See WSDL.)
     * @access public
     */
    function describeGlobal(){
        $param = array('' => '');
        $this->result = $this->client->call('describeGlobal', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }
    
    /**
     * returns description for given sObject
     * 
     * @param string
     *            $sObjectType string sObject name
     * @return mixed DescribeSObjectResult complex type. (See WSDL.)
     * @access public
     */
    function describeSObject($sObjectType){
        $param = array('sObjectType' => $sObjectType);
        $this->result = $this->client->call('describeSObject', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }
    
    /**
     * returns describe results for multiple sObjects.
     * 
     * @param array
     *            $sObjectTypes array of string sObject names
     * @return array array of DescribeSObjectResult complex types. (See WSDL.)
     * @access public
     */
    function describeSObjects($sObjectTypes){
        $param = array('sObjectType' => $sObjectTypes);
        $this->result = $this->client->call('describeSObjects', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }
    
    /**
     * returns layouts for given sObject
     * 
     * @param string
     *            $sObjectType string sObject name
     * @return mixed DescribeLayoutResult complex type. (See WSDL.)
     * @access public
     */
    function describeLayout($sObjectType){
        $param = array('sObjectType' => $sObjectType);
        $this->result = $this->client->call('describeLayout', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }
    
    /**
     * searches for specified entities given SOSL
     * 
     * @param string
     *            $searchString SOSL search request
     * @return mixed SearchResult complex type. (See WSDL.)
     * @access public
     */
    function search($searchString){
        $param = array('searchString' => $searchString);
        $this->result = $this->client->call('search', array('parameters' => $param), '', '', false, true);
        return $this->result['result'];
    }
    
}

/**
 * salesforce sObject.
 * 
 * @access public
 */
class sObject {
    
    var $type;
    var $id;
    var $values;
    var $fieldsToNull;
    
    function sObject($type, $id=null, $values=null, $fieldsToNull=null) {
        
        // deserialize record from nusoap.php
        if (is_array($type)){
        
            $this->values = array();
            
            foreach ($type as $k => $v){
                if ($k == 'type'){
                    $this->type = $v;
                } else if ($k == 'Id'){
                    if (is_array($v)){
                        $this->id = $v[0];
                    } else {
                        $this->id = $v;
                    }
                } else {
                    $this->values[$k] = $v;
                }
            }
            
        } else {
            
            $this->type = $type;
            $this->id = $id;
            $this->values = $values;
            $this->fieldsToNull = $fieldsToNull;
            
        }
    }
    
    function serialize(){
        
        $valuesSer['type'] = $this->type;
        if ($this->fieldsToNull != null){
            $fieldsToNull = array();
            $index = 0;
            foreach($this->fieldsToNull as $value){
                $fieldsToNull[$index] = $value;
                $index++;
            }
            $valuesSer['fieldsToNull'] = new RepeatedElementsArray('fieldsToNull', $fieldsToNull);
        }
        $valuesSer['Id'] = $this->id;
        
        foreach ($this->values as $k => $v) {
            $valuesSer[$k] = $v;
        }
        
        $sobj = new soapval('sObject', false, $valuesSer);

        return $sobj ->serialize();

    }
    
}

/**
 * salesforce sObject.
 * 
 * @access public
 */
class LeadConvert {

    var $accountId;
    var $contactId;
    var $convertedStatus;
    var $doNotCreateOpportunity;
    var $leadId;
    var $opportunityName;
    var $overwriteLeadSource;
    var $ownerId;
    var $sendNotificationEmail;

    function LeadConvert($accountId, $contactId, $convertedStatus, $doNotCreateOpportunity, $leadId, $opportunityName, $overwriteLeadSource, $ownerId, $sendNotificationEmail){

        $this->accountId = $accountId;
        $this->contactId = $contactId;
        $this->convertedStatus = $convertedStatus;
        $this->doNotCreateOpportunity = $doNotCreateOpportunity;
        $this->leadId = $leadId;
        $this->opportunityName = $opportunityName;
        $this->overwriteLeadSource = $overwriteLeadSource;
        $this->ownerId = $ownerId;
        $this->sendNotificationEmail = $sendNotificationEmail;

    } 

    function serialize(){
        
        $valuesSer['accountId'] = $this->accountId;
        $valuesSer['contactId'] = $this->contactId;
        $valuesSer['convertedStatus'] = $this->convertedStatus;
        $valuesSer['doNotCreateOpportunity'] = $this->doNotCreateOpportunity;
        $valuesSer['leadId'] = $this->leadId;
        $valuesSer['opportunityName'] = $this->opportunityName;
        $valuesSer['overwriteLeadSource'] = $this->overwriteLeadSource;
        $valuesSer['ownerId'] = $this->ownerId;
        $valuesSer['sendNotificationEmail'] = $this->sendNotificationEmail;

        $leadConvert = new soapval('LeadConvert', false, $valuesSer);
        return $leadConvert->serialize();

    }
   

}
   

/**
 * helps SOAP-ENC arrays to be encoded as repeated elements.
 * 
 * @access private
 */
class RepeatedElementsArray {
    
    var $elementName;
    var $values;
    
    /**
     * @param string
     *            name of element
     * @param array
     *            values to be encoded. Currently, only strings are supported.
     * @access public
     */
    function RepeatedElementsArray($elementName, $values){
        $this->elementName = $elementName;
        $this->values = $values;    
    }
    
    function serialize($use='encoded'){
        $xml = "";
        foreach($this->values as $value){
            $xml .= "<$this->elementName>$value</$this->elementName>";
        }
        return $xml;
    }
    
}
