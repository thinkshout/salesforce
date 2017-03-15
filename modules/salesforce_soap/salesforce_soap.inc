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
  protected $isConnected;
  private $refreshed;

  /**
   * Constructor for SalesforceSoapPartner.
   *
   * @param Salesforce $sfapi
   *   Salesforce API object
   * @param string $wsdl
   *   WSDL provided by Salesforce account.  One will be created if not
   *   explicitly provided.
   */
  public function __construct(Salesforce $sfapi, $wsdl = NULL) {
    parent::__construct();
    if (empty($wsdl)) {
      $wsdl = libraries_get_path('salesforce') . '/soapclient/partner.wsdl.xml';
    }
    $this->setConnected(FALSE);
    $this->createConnection($wsdl);
    $this->salesforceApi = $sfapi;

    // Use the "isAuthorized" callback to initialize session headers.
    // Callers can use the trySoap() wrapper to automatically handle
    // the SoapFault invalid session exception.
    if ($this->salesforceApi->isAuthorized()) {
      $token = $this->salesforceApi->getAccessToken();
      if (!$token) {
        $token = $this->salesforceApi->refreshToken();
      }
      $this->setSessionHeader($token);
      $this->setEndPoint($this->salesforceApi->getApiEndPoint('partner'));
      $this->setConnected(TRUE);
    }
    else {
      // Alert the caller that the Salesforce instance has not been configured.
      watchdog('salesforce_soap', 'Attempting to use an instance of
        the SalesforceSoapPartner class with a non-authorized Salesforce API
        instance. Please ensure the Salesforce API module has been configured
        and successfully authorized with a Salesforce org.', NULL, WATCHDOG_ERROR);

      $this->setConnected(FALSE);
    }
  }

  /**
   * Setter for isConnected private variable.
   *
   * @param bool $is_connected
   *   Boolean to indicate whether the instance made a successful connection
   *   to the SOAP API.
   */
  protected function setConnected($is_connected) {
    $this->isConnected = $is_connected;
  }

  /**
   * Indicates whether or not a successfull connection was made the SOAP API.
   */
  public function isConnected() {
    return $this->isConnected;
  }

  /**
   * Wrap SF SOAP API resources in a try-catch so that we can automatically
   * re-auth when we have an invalid session id / access token.
   *
   * @param string $function
   *   The name of the SforcePartnerClient function to attempt.
   * @param mixed arg1, arg2, arg3...
   *   Parameters to pass through to $function
   *
   * @return the results of $function
   * @see SforcePartnerClient()
   */
  public function trySoap($function) {
    $args = func_get_args();
    array_shift($args);

    try {
      $results = call_user_func_array(array($this, $function), $args);
      // If returned without exceptions, reset the refreshed flag.
      $this->refreshed = FALSE;
      return $results;
    }
    catch (SoapFault $e) {
      // sf:INVALID_SESSION_ID is thrown on expired login (and other reasons).
      // Our only recourse is to try refreshing our auth token. If we get any
      // other exception, bubble it up.
      if ($e->faultcode != 'sf:INVALID_SESSION_ID') {
        throw $e;
      }

      if (!$this->refreshed) {
        // If we got an invalid session exception, try to refresh the auth
        // token through REST API. The "refreshed" flag will make sure we retry
        // only once.
        $this->refreshed = TRUE;
        $this->salesforceApi->refreshToken();
        return $this->trySoap($function, $args);
      }

      // If we've already tried a refresh, this refresh token is probably
      // invalid. Kill it, log, and bubble the exception.
      $this->setConnected(FALSE);
      variable_del('salesforce_refresh_token');
      $link = l(t('the Salesforce configuration page'), 'admin/config/services/salesforce');
      $message = t('Your website is not authorized to connect with Salesforce. Please visit !config to authorize your site.', array('!config' => $link));
      salesforce_set_message($message, $error, FALSE);
      watchdog_exception('salesforce_soap', $e, t('Salesforce SOAP %function failed. Your website is not authorized to connect with Salesforce.'), array('%function' => $function), WATCHDOG_ERROR, $link);
      throw $e;

    }
  }
}
