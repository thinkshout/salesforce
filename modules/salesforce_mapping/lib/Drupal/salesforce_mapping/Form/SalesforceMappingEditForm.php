<?php

/**
 * @file
 * Contains Drupal\salesforce_mapping\SalesforceMappingEditForm.
 */

namespace Drupal\salesforce_mapping\Form;

use Drupal\Core\Ajax\CommandInterface;
use Drupal\Core\Ajax\AjaxResponse;
use Drupal\Core\Ajax\ReplaceCommand;
use Drupal\Core\Ajax\InsertCommand;
// use Drupal\Core\Entity\EntityFormController;

/**
 * Salesforce Mapping Add/Edit Form
 */
class SalesforceMappingEditForm extends SalesforceMappingFormBase {

  /**
   * {@inheritdoc}
   * @todo this function is almost 200 lines. Look into leveraging core Entity
   *   interfaces like FieldsDefinition (or something). Look at breaking this up
   *   into smaller chunks.
   */
  public function form(array $form, array &$form_state) {
    $mapping = $this->entity;
    if (!$mapping->isNew()) {
      // @todo consider making Entity and SObject create-only.
      drupal_set_message(t('Changing Drupal Entity or Salesforce Object will reset field mappings. Please proceed with caution.'), 'warning');
    }
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => $this->t('Label'),
      '#default_value' => $mapping->label(),
      '#required' => TRUE,
      '#weight' => -30,
    );
    $form['id'] = array(
      '#type' => 'machine_name',
      '#required' => TRUE,
      '#default_value' => $mapping->id(),
      '#maxlength' => 255,
      '#machine_name' => array(
        'exists' => array($this->storageController, 'load'),
        'source' => array('label'),
      ),
      '#disabled' => !$mapping->isNew(),
      '#weight' => -20,
    );

    $form['drupal_entity'] = array(
      '#title' => $this->t('Drupal entity'),
      '#type' => 'details',
      '#attributes' => array(
        'id' => array('edit-drupal-entity'),
      ),
      // Gently discourage admins from breaking existing fieldmaps:
      '#collapsed' => !$mapping->isNew(),
    );

    $entity_types = $this->get_entity_type_options();
    $form['drupal_entity']['drupal_entity_type'] = array(
      '#title' => $this->t('Drupal Entity Type'),
      '#id' => 'edit-drupal-entity-type',
      '#type' => 'select',
      '#description' => $this->t('Select a Drupal entity type to map to a Salesforce object.'),
      '#options' => $entity_types,
      '#default_value' => $mapping->get('drupal_entity_type'),
      '#required' => TRUE,
      '#empty_option' => $this->t('- Select -'),
      // Bundles are based on States now. Ajax is overkill.
      // '#ajax' => array(
      //   'callback' => array($this, 'drupal_entity_type_bundle_callback'),
      //   'wrapper' => 'edit-drupal-entity',
      // ),
    );

    $form['drupal_entity']['drupal_bundle'] = array('#tree' => TRUE, '#title' => 'Drupal Bundle');
    foreach ($entity_types as $entity_type => $label) {
      $bundle_info = \Drupal::entityManager()->getBundleInfo($entity_type);
      if (empty($bundle_info)) {
        continue;
      }
      $form['drupal_entity']['drupal_bundle'][$entity_type] = array(
        '#title' => $this->t('!entity_type Bundle', array('!entity_type' => $label)),
        '#type' => 'select',
        '#empty_option' => $this->t('- Select -'),
        '#options' => array(),
        '#states' => array(
          'visible' => array(
            ':input#edit-drupal-entity-type' => array('value' => $entity_type),
          ),
          'required' => array(
            ':input#edit-drupal-entity-type, dummy1' => array('value' => $entity_type),
          ),
          'disabled' => array(
            ':input#edit-drupal-entity-type, dummy2' => array('!value' => $entity_type),
          ),
        ),
      );
      foreach ($bundle_info as $key => $info) {
        $form['drupal_entity']['drupal_bundle'][$entity_type]['#options'][$key] = $info['label'];
        if ($key == $mapping->get('drupal_bundle')) {
          $form['drupal_entity']['drupal_bundle'][$entity_type]['#default_value'] = $key;
        }
      }
    }

    $form['salesforce_object'] = array(
      '#title' => $this->t('Salesforce object'),
      '#id' => 'edit-salesforce-object',
      '#type' => 'details',
      // Gently discourage admins from breaking existing fieldmaps:
      '#collapsed' => !$mapping->isNew(),
    );

