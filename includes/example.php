<?php

/*
 * Client sample that performs set of basic salesforce API operations.
 */

require_once('salesforce.php');

// test code
$username = 'steve@raincitystudios.com';
$password = 'l3tme1n';

// create client and login
$sfdc = new salesforce('partner.wsdl');
$loginResult = $sfdc->login($username, $password);

//set batch size header
$batchSize = new soapval('batchSize', null, 2);
$sfdc->setHeader('QueryOptions', array($batchSize));

if (!$loginResult){

    print_r("\nfailed login: \n\n");
    print_r($sfdc->result);

} else {


    # if you want to use a local proxy, uncomment this out
    # $sfdc->setURL('http://localhost:81/services/Soap/u/6.0');
        
    // simple call, showing XML request and response
    $response = $sfdc->getServerTimestamp();
    // print_r($sfdc->client->request);
    // print_r($sfdc->client->response);
    print_r("\ngetServerTimestamp: \n\n");
    print_r($response);
    $startDate = $response;

    // create Contact
    $contact = new sObject('Contact', 
                           null, 
                           array(
                               'FirstName' => 'TestFirstName', 
                               'LastName' => 'TestLastName',
                               'Phone' => '555-555-1212',
                               'Fax' => '444-444-1212'
                           )                           
                          );
    $createResult = $sfdc->create($contact);
    print_r("\ncreate one: \n\n");
    print_r($createResult);
    
    // query Contact by ID
    $id = $createResult['id'];
    $queryResult = $sfdc->query("select id, firstname, lastname, phone, fax from contact where id = '$id'");
    print_r("\nquery by id: \n\n");
    print_r($queryResult);
    
    // update Contact
    $contact1 = new sObject('Contact', 
            $id, 
            array(
                'FirstName' => 'TestFirstName', 
                'LastName' => 'TestLastNameUpdate'
            ),                           
                array('Phone', 'Fax')
           );
    $updateResult = $sfdc->update($contact1);
    print_r("\nupdate one: \n\n");
    print_r($updateResult);

    // query updated Contact by ID
    $queryResult = $sfdc->query("select id, firstname, lastname, phone, fax from contact where id = '$id'");
    print_r("\nquery by id: \n\n");
    print_r($queryResult);
    
    // retrieve by ID
    $retrieveResult = $sfdc->retrieve("id, firstname, lastname", "contact", array($id, $id));
    print_r("\nretrieve: \n\n");
    print_r($retrieveResult);

    // delete contact
    $deleteResult = $sfdc->delete($createResult['id']);
    $id = $deleteResult['id'];
    print_r("\ndelete one ($id): \n\n");
    print_r($deleteResult);
    
    // bulk create Contacts
    $contacts = array($contact, $contact);
    $createResult = $sfdc->create($contacts);
    print_r("\ncreate multiple (array): \n\n");
    print_r($createResult);

    // query Contacts by name
    $queryResult = $sfdc->query("select id, firstname, lastname from contact where firstname = 'TestFirstName'");
    print_r("\nquery by name: \n\n");
    print_r($queryResult);

    // bulk delete Contacts by ID
    $id1 = $queryResult['records'][0]->id;
    $id2 = $queryResult['records'][1]->id;
    $ids = array($id1, $id2);
    $deleteResult = $sfdc->delete($ids);
    print_r("\ndelete multiple ($id1 $id2): \n\n");
    print_r($deleteResult);

    // query Contacts by name (verify deleted)
    $queryResult = $sfdc->query("select id, firstname, lastname from contact where firstname = 'TestFirstName'");
    print_r("\nquery: (all deleted) \n\n");
    print_r($queryResult);
    
    // create lead
    $lead = new sObject('Lead', 
                           null, 
                           array(
                               'Company' => 'TestCompany', 
                               'FirstName' => 'TestFirstName', 
                               'LastName' => 'TestLastName',
                               'Phone' => '555-555-1212',
                               'Fax' => '444-444-1212'
                           )                           
                          );

    $useDefaultRule = new soapval('useDefaultRule', null, 'true');
    $sfdc->setHeader('AssignmentRuleHeader', array($useDefaultRule));

    $createResult = $sfdc->create($lead);
    print_r("\ncreate lead: \n\n");
    print_r($sfdc->client->request);
    print_r($sfdc->client->response);
    print_r($createResult);

    $id = $createResult['id'];

    // convert lead
    $leadConvert = new LeadConvert(null, null, 'Qualified', false, $id, 'Converted Lead Opportunity', true, null, false);
    $leadConvertResult = $sfdc->convertLead($leadConvert);
    print_r("\nconvert lead: \n\n");
    print_r($sfdc->client->request);
    print_r($leadConvertResult);

    print_r("\nsleeping for 60 seconds for updated and deleted examples\n\n");
    sleep(60);

    $endDate = $sfdc->getServerTimestamp();

    // getUpdated 
    $getUpdatedResult = $sfdc->getUpdated("contact", $startDate, $endDate);
    print_r("\ngetUpdatedResult (start: $startDate, end: $endDate)\n\n");
    print_r($getUpdatedResult);

    // getDeleted 
    $getDeletedResult = $sfdc->getDeleted("contact", $startDate, $endDate);
    print_r("\ngetDeletedResult (start: $startDate, end: $endDate)\n\n");
    print_r($getDeletedResult);


}

?>

