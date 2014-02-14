<?php
/**
 * @file
 * Objects, properties, and methods to communicate with the Salesforce REST API
 */

/**
 * Ability to authorize and communicate with the Salesforce REST API.
 */
class Salesforce {

  public $response;

  /**
   * Constructor which initializes the consumer.
   *
   * @param string $consumer_key
   *   Salesforce key to connect to your Salesforce instance.
   * @param string $consumer_secret
   *   Salesforce secret to connect to your Salesforce instance.
   */
  public function __construct($consumer_key, $consumer_secret = '') {
    $this->consumer_key = $consumer_key;
    $this->consumer_secret = $consumer_secret;
    $this->login_url = variable_get('salesforce_endpoint', 'https://login.salesforce.com');

    // @TODO: Does this need to be configurable?
    $this->rest_api_version = array(
      "label" => "Spring '13",
      "url" => "/services/data/v27.0/",
      "version" => "27.0",
    );
  }

  /**
   * Determine if this SF instance is fully configured.
   *
   * @TODO: Consider making a test API call.
   */
  public function isAuthorized() {
    return !empty($this->consumer_key) && !empty($this->consumer_secret) && $this->getRefreshToken();
  }

  /**
   * Make a call to the Salesforce REST API.
   *
   * @param string $path
   *   Path to resource.
   * @param array $params
   *   Parameters to provide.
   * @param string $method
   *   Method to initiate the call, such as GET or POST.  Defaults to GET.
   *
   * @return mixed
   *   The requested response.
   *
   * @throws SalesforceException
   */
  public function apiCall($path, $params = array(), $method = 'GET') {
    if (!$this->getAccessToken()) {
      $this->refreshToken();
    }

    $this->response = $this->apiHttpRequest($path, $params, $method);

    switch ($this->response->code) {
      // The session ID or OAuth token used has expired or is invalid.
      case 401:
        // Refresh token.
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
        // All clear.
        break;

      default:
        // We have problem and no specific Salesforce error provided.
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
   * @param string $path
   *   Path to resource.
   * @param array $params
   *   Parameters to provide.
   * @param string $method
   *   Method to initiate the call, such as GET or POST.  Defaults to GET.
   *
   * @return object
   *   The requested data.
   */
  protected function apiHttpRequest($path, $params, $method) {
    $url = $this->getApiEndPoint() . $path;
    $headers = array(
      'Authorization' => 'OAuth ' . $this->getAccessToken(),
      'Content-type' => 'application/json',
    );
    $data = drupal_json_encode($params);
    return $this->httpRequest($url, $data, $headers, $method);
  }

  /**
   * Make the HTTP request. Wrapper around drupal_http_request().
   *
   * @param string $url
   *   Path to make request from.
   * @param array $data
   *   The request body.
   * @param array $headers
   *   Request headers to send as name => value.
   * @param string $method
   *   Method to initiate the call, such as GET or POST.  Defaults to GET.
   *
   * @return object
   *   Salesforce response object.
   */
  protected function httpRequest($url, $data, $headers = array(), $method = 'GET') {
    // Build the request, including path and headers. Internal use.
    $options = array(
      'method' => $method,
      'headers' => $headers,
      'data' => $data,
    );

    return drupal_http_request($url, $options);
  }

  /**
   * Get the API end point for a given type of the API.
   *
   * @param string $api_type
   *   E.g., rest, partner, enterprise.
   *
   * @return string
   *   Complete URL endpoint for API access.
   */
  public function getApiEndPoint($api_type = 'rest') {
    $url = &drupal_static(__FUNCTION__ . $api_type);
    if (!isset($url)) {
      $identity = $this->getIdentity();
      $url = str_replace('{version}', $this->rest_api_version['version'], $identity['urls'][$api_type]);
    }
    return $url;
  }

  /**
   * Get the SF instance URL. Useful for linking to objects.
   */
  public function getInstanceUrl() {
    return variable_get('salesforce_instance_url', '');
  }

  /**
   * Set the SF instanc URL.
   *
   * @param string $url
   *   URL to set.
   */
  protected function setInstanceUrl($url) {
    variable_set('salesforce_instance_url', $url);
  }

  /**
   * Get the access token.
   */
  public function getAccessToken() {
    return isset($_SESSION['salesforce_access_token']) ? $_SESSION['salesforce_access_token'] : FALSE;
  }

  /**
   * Set the access token.
   *
   * It is stored in session.
   *
   * @param string $token
   *   Access token from Salesforce.
   */
  protected function setAccessToken($token) {
    $_SESSION['salesforce_access_token'] = $token;
  }

  /**
   * Get refresh token.
   */
  protected function getRefreshToken() {
    return variable_get('salesforce_refresh_token', '');
  }

  /**
   * Set refresh token.
   *
   * @param string $token
   *   Refresh token from Salesforce.
   */
  protected function setRefreshToken($token) {
    variable_set('salesforce_refresh_token', $token);
  }

  /**
   * Refresh access token based on the refresh token. Updates session variable.
   *
   * @throws SalesforceException
   */
  protected function refreshToken() {
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
      // This is an undocumented requirement on Salesforce's end.
      'Content-Type' => 'application/x-www-form-urlencoded',
    );
    $response = $this->httpRequest($url, $data, $headers, 'POST');

    if ($response->code != 200) {
      // @TODO: Deal with error better.
      throw new SalesforceException(t('Unable to get a Salesforce access token.'), $response->code);
    }

    $data = drupal_json_decode($response->data);

    if (isset($data['error'])) {
      throw new SalesforceException($data['error_description'], $data['error']);
    }

    $this->setAccessToken($data['access_token']);
    $this->setIdentity($data['id']);
    $this->setInstanceUrl($data['instance_url']);
  }

  /**
   * Retrieve and store the Salesforce identity given an ID url.
   *
   * @param string $id
   *   Identity URL.
   *
   * @throws SalesforceException
   */
  protected function setIdentity($id) {
    $headers = array(
      'Authorization' => 'OAuth ' . $this->getAccessToken(),
      'Content-type' => 'application/json',
    );
    $response = $this->httpRequest($id, NULL, $headers);
    if ($response->code != 200) {
      throw new SalesforceException(t('Unable to access identity service.'), $response->code);
    }
    $data = drupal_json_decode($response->data);
    variable_set('salesforce_identity', $data);
  }

  /**
   * Return the Salesforce identity, which is stored in a variable.
   *
   * @return array
   *   Returns FALSE is no identity has been stored.
   */
  public function getIdentity() {
    return variable_get('salesforce_identity', FALSE);
  }

  /**
   * OAuth step 1: Redirect to Salesforce and request and authorization code.
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
   * OAuth step 2: Exchange an authorization code for an access token.
   *
   * @param string $code
   *   Code from Salesforce.
   */
  public function requestToken($code) {
    $data = drupal_http_build_query(array(
      'code' => $code,
      'grant_type' => 'authorization_code',
      'client_id' => $this->consumer_key,
      'client_secret' => $this->consumer_secret,
      'redirect_uri' => $this->redirectUrl(),
    ));

    $url = $this->login_url . '/services/oauth2/token';
    $headers = array(
      // This is an undocumented requirement on SF's end.
      'Content-Type' => 'application/x-www-form-urlencoded',
    );
    $response = $this->httpRequest($url, $data, $headers, 'POST');

    $data = drupal_json_decode($response->data);

    if ($response->code != 200) {
      $error = isset($data['error_description']) ? $data['error_description'] : $response->error;
      throw new SalesforceException($error, $response->code);
    }

    $this->setRefreshToken($data['refresh_token']);
    $this->setAccessToken($data['access_token']);
    $this->setIdentity($data['id']);
    $this->setInstanceUrl($data['instance_url']);
  }

  /**
   * Helper to build the redirect URL for OAUTH workflow.
   *
   * @return string
   *   Redirect URL.
   */
  protected function redirectUrl() {
    return url('salesforce/oauth_callback', array(
      'absolute' => TRUE,
      'https' => TRUE,
    ));
  }

  /**
   * @defgroup salesforce_apicalls Wrapper calls around core apiCall()
   */

  /**
   * Available objects and their metadata for your organization's data.
   *
   * @param array $conditions
   *   Associative array of filters to apply to the returned objects. Filters
   *   are applied after the list is returned from Salesforce.
   * @param bool $reset
   *   Whether to reset the cache and retrieve a fresh version from Salesforce.
   *
   * @return array
   *   Available objects and metadata.
   *
   * @addtogroup salesforce_apicalls
   */
  public function objects($conditions = array('updateable' => TRUE), $reset = FALSE) {
    $cache = cache_get('salesforce_objects');
    // Force the recreation of the cache when it is older than 5 minutes.
    if ($cache && REQUEST_TIME < ($cache->created + 300) && !$reset) {
      $result = $cache->data;
    }
    else {
      $result = $this->apiCall('sobjects');
      // Allow the cache to clear at any time by not setting an expire time.
      cache_set('salesforce_objects', $result, 'cache', CACHE_TEMPORARY);
    }

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
   * Use SOQL to get objects based on query string.
   *
   * @param SalesforceSelectQuery $query
   *   The constructed SOQL query.
   *
   * @return array
   *   Array of Salesforce objects that match the query.
   *
   * @addtogroup salesforce_apicalls
   */
  public function query(SalesforceSelectQuery $query) {
    drupal_alter('salesforce_query', $query);
    // Casting $query as a string calls SalesforceSelectQuery::__toString().
    $result = $this->apiCall('query?q=' . (string) $query);
    return $result;
  }

  /**
   * Retreieve all the metadata for an object.
   *
   * @param string $name
   *   Object type name, E.g., Contact, Account, etc.
   * @param bool $reset
   *   Whether to reset the cache and retrieve a fresh version from Salesforce.
   *
   * @return array
   *   All the metadata for an object, including information about each field,
   *   URLs, and child relationships.
   *
   * @addtogroup salesforce_apicalls
   */
  public function objectDescribe($name, $reset = FALSE) {
    if (empty($name)) {
      return array();
    }
    $cache = cache_get($name, 'cache_salesforce_object');
    // Force the recreation of the cache when it is older than 5 minutes.
    if ($cache && REQUEST_TIME < ($cache->created + 300) && !$reset) {
      return $cache->data;
    }
    else {
      $object = $this->apiCall("sobjects/{$name}/describe");
      // Sort field properties, because salesforce API always provides them in a
      // random order. We sort them so that stored and exported data are
      // standardized and predictable.
      foreach($object['fields'] as &$field) {
        ksort($field);
        if (!empty($field['picklistValues'])) {
          foreach($field['picklistValues'] as &$picklist_value) {
            ksort($picklist_value);
          }
        }
      }

      // Allow the cache to clear at any time by not setting an expire time.
      cache_set($name, $object, 'cache_salesforce_object', CACHE_TEMPORARY);
      return $object;
    }
  }

  /**
   * Create a new object of the given type.
   *
   * @param string $name
   *   Object type name, E.g., Contact, Account, etc.
   * @param array $params
   *   Values of the fields to set for the object.
   *
   * @return array
   *   "id" : "001D000000IqhSLIAZ",
   *   "errors" : [ ],
   *   "success" : true
   *
   * @addtogroup salesforce_apicalls
   */
  public function objectCreate($name, $params) {
    return $this->apiCall("sobjects/{$name}", $params, 'POST');
  }

  /**
   * Create new records or update existing records.
   *
   * The new records or updated records are based on the value of the specified
   * field.  If the value is not unique, REST API returns a 300 response with
   * the list of matching records.
   *
   * @param string $name
   *   Object type name, E.g., Contact, Account.
   * @param string $key
   *   The field to check if this record should be created or updated.
   * @param string $value
   *   The value for this record of the field specified for $key.
   * @param array $params
   *   Values of the fields to set for the object.
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
   * @addtogroup salesforce_apicalls
   */
  public function objectUpsert($name, $key, $value, $params) {
    // If key is set, remove from $params to avoid UPSERT errors.
    if (isset($params[$key])) {
      unset($params[$key]);
    }

    $data = $this->apiCall("sobjects/{$name}/{$key}/{$value}", $params, 'PATCH');
    if ($this->response->code == 300) {
      $data['message'] = t('The value provided is not unique.');
    }
    return $data;
  }

  /**
   * Update an existing object.
   *
   * @param string $name
   *   Object type name, E.g., Contact, Account.
   * @param string $id
   *   Salesforce id of the object.
   * @param array $params
   *   Values of the fields to set for the object.
   *
   * @addtogroup salesforce_apicalls
   */
  public function objectUpdate($name, $id, $params) {
    $this->apiCall("sobjects/{$name}/{$id}", $params, 'PATCH');
  }

  /**
   * Return a full loaded Salesforce object.
   *
   * @param string $name
   *   Object type name, E.g., Contact, Account.
   * @param string $id
   *   Salesforce id of the object.
   *
   * @return object
   *   Object of the requested Salesforce object.
   *
   * @addtogroup salesforce_apicalls
   */
  public function objectRead($name, $id) {
    return $this->apiCall("sobjects/{$name}/{$id}", array(), 'GET');
  }

  /**
   * Return a full loaded Salesforce object from External ID.
   *
   * @param string $name
   *   Object type name, E.g., Contact, Account.
   * @param string $field
   *   Salesforce external id field name.
   * @param string $value
   *   Value of external id.
   *
   * @return object
   *   Object of the requested Salesforce object.
   *
   * @addtogroup salesforce_apicalls
   */
  public function objectReadbyExternalId($name, $field, $value) {
    return $this->apiCall("sobjects/{$name}/{$field}/{$value}");
  }

  /**
   * Delete a Salesforce object.
   *
   * @param string $name
   *   Object type name, E.g., Contact, Account.
   * @param string $id
   *   Salesforce id of the object.
   *
   * @addtogroup salesforce_apicalls
   */
  public function objectDelete($name, $id) {
    $this->apiCall("sobjects/{$name}/{$id}", array(), 'DELETE');
  }

  /**
   * Return a list of available resources for the configured API version.
   *
   * @return array
   *   Associative array keyed by name with a URI value.
   *
   * @addtogroup salesforce_apicalls
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
