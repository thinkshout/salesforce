<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\Form\SalesforceMappingFieldsForm.
 */

namespace Drupal\salesforce_mapping\Form;

use Symfony\Component\Debug\Debug;

use Drupal\Component\Utility\NestedArray;
use Drupal\Core\Ajax\CommandInterface;
use Drupal\Core\Ajax\AjaxResponse;
use Drupal\Core\Ajax\ReplaceCommand;
use Drupal\Core\Ajax\InsertCommand;
use Drupal\Core\Form\FormBase;

/**
 * Salesforce Mapping Fields Form
 */
class SalesforceMappingFieldsForm extends SalesforceMappingFormBase {

  /**
   * Previously "Field Mapping" table on the map edit form.
   * {@inheritdoc}
   * @todo add a header with Fieldmap Property information
   */
  public function buildForm(array $form, array &$form_state) {
    $form['#entity'] = $this->entity;
    // For each field on the map, add a row to our table.
    $form['overview'] = array('#markup' => 'Field mapping overview goes here.');
    $form['field_mappings_wrapper'] = array(
      '#title' => t('Field map'),
      '#type' => 'fieldset',
      '#id' => 'edit-field-mappings-wrapper',
      '#description' => '* Key refers to an property mapped to a Salesforce external ID. if specified an UPSERT will be used to avoid duplicate data when possible.',
    );

    $field_mappings_wrapper = &$form['field_mappings_wrapper'];
    // Check to see if we have enough information to allow mapping fields.  If
    // not, tell the user what is needed in order to have the field map show up.

    $field_mappings_wrapper['field_mappings'] = array(
      '#tree' => TRUE,
      '#type' => 'table',
      // @todo there's probably a better way to tie ajax callbacks to this element than by hard-coding an HTML DOM ID here.
      '#id' => 'edit-field-mappings',
      '#header' => array(
        // @todo: there must be a better way to get two fields in the same cell than to create an extraneous column
        'drupal_field_type' => '',
        'drupal_field_type_label' => $this->t('Field type'),
        'drupal_field_value' => $this->t('Drupal field'),
        'salesforce_field' => $this->t('Salesforce field'),
        'key' => $this->t('Key') . '*',
        'direction' => $this->t('Direction'),
        'ops' => $this->t('Operations'),
      ),
    );
    $rows = &$field_mappings_wrapper['field_mappings'];

    $form['field_mappings_wrapper']['ajax_warning'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'id' => array('edit-ajax-warning'),
      ),
    );

    // @todo figure out how D8 does tokens
    // $form['field_mappings_wrapper']['token_tree'] = array(
    //   '#type' => 'container',
    //   '#attributes' => array(
    //     'id' => array('edit-token-tree'),
    //   ),
    // );
    // $form['field_mappings_wrapper']['token_tree']['tree'] = array(
    //   '#theme' => 'token_tree',
    //   '#token_types' => array($drupal_entity_type),
    //   '#global_types' => TRUE,
    // );
    $add_field_text = !empty($field_mappings) ? t('Add another field mapping') : t('Add a field mapping to get started');

    
    $form['buttons'] = array('#type' => 'container');
    $form['buttons']['field_type'] = array(
      '#title' => t('Field Type'),
      '#type' => 'select',
      '#options' => $this->get_drupal_type_options(),
      '#attributes' => array('id' => array('edit-mapping-add-field-type')),
      '#empty_option' => $this->t('- Select -'),
    );
    $form['buttons']['add'] = array(
      '#value' => $add_field_text,
      '#type' => 'submit',
      '#executes_submit_callback' => FALSE,
      '#limit_validation_errors' => array(),
      '#ajax' => array(
        'callback' => array($this, 'field_add_callback'),
        'wrapper' => 'edit-field-mappings-wrapper',
      ),
      // @todo add validation to field_add_callback()
      '#states' => array(
        'disabled' => array(
          ':input#edit-mapping-add-field-type' => array('value' => ''),
        ),
      ),
    );

    // Field mapping form.
    $field_mappings = array_filter($this->entity->get('field_mappings'));
    $has_token_type = FALSE;

    // Add a row for each saved mapping
    $delta = 0;
    foreach ($field_mappings as $delta => $value) {
      $value['delta'] = $delta;
      $rows[$delta] = $this->get_row($value, $form, $form_state);
    }

    // Apply any changes from form_state to existing fields.
    $input = array();
    if (!empty($form_state['input']['field_mappings'])) {
      $input = &$form_state['input']['field_mappings'];
    }
    while (isset($input[++$delta])) {
      $rows[$delta] = $this->get_row($input[$delta], $form, $form_state);
    }
    // @todo input does not contain the clicked button, have to go to values for
    // that. This may change?

    // Add button was clicked. See if we have a field_type value -- it's
    // required. If not, take no action. #states is already used to prevent
    // users from adding without selecting field_type. If they've worked
    // around that, they're going to have problems.
    if (!empty($form_state['values'])
    && $form_state['values']['add'] == $form_state['values']['op']
    && !empty($form_state['input']['field_type'])) {
      $rows[$delta] = $this->get_row(array(), $form, $form_state);
    }

