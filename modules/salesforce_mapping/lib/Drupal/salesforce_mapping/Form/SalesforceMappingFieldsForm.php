<?php

/**
 * @file
 * Contains Drupal\salesforce_mapping\SalesforceMappingFieldsForm.
 */

namespace Drupal\salesforce_mapping\Form;

use Drupal\Core\Ajax\CommandInterface;
use Drupal\Core\Ajax\AjaxResponse;
use Drupal\Core\Ajax\ReplaceCommand;
use Drupal\Core\Ajax\InsertCommand;
// use Drupal\Core\Entity\EntityFormController;

/**
 * Salesforce Mapping Fields Form .
 */
class SalesforceMappingFieldsForm extends SalesforceMappingFormBase {

  /**
   * Previously "Field Mapping" table on the map edit form.
   * {@inheritdoc}
   * @todo add a header with Fieldmap Property information
   */
  public function form(array $form, array &$form_state) {
    $form['salesforce_field_mappings_wrapper'] = array(
      '#title' => t('Field map'),
      '#type' => 'fieldset',
      '#id' => 'edit-salesforce-field-mappings-wrapper',
      '#description' => '* Key refers to an property mapped to a Salesforce external ID. if specified an UPSERT will be used to avoid duplicate data when possible.',
    );
    $field_mappings_wrapper = &$form['salesforce_field_mappings_wrapper'];
    // Check to see if we have enough information to allow mapping fields.  If
    // not, tell the user what is needed in order to have the field map show up.

    $field_mappings_wrapper['salesforce_field_mappings'] = array(
      '#type' => 'table',
      '#tree' => TRUE,
      '#header' => array(
        'drupal_field' => t('Drupal field'),
        'salesforce_field' => t('Salesforce field'),
        'key' => t('Key') . '*',
        'direction' => t('Direction'),
        'delete_field_mapping' => t('Delete'),
      ),
      '#attributes' => array(
        'id' => array('edit-salesforce-field-mappings'),
      ),
      '#tableselect' => TRUE,
    );
    $field_mappings_table = &$field_mappings_wrapper['salesforce_field_mappings'];

    // Field mapping form.
    $field_mappings = array($this->entity->get('field_mappings'));
    $has_token_type = FALSE;

    // Add a row for each mapping
    foreach ($field_mappings as $delta => $value) {
      $row_id = 'edit-salesforce-field-mappings-' . $delta;
      $table[$delta] = array(
        '#attributes' => array(
          'id' => array($row_id),
        ),
      );
      $row = &$field_mappings_table[$delta];

      $row['drupal_field'] = array(
        '#attributes' => array(
          'id' => array('edit-drupal-field-' . $delta),
        ),
      );
      $row['drupal_field']['fieldmap_type'] = array(
        '#id' => 'edit-fieldmap-type-' . $delta,
        '#type' => 'select',
        '#options' => $this->get_drupal_type_options(),
        // '#default_value' => $fieldmap_type_name,
        '#ajax' => array(
          'wrapper' => $row_id,
          'callback' => 'field_callback',
        ),
      );

$fieldmap_type_name = 'properties';
      if ($fieldmap_type_name) {
        $row['drupal_field']['fieldmap_value'] = array(
          '#id' => 'edit-fieldmap-value-' . $delta,
          '#type' => $fieldmap_type['field_type'],
          '#description' => check_plain($fieldmap_type['description']),
          '#size' => !empty($fieldmap_type['description']) ? $fieldmap_type['description'] : 30,
          // '#default_value' => _salesforce_mapping_get_default_value('fieldmap_value', $form_state, $delta),
        );
        if (!empty($fieldmap_type['options_callback'])) {
          $row['drupal_field']['fieldmap_value']['#options'] = call_user_func($fieldmap_type['options_callback'], $this->entity->get('drupal_entity_type'), $this->entity->get('drupal_bundle'));
          $row['drupal_field']['fieldmap_value']['#options'][''] = '- ' . t('Select @field_type', array('@field_type' => $fieldmap_type_name)) . ' -';
        }
      }

      $row['salesforce_field'] = array(
        '#id' => 'edit-salesforce-field-' . $delta,
        '#type' => 'select',
        '#description' => t('Select a Salesforce field to map.'),
        '#multiple' => (isset($fieldmap_type['salesforce_multiple_fields']) && $fieldmap_type['salesforce_multiple_fields']) ? TRUE : FALSE,
        '#options' => $this->get_salesforce_field_options(),
        // '#default_value' => _salesforce_mapping_get_default_value('salesforce_field', $form_state, $delta),
      );

      $row['key'] = array(
        '#id' => 'edit-key-' . $delta,
        '#type' => 'radio',
        '#name' => 'key',
        '#return_value' => $delta,
        '#tree' => FALSE,
        // '#default_value' => _salesforce_mapping_get_default_value('key', $form_state, $delta),
      );

      $row['direction'] = array(
        '#id' => 'edit-direction-' . $delta,
        '#type' => 'radios',
        '#options' => array(
          SALESFORCE_MAPPING_DIRECTION_DRUPAL_SF => t('Drupal to SF'),
          SALESFORCE_MAPPING_DIRECTION_SF_DRUPAL => t('SF to Drupal'),
          SALESFORCE_MAPPING_DIRECTION_SYNC => t('Sync'),
        ),
        '#required' => TRUE,
        // '#default_value' => _salesforce_mapping_get_default_value('direction', $form_state, $delta),
      );

      $row['delete_field_mapping'] = array(
        '#id' => 'edit-delete-field-mapping-' . $delta,
        '#type' => 'checkbox',
        '#name' => 'delete_field_mapping-' . $delta,
        '#ajax' => array(
          'callback' => 'field_callback',
          'wrapper' => 'edit-salesforce-field-mappings-wrapper',
          'delta' => $delta,
        ),
      );
    } // end Add existing field mappings

    $form['salesforce_field_mappings_wrapper']['ajax_warning'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'id' => array('edit-ajax-warning'),
      ),
    );