    $salesforce_object_type = '';
    if (!empty($form_state['values']) && !empty($form_state['values']['salesforce_object_type'])) {
      $salesforce_object_type = $form_state['values']['salesforce_object_type'];
    }
    elseif ($mapping->get('salesforce_object_type')) {
      $salesforce_object_type = $mapping->get('salesforce_object_type');
    }
    $form['salesforce_object']['salesforce_object_type'] = array(
      '#title' => $this->t('Salesforce Object'),
      '#id' => 'edit-salesforce-object-type',
      '#type' => 'select',
      '#description' => $this->t('Select a Salesforce object to map.'),
      '#default_value' => $salesforce_object_type,
      '#options' => $this->get_salesforce_object_type_options(),
      '#ajax' => array(
        'callback' => array($this, 'salesforce_record_type_callback'),
        'wrapper' => 'edit-salesforce-object',
      ),
      '#required' => TRUE,
      '#empty_option' => $this->t('- Select -'),
    );

    $form['salesforce_object']['salesforce_record_type'] = array(
      '#id' => 'edit-salesforce-record-type',
    );

    if ($salesforce_object_type) {
      // Check for custom record types.
      $salesforce_record_type = $mapping->get('salesforce_record_type');
      $salesforce_record_type_options = $this->get_salesforce_record_type_options($salesforce_object_type, $form_state);
      if (count($salesforce_record_type_options) > 1) {
        // There are multiple record types for this object type, so the user
        // must choose one of them.  Provide a select field.
        $form['salesforce_object']['salesforce_record_type'] = array(
          '#title' => $this->t('Salesforce Record Type'),
          '#type' => 'select',
          '#description' => $this->t('Select a Salesforce record type to map.'),
          '#default_value' => $salesforce_record_type,
          '#options' => $salesforce_record_type_options,
          '#empty_option' => $this->t('- Select -'),
          // Do not make it required to preserve graceful degradation:
          // '#required' => TRUE,
        );
      }
      else {
        // There is only one record type for this object type.  Don't bother the
        // user and just set the single record type by default.
        $form['salesforce_object']['salesforce_record_type'] = array(
          '#title' => $this->t('Salesforce Record Type'),
          '#type' => 'hidden',
          '#value' => '',
        );
      }
    }

    // @todo either change sync_triggers to human readable values, or make them work as hex flags again.
    $trigger_options = $this->get_sync_trigger_options();
    $form['sync_triggers'] = array(
      '#title' => t('Action triggers'),
      '#type' => 'checkboxes',
      '#description' => t('Select which actions on Drupal entities and Salesforce objects should trigger a synchronization. These settings are used by the salesforce_push and salesforce_pull modules respectively.'),
      '#options' => $trigger_options,
      '#required' => TRUE,
      // form.inc doesn't type-check default values. Don't pass NULL or FALSE.
      '#default_value' => $mapping->get('sync_triggers'),
    );

