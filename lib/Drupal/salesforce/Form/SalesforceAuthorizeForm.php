<?php

/**
 * @file
 * Contains \Drupal\salesforce\Form\SalesforceAuthorizeForm.
 */

namespace Drupal\salesforce\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\salesforce\Salesforce;

/**
 * Creates authorization form for Salesforce.
 */
class SalesforceAuthorizeForm extends ConfigFormBase {

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
      '#markup' => t('Authorize this website to communicate with Salesforce by entering the consumer key and secret from a remote application. Clicking authorize will redirect you to Salesforce where you will be asked to grant access.'),
    );

    $form['consumer_key'] = array(
      '#title' => t('Salesforce consumer key'),
      '#type' => 'textfield',
      '#description' => t('Consumer key of the Salesforce remote application you want to grant access to'),
      '#required' => TRUE,
      '#default_value' => $config->get('consumer_key'),
    );
    $form['consumer_secret'] = array(
      '#title' => t('Salesforce consumer secret'),
      '#type' => 'textfield',
      '#description' => t('Consumer secret of the Salesforce remote application you want to grant access to'),
      '#required' => TRUE,
      '#default_value' => $config->get('consumer_secret'),
    );

    // If we're authenticated, show a list of available REST resources.
    if ($config->get('consumer_key') && $config->get('consumer_secret')) {
      $sfapi = new Salesforce($config->get('consumer_key'), $config->get('consumer_secret'));
      // If fully configured, attempt to connect to Salesforce and return a list
      // of resources.
      if ($sfapi->isAuthorized()) {
        try {
          $resources = $sfapi->listResources();
          foreach ($resources as $key => $path) {
            $items[] = $key . ': ' . $path;
          }
          $form['resources'] = array(
            '#title' => t('Your Salesforce instance is authorized and has access to the following resources:'),
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

    $salesforce = new Salesforce($config->get('consumer_key'), $config->get('consumer_secret'));
    try {
      return $salesforce->getAuthorizationCode();
    }
    catch (SalesforceException $e) {
      // Set form error
    }

    parent::submitForm($form, $form_state);
  }

}
