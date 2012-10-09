<?php
/**
 * @file
 * Objects, properties, and methods to communicate with the Salesforce REST API
 */

/**
 * Ability to authorize and communicate with the Salesforce REST API.
 */
class Salesforce {

  public $consumer_key;
  public $consumer_secret;

  public $login_url = 'https://login.salesforce.com';

  private $request;

  //@todo: does this need to be configurable?
  public $rest_api_version = array(
    "label" => "Summer '12",
    "url" => "/services/data/v25.0/",
    "version" => "25.0"
  );

  public function __construct($consumer_key, $consumer_secret = '') {
    $this->consumer_key = $consumer_key;
    $this->consumer_secret = $consumer_secret;
  }

  /**
   * Make a call to the Salesforce REST API.
   *
   * @param $path
   * @param array $params
   * @param string $method
   *
   * @return mixed
   * @throws SalesforceException
   */
  public function apiCall($path, $params = array(), $method = 'GET') {
    if (!$this->getAccessToken()) {
      $this->refreshToken();
    }

    $this->buildRequest($path, $params, $method);
    $response = $this->httpRequest();

    switch ($response->code) {
      // The session ID or OAuth token used has expired or is invalid
      case 401:
        // refresh token
        $this->refreshToken();

        // rebuild our request and repeat request
        $this->buildRequest($path, $params, $method);
        $response = $this->httpRequest();
        //@todo: test response code, don't get caught in a loop

        break;
      case 200:
      case 201:
      case 204:
        // all clear
        break;
      default:
        // we have problem
        throw new SalesforceException($$response->message, $response->code);
    }

    $data = drupal_json_decode($response->data);

    if (isset($data['error'])) {
      throw new SalesforceException($data['error_description'], $data['error']);
    }

    return $data;
  }

  /**
   * Build the request, including path and headers. Internal use.
   *
   * @param $path
   * @param $params
   * @param $method
   */
  private function buildRequest($path, $params, $method) {
    $url = $this->getInstanceURL() . $this->rest_api_version['url'] . $path;

    $options = array(
      'method' => $method,
      'headers' => array(
        'Authorization:' => 'OAuth ' . $this->getAccessToken()
      ),
      'data' => drupal_http_build_query($params)
    );

    $this->request = array(
      'url' => $url,
      'options' => $options
    );
  }

  /**
   * Make the HTTP request. Wrapper around drupal_http_request().
   *
   * @return object response.
   */
  private function httpRequest() {
    return drupal_http_request($this->request['url'], $this->request['options']);
  }

  /**
   * Getter and setter for SF API instance URL.
   */
  private function getInstanceURL() {
    return variable_get('salesforce_instance_url', '');
  }
  private function setInstanceURL($url) {
    variable_set('salesforce_instance_url', $url);
  }

  /**
   * Getter and setter for the access token. It's stored in session.
   */
  private function getAccessToken() {
    return isset($_SESSION['salesforce_access_token']) ? $_SESSION['salesforce_access_token'] : FALSE;
  }
  private function setAccessToken($token) {
    $_SESSION['salesforce_access_token'] = $token;
  }

  /**
   * Getter and setter for refresh token.
   */
  private function getRefreshToken() {
    return variable_get('salesforce_refresh_token', '');
  }
  private function setRefreshToken($token) {
    variable_set('salesforce_refresh_token', $token);
  }

  /**
   * Refresh access token based on the refresh token. Updates session variable.
   *
   * @throws SalesforceException
   */
  private function refreshToken() {
    $token_url = $this->login_url . '/services/oauth2/token';

    $query = array(
      'grant_type' => 'refresh_token',
      'refresh_token' => $this->getRefreshToken(),
      'client_id' => $this->consumer_key,
      'client_secret' => $this->consumer_secret,
    );

    $response = drupal_http_request($token_url, array(
      'data' => drupal_http_build_query($query),
      'method' => 'POST',
      'headers' => array(
        // this is an undocumented requirement on SF's end
        'Content-Type' => 'application/x-www-form-urlencoded'
      )
    ));

    if ($response->code != 200) {
      //@todo: deal with error better
      throw new SalesforceException(t('Unable to get a Salesforce access token.'), $response->code);
    }

    $data = drupal_json_decode($response->data);

    if (isset($data['error'])) {
      throw new SalesforceException($data['error_description'], $data['error']);
    }

    $this->setInstanceURL($data['instance_url']);
    $this->setAccessToken($data['access_token']);
  }

  /**
   * OAuth step 1
   * Used to redirect to Salesforce and request and authorization code.
   */
  public function getAuthorizationCode() {
    $url = $this->login_url . '/services/oauth2/authorize';
    $query = array(
      'redirect_uri' => $this->redirectUrl(),
      'response_type' => 'code',
      'client_id' => $this->consumer_key,
    );

    drupal_goto($url, array('query' => $query));
  }

  /**
   * OAuth step 2
   * Exchange an authorization code for an access token.
   *
   * @param $code
   */
  public function requestToken($code) {
    $token_url = $this->login_url . '/services/oauth2/token';

    $query = array(
      'code' => $code,
      'grant_type' => 'authorization_code',
      'client_id' => $this->consumer_key,
      'client_secret' => $this->consumer_secret,
      'redirect_uri' => $this->redirectUrl()
    );

    $response = drupal_http_request($token_url, array(
      'data' => drupal_http_build_query($query),
      'method' => 'POST',
      'headers' => array(
        // this is an undocumented requirement on SF's end
        'Content-Type' => 'application/x-www-form-urlencoded'
      )
    ));

    if ($response->code != 200) {
      //@todo: deal with error
    }

    $data = drupal_json_decode($response->data);

    $this->setRefreshToken($data['refresh_token']);
    $this->setInstanceURL($data['instance_url']);
    $this->setAccessToken($data['access_token']);
  }

  /**
   * Helper to build the redirect URL for OAUTH workflow.
   *
   * @return string
   */
  private function redirectUrl() {
    return url('salesforce/oauth_callback', array(
      'absolute' => TRUE,
      'https' => TRUE
    ));
  }

  // wrapper calls around core apiCall() follow.

  /**
   * Available objects and their metadata for your organization's data.
   */
  public function objects() {
    return $this->apiCall('sobjects');
  }

}

class SalesforceException extends Exception {}
