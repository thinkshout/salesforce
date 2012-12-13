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
  public $response;

  //@todo: does this need to be configurable?
  public $rest_api_version = array(
    "label" => "Summer '12",
    "url" => "/services/data/v25.0/",
    "version" => "25.0"
  );

  /**
   * Constructor which initializes the consumer.
   *
   * @param $consumer_key
   * @param string $consumer_secret
   */
  public function __construct($consumer_key, $consumer_secret = '') {
    $this->consumer_key = $consumer_key;
    $this->consumer_secret = $consumer_secret;
  }

  /**
   * Determine if this SF instance is fully configured.
   * TODO: Consider making a test API call.
   *
   * @return bool
   */
  public function isAuthorized() {
    return !empty($this->consumer_key) && !empty($this->consumer_secret) && $this->getRefreshToken();
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

    $this->response = $this->apiHttpRequest($path, $params, $method);

    switch ($this->response->code) {
      // The session ID or OAuth token used has expired or is invalid
      case 401:
        // refresh token
        $this->refreshToken();

        // Rebuild our request and repeat request.
        $this->response = $this->apiHttpRequest($path, $params, $method);
        // Throw an error if we still have bad response.
        if (!in_array($this->response->code, array(200, 201, 204))) {
          throw new SalesforceException($this->response->error, $this->response->code);
        }

        break;
      case 200:
      case 201:
      case 204:
        // all clear
        break;
      default:
        // we have problem and no specific Salesforce error provided
        if (empty($this->response->data)) {
          throw new SalesforceException($this->response->error, $this->response->code);
        }
    }

    $data = drupal_json_decode($this->response->data);
    if (!empty($data[0]) && count($data) == 1) {
      $data = $data[0];
    }

    if (isset($data['error'])) {
      throw new SalesforceException($data['error_description'], $data['error']);
    }

    if (!empty($data['errorCode'])) {
      throw new SalesforceException($data['message'], $this->response->code);
    }

    return $data;
  }

  /**
   * Private helper to issue an SF API request.
   *
   * @param $path
   * @param $params
   * @param $method
   *
   * @return object
   */
  private function apiHttpRequest($path, $params, $method) {
    $url = $this->getInstanceURL() . $this->rest_api_version['url'] . $path;
    $headers = array(
      'Authorization' => 'OAuth ' . $this->getAccessToken(),
      'Content-type' => 'application/json'
    );
    $data = drupal_json_encode($params);
    return $this->httpRequest($url, $data, $headers, $method);
  }

  /**
   * Make the HTTP request. Wrapper around drupal_http_request().
   *
   * @param $path
   * @param $params
   * @param $method
   *
   * @return object response.
   */
  private function httpRequest($url, $data, $headers = array(), $method = 'GET') {
    // Build the request, including path and headers. Internal use.
    $options = array(
      'method' => $method,
      'headers' => $headers,
      'data' => $data
    );

    return drupal_http_request($url, $options);
  }

  /**
   * Getter and setter for SF API instance URL.
   */
  public function getInstanceURL() {
    return variable_get('salesforce_instance_url', '');
  }

  private function setInstanceURL($url) {
    variable_set('salesforce_instance_url', $url);
  }

  /**
   * Getter and setter for the access token. It's stored in session.
   */
  public function getAccessToken() {
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
    $refresh_token = $this->getRefreshToken();
    if (empty($refresh_token)) {
      throw new SalesforceException(t('There is no refresh token.'));
    }

    $data = drupal_http_build_query(array(
      'grant_type' => 'refresh_token',
      'refresh_token' => $refresh_token,
      'client_id' => $this->consumer_key,
      'client_secret' => $this->consumer_secret,
    ));

    $url = $this->login_url . '/services/oauth2/token';
    $headers = array(
      // this is an undocumented requirement on SF's end
      'Content-Type' => 'application/x-www-form-urlencoded'
    );
    $response = $this->httpRequest($url, $data, $headers, 'POST');

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
    $data = drupal_http_build_query(array(
      'code' => $code,
      'grant_type' => 'authorization_code',
      'client_id' => $this->consumer_key,
      'client_secret' => $this->consumer_secret,
      'redirect_uri' => $this->redirectUrl()
    ));

    $url = $this->login_url . '/services/oauth2/token';
    $headers = array(
      // this is an undocumented requirement on SF's end
      'Content-Type' => 'application/x-www-form-urlencoded'
    );
    $response = $this->httpRequest($url, $data, $headers, 'POST');

    $data = drupal_json_decode($response->data);

    if ($response->code != 200) {
      $error = isset($data['error_description']) ? $data['error_description'] : $response->error;
      throw new SalesforceException($error, $response->code);
    }

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
   *
   * @param array $conditions
   *   Associative array of filters to apply to the returned objects. Filters
   *   are applied after the list is returned from Salesforce.
   *
   * @return array
   */
  public function objects($conditions = array('updateable' => TRUE)) {
    $result = $this->apiCall('sobjects');

    if (!empty($conditions)) {
      foreach ($result['sobjects'] as $key => $object) {
        foreach ($conditions as $condition => $value) {
          if (!$object[$condition] == $value) {
            unset($result['sobjects'][$key]);
          }
        }
      }
    }

    return $result['sobjects'];
  }

  /**
   * Use SOQL to get objects based on query string
   *
   * @param array $conditions
   *   Associative array of filters to apply to the SOQL query
   *
   * @return
   */
  public function query($query_string) {
    $result = $this->apiCall('query?q=' . $query_string);
    return $result;
  }

  /**
   * Retrieve all the metadata for an object, including information about each
   * field, URLs, and child relationships.
   *
   * @param $name object name
   *
   * @return array
   */
  public function objectDescribe($name) {
    if (empty($name)) {
      return array();
    }
    return $this->apiCall("sobjects/{$name}/describe");
  }

  /**
   * Create a new object of the given type.
   *
   * @param $name
   * @param $params
   *
   * @return array
   *   "id" : "001D000000IqhSLIAZ",
   *   "errors" : [ ],
   *   "success" : true
   */
  public function objectCreate($name, $params) {
    return $this->apiCall("sobjects/{$name}", $params, 'POST');
  }

  /**
   * Create new records or update existing records based on the value of the
   * specified field. If the value is not unique, REST API returns a 300
   * response with the list of matching records.
   *
   * @param $name
   *   Object type name, E.g., Contact, Account.
   * @param $id
   *   Object id.
   * @param $params
   *   Fields to update.
   *
   * @return array
   *   success:
   *     "id" : "00190000001pPvHAAU",
   *     "errors" : [ ],
   *     "success" : true
   *   error:
   *     "message" : "The requested resource does not exist"
   *     "errorCode" : "NOT_FOUND"
   *
   */
  public function objectUpsert($name, $key, $value, $params) {
    $data = $this->apiCall("sobjects/{$name}/{$key}/{$value}", $params, 'PATCH');
    if ($this->response->code == 300) {
      $data['message'] = t('The value provided is not unique.');
    }
    return $data;
  }

  /**
   * Update an existing object.
   *
   * @param $name
   *   Object type name, E.g., Contact, Account.
   * @param $id
   *   Object id.
   * @param $params
   *   Fields to update.
   */
  public function objectUpdate($name, $id, $params) {
    $this->apiCall("sobjects/{$name}/{$id}", $params, 'PATCH');
  }

  /**
   * Return a full loaded Salesforce object.
   *
   * @param $name
   *   Object type name, E.g., Contact, Account.
   * @param $id
   *   Object id.
   */
  public function objectRead($name, $id) {
    return $this->apiCall("sobjects/{$name}/{$id}", array(), 'GET');
  }

  /**
   * Delete a Salesforce object.
   *
   * @param $name
   *   Object type name, E.g., Contact, Account.
   * @param $id
   *   Object id.
   */
  public function objectDelete($name, $id) {
    $this->apiCall("sobjects/{$name}/{$id}", array(), 'DELETE');
  }

  /**
   * Return a list of available resources for the configured API version.
   *
   * @return array
   *   Associative array keyed by name with a URI value.
   */
  public function listResources() {
    $resources = $this->apiCall('');
    foreach ($resources as $key => $path) {
      $items[$key] = $path;
    }
    return $items;
  }

}

class SalesforceException extends Exception {
}
