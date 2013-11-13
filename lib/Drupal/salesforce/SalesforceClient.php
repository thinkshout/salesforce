<?php

/**
 * @file
 * Contains \Drupal\salesforce\SalesforceClient.
 */

namespace Drupal\salesforce;

use Drupal\Core\Cache\CacheBackendInterface;
use Drupal\Core\Config\ConfigFactory;
use Guzzle\Http\Exception\RequestException;
use Guzzle\Http\ClientInterface;
use Symfony\Component\HttpFoundation\RedirectResponse;

/**
 * Objects, properties, and methods to communicate with the Salesforce REST API.
 */
class SalesforceClient {

  public $response;
  protected $httpClient;
  protected $configFactory;
  private $config;

  /**
   * Constructor which initializes the consumer.
   * @param \Drupal\Core\Config\ConfigFactory $config_factory
   *   The config factory
   * @param \Guzzle\Http\ClientInterface $http_client
   *   The config factory
   */
  public function __construct(ClientInterface $http_client, ConfigFactory $config_factory) {
    $this->configFactory = $config_factory;
    $this->httpClient = $http_client;
    $this->config = $this->configFactory->get('salesforce.settings');
  }

  // Don't need this?
  // /**
  //  * {@inheritdoc}
  //  */
  // public static function create(ContainerInterface $container) {
  //   return new static(
  //     $container->get('http_default_client'),
  //     $container->get('')
  //   );
  // }

  /**
   * Determine if this SF instance is fully configured.
   *
   * @TODO: Consider making a test API call.
   */
  public function isAuthorized() {
    return $this->getConsumerKey() && $this->getConsumerSecret() && $this->getRefreshToken();
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

    try {
      $this->response = $this->apiHttpRequest($path, $params, $method);
    }
    catch (RequestException $e) {
      // A RequestException gets thrown if the response has any error status.
      $this->response = $e->getRequest()->getResponse();
    }

    if (!is_object($this->response)) {
      throw new SalesforceException('Unknown error occurred during API call');
    }

    switch ($this->response->getStatusCode()) {
      case 401:
        // The session ID or OAuth token used has expired or is invalid: refresh
        // token. If refreshToken() throws an exception, or if apiHttpRequest()
        // throws anything but a RequestException, let it bubble up.
        $this->refreshToken();
        try {
          $this->response = $this->apiHttpRequest($path, $params, $method);
        }
        catch (RequestException $e) {
          $this->response = $e->getRequest()->getResponse();
          throw new SalesforceException($this->response->getReasonPhrase(), $this->response->getStatusCode());
        }
        break;
      case 200:
      case 201:
      case 204:
        // All clear.
        break;

      default:
        // We have problem and no specific Salesforce error provided.
        if (empty($this->response)) {
          throw new SalesforceException('Unknown error occurred during API call');
        }
    }

    try {
      $data = $this->response->json();
    }
    catch (\RuntimeException $e) {
      throw new SalesforceException('Unable to parse API response.');
    }

    if (!empty($data[0]) && count($data) == 1) {
      $data = $data[0];
    }

    if (isset($data['error'])) {
      throw new SalesforceException($data['error_description'], $data['error']);
    }

    if (!empty($data['errorCode'])) {
      throw new SalesforceException($data['message'], $this->response->getStatusCode());
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
    if (!$this->getAccessToken()) {
      throw new SalesforceException('Missing OAuth Token');
    }
    $url = $this->getApiEndPoint() . $path;
    $headers = array(
      'Authorization' => 'OAuth ' . $this->getAccessToken(),
      'Content-type' => 'application/json',
    );
    $data = NULL;
    if (!empty($params)) {
      // @todo: convert this into Dependency Injection
      $data =  \Json::encode($params);
    }
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
   * @throws RequestException
   *
   * @return object
   *   Salesforce response object.
   */
  protected function httpRequest($url, $data = NULL, $headers = array(), $method = 'GET') {
    // Build the request, including path and headers. Internal use.
    $method = $method == 'POST' ? 'post' : 'get';
    $request = $this->httpClient->$method($url, $headers, $data);
    $request->send();
    return $request->getResponse();
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
      if (is_string($identity)) {
        $url = $identity;
      }
      elseif (isset($identity['urls'][$api_type])) {
        $url = $identity['urls'][$api_type];
      }
      $url = str_replace('{version}', $this->config->get('rest_api_version.version'), $url);
    }
    return $url;
  }

  public function getConsumerKey() {
    return $this->config->get('consumer_key');
  }

  public function setConsumerKey($value) {
    return $this->config->set('consumer_key', $value)->save();
  }

  public function getConsumerSecret() {
    return $this->config->get('consumer_secret');
  }

  public function setConsumerSecret($value) {
    return $this->config->set('consumer_secret', $value)->save();
  }
  /**
   * Get the SF instance URL. Useful for linking to objects.
   */
  public function getInstanceUrl() {
    return $this->config->get('instance_url');
  }

  /**
   * Set the SF instanc URL.
   *
   * @param string $url
   *   URL to set.
   */
  protected function setInstanceUrl($url) {
    $this->config->set('instance_url', $url)->save();
  }

  /**
   * Get the access token.
   */
  public function getAccessToken() {
    // @todo There is probably a better way to do this in D8.
    // Why not put it in settings?
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
    // @todo There is probably a better way to do this in D8.
    // Why not put it in settings?
    $_SESSION['salesforce_access_token'] = $token;
  }

  /**
   * Get refresh token.
   */
  protected function getRefreshToken() {
    return $this->config->get('refresh_token');
  }

  /**
   * Set refresh token.
   *
   * @param string $token
   *   Refresh token from Salesforce.
   */
  protected function setRefreshToken($token) {
    $this->config->set('refresh_token', $token)->save();
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
      'client_id' => $this->getConsumerKey(),
      'client_secret' => $this->getConsumerSecret(),
    ));