    $form['push_plugin'] = array(
      '#title' => t('Push Plugin'),
      '#type' => 'select',
      '#options' => $this->get_push_plugin_options(),
      '#description' => t('Choose a plugin to handle synching with Salesforce.'),
      '#default_value' => $mapping->get('push_plugin'),
      '#empty_option' => $this->t('- Select -'),
    );

    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#value' => t('Save mapping'),
      '#type' => 'submit',
    );
    return parent::form($form, $form_state);
  }

 /**
   * {@inheritdoc}
   */
  public function validate(array $form, array &$form_state) {
    parent::validate($form, $form_state);

    $values = $form_state['values'];

    $entity_type = $values['drupal_entity_type'];
    if (!empty($entity_type) && empty($values['drupal_bundle'][$entity_type])) {
      $element = &$form['drupal_entity']['drupal_bundle'][$entity_type];
      // @todo replace with Dependency Injection
      \Drupal::formBuilder()->setError($element, $this->t('!name field is required.', array('!name' => $element['#title'])));
    }

    // In case the form was submitted without javascript, we must validate the
    // salesforce record type.
    if (empty($values['salesforce_record_type'])) {
      $record_types = $this->get_salesforce_record_type_options($values['salesforce_object_type'], $form_state);
      if (count($record_types) > 1) {
        $element = &$form['salesforce_object']['salesforce_record_type'];
        drupal_set_message($this->t('!name field is required for this Salesforce Object type.', array('!name' => $element['#title'])));
        $form_state['rebuild'] = TRUE;
      }
    }
  }
 
  /**
   * {@inheritdoc}
   */
  public function submit(array $form, array &$form_state) {
    parent::submit($form, $form_state);

    // Drupal bundle is still an array, but needs to be a string.
    $entity_type = $this->entity->get('drupal_entity_type');
    $bundle = $form_state['values']['drupal_bundle'][$entity_type];
    $this->entity->set('drupal_bundle', $bundle);

    return $this->entity;
  }

  public function drupal_entity_type_bundle_callback($form, $form_state) {
    $response = new AjaxResponse();
    // Requires updating itself and the field map.
    $response->addCommand(new ReplaceCommand('#edit-salesforce-object', render($form['salesforce_object'])))->addCommand(new ReplaceCommand('#edit-salesforce-field-mappings-wrapper', render($form['salesforce_field_mappings_wrapper'])));
    return $response;
  }

  /**
   * Ajax callback for salesforce_mapping_form() salesforce record type.
   */
  public function salesforce_record_type_callback($form, $form_state) {
    $response = new AjaxResponse();
    // Requires updating itself and the field map.
    $response->addCommand(new ReplaceCommand('#edit-salesforce-object', render($form['salesforce_object'])))->addCommand(new ReplaceCommand('#edit-salesforce-field-mappings-wrapper', render($form['salesforce_field_mappings_wrapper'])));
    return $response;
  }

  /**
   * Ajax callback for salesforce_mapping_form() field CRUD
   */
  public function field_callback($form, $form_state) {
    $response = new AjaxResponse();
    $response->addCommand(new ReplaceCommand('#edit-salesforce-field-mappings-wrapper', render($form['salesforce_field_mappings_wrapper'])));
    return $response;
  }


  /**
   * Return a list of Drupal entity types for mapping.
   *
   * @return array
   *   An array of values keyed by machine name of the entity with the label as
   *   the value, formatted to be appropriate as a value for #options.
   */
  protected function get_entity_type_options() {
    $options = array();
    $entity_info = \Drupal::entityManager()->getDefinitions();

    // For now, let's only concern ourselves with fieldable entities. This is an
    // arbitrary restriction, but otherwise there would be dozens of entities,
    // making this options list unwieldy.
    foreach ($entity_info as $info) {
      if (!$info['fieldable']) {
        continue;
      }
      $options[$info['id']] = $info['label'];
    }
    return $options;
  }

  /**
   * Helper to retreive a list of object type options.
   *
   * @param array $form_state
   *   Current state of the form to store and retreive results from to minimize
   *   the need for recalculation.
   *
   * @return array
   *   An array of values keyed by machine name of the object with the label as
   *   the value, formatted to be appropriate as a value for #options.
   */
  protected function get_salesforce_object_type_options() {
    $sfobject_options = array();
    // No need to cache here: Salesforce::objects() implements its own caching.
    $sfapi = salesforce_get_api();
    // Note that we're filtering SF object types to a reasonable subset.
    $sfobjects = $sfapi->objects(array(
      'updateable' => TRUE,
      'triggerable' => TRUE,
    ));
    foreach ($sfobjects as $object) {
      $sfobject_options[$object['name']] = $object['label'];
    }
    return $sfobject_options;
  }

  /**
   * Helper to retreive a list of record type options for a given object type.
   *
   * @param string $salesforce_object_type
   *   The object type of whose records you want to retreive.
   * @param array $form_state
   *   Current state of the form to store and retreive results from to minimize
   *   the need for recalculation.
   *
   * @return array
   *   An array of values keyed by machine name of the record with the label as
   *   the value, formatted to be appropriate as a value for #options.
   */
  protected function get_salesforce_record_type_options($salesforce_object_type) {
    $sf_types = array();
    $sfobject = $this->get_salesforce_object($salesforce_object_type);
    if (isset($sfobject['recordTypeInfos'])) {
      foreach ($sfobject['recordTypeInfos'] as $type) {
        $sf_types[$type['recordTypeId']] = $type['name'];
      }
    }
    return $sf_types;
  }

  /**
   * Return form options for available sync triggers.
   *
   * @return array
   *   Array of sync trigger options keyed by their machine name with their 
   *   label as the value.
   */
  protected function get_sync_trigger_options() {
    return array(
      SALESFORCE_MAPPING_SYNC_DRUPAL_CREATE => t('Drupal entity create'),
      SALESFORCE_MAPPING_SYNC_DRUPAL_UPDATE => t('Drupal entity update'),
      SALESFORCE_MAPPING_SYNC_DRUPAL_DELETE => t('Drupal entity delete'),
      SALESFORCE_MAPPING_SYNC_SF_CREATE => t('Salesforce object create'),
      SALESFORCE_MAPPING_SYNC_SF_UPDATE => t('Salesforce object update'),
      SALESFORCE_MAPPING_SYNC_SF_DELETE => t('Salesforce object delete'),
    );
  }

  protected function get_push_plugin_options() {
    $field_plugins = $this->pushPluginManager->getDefinitions();
    $field_type_options = array();
    foreach ($field_plugins as $field_plugin) {
      $field_type_options[$field_plugin['id']] = $field_plugin['label'];
    }
    return $field_type_options;
  }

}
