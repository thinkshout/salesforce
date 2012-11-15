<?php

/**
 * Expose the partner SOAP API by extending SforcePartnerClient and configuring
 * it with the OAUTH credentials and endpoints from the Salesforce API class.
 */
class SalesforceSoapPartner extends SforcePartnerClient {
  private $SalesforceApi;

  function __construct(Salesforce $SalesforceApi, $wsdl = NULL) {
    parent::SforcePartnerClient();
    if (empty($wsdl)) {
      $wsdl = libraries_get_path('salesforce') . '/soapclient/partner.wsdl.xml';
    }
    $this->createConnection($wsdl);
    $this->SalesforceApi = $SalesforceApi;
    $this->setSessionHeader($this->SalesforceApi->getAccessToken());
    $this->setEndPoint(($this->endPoint()));
  }

  /**
   * Return the SOAP endpoint URL.
   *
   * @return string
   */
  private function endPoint() {
    return $this->SalesforceApi->getInstanceURL() . "/services/Soap/u" . $this->SalesforceApi->rest_api_version['url'];
  }

}