    $form['salesforce_field_mappings_wrapper']['token_tree'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'id' => array('edit-token-tree'),
      ),
    );

    $form['salesforce_field_mappings_wrapper']['token_tree']['tree'] = array(
      '#theme' => 'token_tree',
      '#token_types' => array($drupal_entity_type),
      '#global_types' => TRUE,
      ''
    );

    $form['salesforce_field_mappings_wrapper']['salesforce_add_field'] = array(
      '#value' => t('Add another field mapping'),
      '#id' => 'edit-salesforce-add-field',
      '#name' => 'salesforce_add_field',
      '#type' => 'button',
      '#description' => t('Add one or more fields to configure a mapping for.'),
      '#executes_submit_callback' => FALSE,
      '#limit_validation_errors' => array(),
      '#ajax' => array(
        'callback' => 'field_callback',
        'wrapper' => 'edit-salesforce-field-mappings-wrapper',
      ),
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
    // What do we need to do here?
  }
 
  /**
   * {@inheritdoc}
   */
  public function submit(array $form, array &$form_state) {
    parent::submit($form, $form_state);
    // What do we need to do here?
    return $this->entity;
  }
 
  protected function get_drupal_type_options($include_select = TRUE) {
    $types = salesforce_mapping_get_fieldmap_types();
    $drupal_type_options = array();
    if ($include_select) {
      $drupal_type_options[''] = '- ' . t('Select Drupal field type') . ' -';
    }
    foreach ($types as $key => $fieldmap_type) {
      $drupal_type_options[$key] = $fieldmap_type['label'];
    }
    return $drupal_type_options;
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
    $sf_fields = array('' => $this->t('- ' . t('Select') . ' -'));
    if (isset($sfobject['fields'])) {
      foreach ($sfobject['fields'] as $sf_field) {
        $sf_fields[$sf_field['name']] = $sf_field['label'];
      }
    }
    return $sf_fields;
  }

}