    $entity_uri = $this->entity->uri();
    $form['actions'] = array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => $this->t('Save'),
        '#validate' => array(
          array($this, 'validate'),
        ),
        '#submit' => array(
          array($this, 'submit'),
          array($this, 'save'),
        ),
        '#button_type' => 'primary'
      ),
      'delete' => array(
        // @todo is there a better way to get this path?
        '#markup' => l($this->t('Delete'), $entity_uri['path']. '/delete', array('attributes' => array('class' => array('button-danger')))),
      ),
    );
    return $form;
  }

  /**
   * Helper function to return an empty row for the field mapping form.
   */
  private function get_row($field_configuration = array(), $form, $form_state) {
    $field_type = FALSE;
    if (empty($field_configuration) && !empty($form_state['input']['field_type'])) {
      $field_type = $form_state['input']['field_type'];
    }
    elseif (!empty($field_configuration['drupal_field_type'])) {
      $field_type = $field_configuration['drupal_field_type'];
    }
    else {
      // Can't provide a row without a field type.
      // @todo throw an exception here
      return;
    }
    $field_plugin_definition = $this->get_field_plugin($field_type);
    if (empty($field_plugin_definition)) {
      // @todo throw an exception here
      return;
    }

    $field_plugin = $this->mappingFieldManager->createInstance($field_plugin_definition['id'], $field_configuration);

    // @todo allow plugins to override forms for all these fields
    $row['drupal_field_type'] = array(
        '#type' => 'hidden',
        '#value' => $field_type,
    );
    $row['drupal_field_type_label'] = array(
        '#markup' => $field_plugin_definition['label'],
    );

    // Display the plugin config form here:
    $row['drupal_field_value'] = $field_plugin->buildConfigurationForm($form, $form_state);

    $row['salesforce_field'] = array(
      '#type' => 'select',
      '#description' => t('Select a Salesforce field to map.'),
      '#multiple' => (isset($drupal_field_type['salesforce_multiple_fields']) && $drupal_field_type['salesforce_multiple_fields']) ? TRUE : FALSE,
      '#options' => $this->get_salesforce_field_options(),
      '#default_value' => $field_plugin->config('salesforce_field'),
      '#empty_option' => $this->t('- Select -'),
    );

    $row['key'] = array(
      '#name' => 'key',
      '#type' => 'radio',
      '#default_value' => $field_plugin->config('key'),
    );

    $row['direction'] = array(
      '#type' => 'radios',
      '#options' => array(
        SALESFORCE_MAPPING_DIRECTION_DRUPAL_SF => t('Drupal to SF'),
        SALESFORCE_MAPPING_DIRECTION_SF_DRUPAL => t('SF to Drupal'),
        SALESFORCE_MAPPING_DIRECTION_SYNC => t('Sync'),
      ),
      '#required' => TRUE,
      '#default_value' => $field_plugin->config('direction') ? $field_plugin->config('direction') : SALESFORCE_MAPPING_DIRECTION_SYNC,
    );

    // @todo implement "lock/unlock" logic here:
    // @todo convert these to AJAX operations
    $operations = array('lock' => $this->t('Lock'), 'delete' => $this->t('Delete'));
    $defaults = array();
    if ($field_plugin->config('locked')) {
      $defaults = array('lock');
    }
    $row['ops'] = array(
      '#type' => 'checkboxes',
      '#options' => $operations,
      '#default_value' => $defaults,
    );
    $row['mapping_name'] = array(
        '#type' => 'value',
        '#value' => $this->entity->id(),
    );
    return $row;
  }

 /**
   * {@inheritdoc}
   */
  public function validate(array $form, array &$form_state) {
    // @todo require a "Key" radio field to be checked
    // Assign key to special "key_field" property for easy locating.

    // Transform data from the operations column into the expected schema.
    // Copy the submitted values so we don't run into problems with array
    // indexing while removing delete field mappings.
    $values = $form_state['values']['field_mappings'];
    foreach ($values as $i => $value) {
      // If a field was deleted, delete it!
      if (!empty($value['ops']['delete'])) {
        unset($form_state['values']['field_mappings'][$i]);
        continue;
      }
      $form_state['values']['field_mappings'][$i]['locked'] = !empty($value['ops']['lock']);
      unset($form_state['values']['field_mappings'][$i]['ops']);
    }
  }
 
  public function field_add_callback($form, &$form_state) {
    $response = new AjaxResponse();
    // Requires updating itself and the field map.
    $response->addCommand(new ReplaceCommand('#edit-field-mappings-wrapper', render($form['field_mappings_wrapper'])));
    return $response;
  }

  protected function get_drupal_type_options() {
    $field_plugins = $this->mappingFieldManager->getDefinitions();
    $field_type_options = array();
    foreach ($field_plugins as $field_plugin) {
      $field_type_options[$field_plugin['id']] = $field_plugin['label'];
    }
    return $field_type_options;
  }

  protected function get_field_plugin($field_type) {
    // @todo not sure if it's best practice to static cache definitions, or just
    // get them from mappingFieldManager each time.
    $field_plugins = $this->mappingFieldManager->getDefinitions();
    return $field_plugins[$field_type];
  }

  /**
   * Helper to retreive a list of fields for a given object type.
   *
   * @param string $salesforce_object_type
   *   The object type of whose fields you want to retreive.
   *
   * @return array
   *   An array of values keyed by machine name of the field with the label as
   *   the value, formatted to be appropriate as a value for #options.
   */
  protected function get_salesforce_field_options($salesforce_object_type = '') {
    if (empty($salesforce_object_type)) {
      $salesforce_object_type = $this->entity->get('salesforce_object_type');
    }
    $sfobject = $this->get_salesforce_object($salesforce_object_type);
    $sf_fields = array();
    if (isset($sfobject['fields'])) {
      foreach ($sfobject['fields'] as $sf_field) {
        $sf_fields[$sf_field['name']] = $sf_field['label'];
      }
    }
    return $sf_fields;
  }

}