    $url = $this->config->get('login_url') . '/services/oauth2/token';
    $headers = array(
      // This is an undocumented requirement on Salesforce's end.
      'Content-Type' => 'application/x-www-form-urlencoded',
    );
    $response = $this->httpRequest($url, $data, $headers, 'POST');

    if ($response->isError()) {
      // @TODO: Deal with error better.
      throw new SalesforceException(t('Unable to get a Salesforce access token.'), $response->getStatusCode());
    }

    try {
      $data = $response->json();
    }
    catch (\RuntimeException $e) {
      throw new SalesforceException($e->getMessage(), $e->getCode());
    }

    if (isset($data['error'])) {
      throw new SalesforceException($data['error_description'], $data['error']);
    }

    $this->setAccessToken($data['access_token']);
    $this->setIdentity($data['id']);
    $this->config
      ->set('instance_url', $data['instance_url'])
      ->save();
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
    if ($response->isError()) {
      throw new SalesforceException(t('Unable to access identity service.'), $response->getStatusCode());
    }

    try {
      $data = $response->json();
    }
    catch (\RuntimeException $e) {
      throw new SalesforceException($e->getMessage(), $e->getCode());
    }

    $this->config->set('identity', $data)->save();
  }

  /**
   * Return the Salesforce identity, which is stored in a variable.
   *
   * @return array
   *   Returns FALSE is no identity has been stored.
   */
  public function getIdentity() {
    return $this->config->get('identity');
  }

  /**
   * OAuth step 1: Redirect to Salesforce and request and authorization code.
   */
  public function getAuthorizationCode() {
    $path = $this->config->get('login_url') . '/services/oauth2/authorize';
    $query = array(
      'redirect_uri' => $this->redirectUrl(),
      'response_type' => 'code',
      'client_id' => $this->getConsumerKey(),
    );

    $response = new RedirectResponse(url($path, array('query' => $query, 'absolute' => TRUE)));
    $response->send();
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
      'client_id' => $this->getConsumerKey(),
      'client_secret' => $this->getConsumerSecret(),
      'redirect_uri' => $this->redirectUrl(),
    ));
    $url = $this->config->get('login_url') . '/services/oauth2/token';
    $headers = array(
      // This is an undocumented requirement on SF's end.
      'Content-Type' => 'application/x-www-form-urlencoded',
    );
    $response = $this->httpRequest($url, $data, $headers, 'POST');

    if ($response->isError()) {
      throw new SalesforceException($response->getReasonPhrase(), $response->getStatusCode());
    }

    try {
      $data = $response->json();
    }
    catch (\RuntimeException $e) {
      throw new SalesforceException($e->getMessage(), $e->getCode());
    }

    $this->setAccessToken($data['access_token']);
    $this->setIdentity($data['id']);
    $this->config
      ->set('refresh_token', $data['refresh_token'])
      ->set('instance_url', $data['instance_url'])
      ->save();
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
    $cache = cache()->get('salesforce:objects');
    // Force the recreation of the cache when it is older than 5 minutes.
    if ($cache && REQUEST_TIME < ($cache->created + 300) && !$reset) {
      $result = $cache->data;
    }
    else {
      $result = $this->apiCall('sobjects');

      // Allow the cache to clear at any time by not setting an expire time.
      // CACHE_TEMPORARY has been removed. Using 'content' tag to replicate
      // old functionality.
      // @see https://drupal.org/node/1534648
      cache()->set('salesforce:objects', $result, CacheBackendInterface::CACHE_PERMANENT, array('salesforce' => TRUE, 'content' => TRUE));
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
    $cache = cache()->get('salesforce:object:' . $name);
    // Force the recreation of the cache when it is older than 5 minutes.
    if ($cache && REQUEST_TIME < ($cache->created + 300) && !$reset) {
      return $cache->data;
    }
    else {
      $object = $this->apiCall("sobjects/{$name}/describe");
      // Allow the cache to clear at any time by not setting an expire time.
      // CACHE_TEMPORARY has been removed. Using 'content' tag to replicate
      // old functionality. @see https://drupal.org/node/1534648
      cache()->set('salesforce:object:' . $name, $object, CacheBackendInterface::CACHE_PERMANENT, array('salesforce' => TRUE, 'content' => TRUE));
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
