<?php

/**
 * @file
 * Contains SalesforceSoapPartner.
 */

/**
 * Expose the partner SOAP API by extending SforcePartnerClient and configuring
 * it with the OAUTH credentials and endpoints from the Salesforce API class.
 */
class SalesforceSoapPartner extends SforcePartnerClient {
  protected $salesforceApi;

  /**
   * Constructor for SalesforceSoapPartner.
   *
   * @param Salesforce $sfapi
   *   Salesforce API object
   * @param string $wsdl
   *   WSDL provided by Salesforce account.  One will be created if not
   *   explicitely provided.
   */
  public function __construct(Salesforce $sfapi, $wsdl = NULL) {
    parent::__construct();
    if (empty($wsdl)) {
      $wsdl = libraries_get_path('salesforce') . '/soapclient/partner.wsdl.xml';
    }
    $this->createConnection($wsdl);
    $this->salesforceApi = $sfapi;
    $this->setSessionHeader($this->salesforceApi->getAccessToken());
    $this->setEndPoint($this->salesforceApi->getApiEndPoint('partner'));
  }
}
