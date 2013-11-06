<?php

/**
 * @file
 * Contains \Drupal\salesforce\Form\SalesforceAuthorizeForm.
 */

namespace Drupal\salesforce\Form;

use Drupal\Core\Config\ConfigFactory;
use Drupal\Core\Form\ConfigFormBase;
use Drupal\salesforce\Salesforce;
use Drupal\salesforce\SalesforceException;
use Guzzle\Http\ClientInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Creates authorization form for Salesforce.
 */
class SalesforceAuthorizeForm extends ConfigFormBase {

  protected $httpClient;

  /**
   * Constructs a \Drupal\system\ConfigFormBase object.
   *
   * @param \Drupal\Core\Config\ConfigFactory $config_factory
   *   The factory for configuration objects.
   * @param \Drupal\Core\Config\Context\ContextInterface $context
   *   The configuration context to use.
   * @param \Drupal\Core\Config\Context\ContextInterface $context
   *   The configuration context to use.
   * @param \Guzzle\Http\ClientInterface $http_client
   *   The Guzzle HTTP client.
   */
  public function __construct(ConfigFactory $config_factory, ContextInterface $context, ClientInterface $http_client) {
    $this->configFactory = $config_factory;
    $this->configFactory->enterContext($context);
    $this->httpClient = $http_client;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('config.factory'),
      $container->get('config.context.free'),
      $container->get('http_default_client')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getFormID() {
    return 'salesforce_oauth';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    $config = $this->config('salesforce.settings');

    $form['message'] = array(
      '#type' => 'item',
      '#markup' => $this->t('Authorize this website to communicate with Salesforce by entering the consumer key and secret from a remote application. Clicking authorize will redirect you to Salesforce where you will be asked to grant access.'),
    );

    $form['consumer_key'] = array(
      '#title' => $this->t('Salesforce consumer key'),
      '#type' => 'textfield',
      '#description' => $this->t('Consumer key of the Salesforce remote application you want to grant access to'),
      '#required' => TRUE,
      '#default_value' => $config->get('consumer_key'),
    );
    $form['consumer_secret'] = array(
      '#title' => $this->t('Salesforce consumer secret'),
      '#type' => 'textfield',
      '#description' => $this->t('Consumer secret of the Salesforce remote application you want to grant access to'),
      '#required' => TRUE,
      '#default_value' => $config->get('consumer_secret'),
    );

    // If we're authenticated, show a list of available REST resources.
    if ($config->get('consumer_key') && $config->get('consumer_secret')) {
      $sfapi = new Salesforce($this->httpClient, $this->configFactory);

      // If fully configured, attempt to connect to Salesforce and return a list
      // of resources.
      if ($sfapi->isAuthorized()) {
        try {
          $resources = $sfapi->listResources();
          foreach ($resources as $key => $path) {
            $items[] = $key . ': ' . $path;
          }
          $form['resources'] = array(
            '#title' => $this->t('Your Salesforce instance is authorized and has access to the following resources:'),
            '#type' => 'item',
            '#markup' => theme('item_list', array('items' => $items)),
          );
        }
        catch(SalesforceException $e) {
          salesforce_set_message($e->getMessage(), 'warning');
        }
      }
      else {
        salesforce_set_message(t('Salesforce needs to be authorized to connect to this website.'), 'error');
      }
    }

    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, array &$form_state) {
    $config = $this->config('salesforce.settings')
      ->set('consumer_key', $form_state['values']['consumer_key'])
      ->set('consumer_secret', $form_state['values']['consumer_secret'])
      ->save();

    $salesforce = new Salesforce($this->httpClient, $this->configFactory);
    try {
      return $salesforce->getAuthorizationCode();
    }
    catch (SalesforceException $e) {
      // Set form error
    }

    parent::submitForm($form, $form_state);
  }

}
