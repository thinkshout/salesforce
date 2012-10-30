<?php

class SalesforceSoap extends SforcePartnerClient {
  private $SalesforceApi;

  function __construct(Salesforce $SalesforceApi, $wsdl = 'partner.wsdl.xml') {
    parent::SforcePartnerClient();
    $wsdl_path = DRUPAL_ROOT . '/profiles/ams_dev/libraries/salesforce/soapclient/' . $wsdl;
    $this->createConnection($wsdl_path);
    $this->SalesforceApi = $SalesforceApi;
    $this->setSessionHeader($this->SalesforceApi->getAccessToken());
    $this->setEndPoint(($this->endPoint()));
  }

  private function endPoint() {
    return $this->SalesforceApi->getInstanceURL() . "/services/Soap/c" . $this->SalesforceApi->rest_api_version['url'];
  }

}
