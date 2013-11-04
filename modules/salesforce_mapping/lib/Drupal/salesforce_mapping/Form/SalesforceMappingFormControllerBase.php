<?php

/**
 * @file
 * Contains Drupal\salesforce_mapping\Form\SalesforceMappingFormControllerBase.
 */

namespace Drupal\salesforce_mapping\Form;

use Drupal\Core\Ajax\CommandInterface;
use Drupal\Core\Entity\EntityFormController;
use Drupal\Core\Entity\EntityStorageControllerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\Core\Ajax\AjaxResponse;
use Drupal\Core\Ajax\ReplaceCommand;
use Drupal\Core\Ajax\InsertCommand;

/**
 * Salesforce Mapping Form controller base.
 */
abstract class SalesforceMappingFormControllerBase extends EntityFormController {

  /**
   * The storage controller.
   *
   * @var \Drupal\Core\Entity\EntityStorageControllerInterface
   */
  protected $storageController;

  /**
   * Constructs a new FilterFormatFormControllerBase.
   *
   * @param \Drupal\Core\Config\ConfigFactory $config_factory
   *   The config factory.
   * @param \Drupal\Core\Entity\Query\QueryFactory $query_factory
   *   The entity query factory.
   */
  public function __construct(EntityStorageControllerInterface $storageController) {
    $this->storageController = $storageController;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('entity.manager')->getStorageController('salesforce_mapping')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function form(array $form, array &$form_state) {
    // watchdog('foo', ddebug_backtrace(TRUE));
    $mapping = $this->entity;
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Label'),
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
      '#title' => t('Drupal entity'),
      '#type' => 'fieldset',
      '#attributes' => array(
        'id' => array('edit-drupal-entity'),
      ),
    );

    $entity_types = $this->get_entity_type_options();
    $form['drupal_entity']['drupal_entity_type'] = array(
      '#title' => t('Drupal Entity Type'),
      '#id' => 'edit-drupal-entity-type',
      '#type' => 'select',
      '#description' => t('Select a Drupal entity type to map to a Salesforce object.'),
      '#options' => $entity_types,
      '#default_value' => $this->entity->get('drupal_entity_type'),
      '#required' => TRUE,
      // Do we really need ajax for this? How many bundles could there be?
      // Seems easier to load them all and manage the UI with #states.
      // '#ajax' => array(
      //   'callback' => 'salesforce_mapping_form_callback',
      //   'wrapper' => 'edit-drupal-entity',
      // ),
    );

    $form['drupal_entity']['drupal_bundle'] = array('#tree' => TRUE);
    foreach ($entity_types as $entity_type => $label) {
      $bundle_info = \Drupal::entityManager()->getBundleInfo($entity_type);
      if (empty($bundle_info)) {
        continue;
      }
      $form['drupal_entity']['drupal_bundle'][$entity_type] = array(
        '#title' => 'Drupal Entity Bundle',
        '#type' => 'select',
        '#options' => array('' => '- ' . t('Select') . ' -'),
        '#states' => array(
          'visible' => array(
            ':input#edit-drupal-entity-type' => array('value' => $entity_type),
          ),
          'required' => array(
            ':input#edit-drupal-entity-type, dummy1' => array('value' => $entity_type),
          ),
          'disabled' => array(
            ':input#edit-drupal-entity-type, dummy2' => array('!value' => $entity_type),
          )
        ),
      );
      foreach ($bundle_info as $key => $info) {
        $form['drupal_entity']['drupal_bundle'][$entity_type]['#options'][$key] = $info['label'];
        if ($key == $this->entity->get('drupal_bundle')) {
          $form['drupal_entity']['drupal_bundle'][$entity_type]['#default_value'] = $key;
        }
      }
    }

    $form['salesforce_object'] = array(
      '#title' => t('Salesforce object'),
      '#id' => 'edit-salesforce-object',
      '#type' => 'fieldset',
    );

    $salesforce_object_type = '';
    if (!empty($form_state['values']) && !empty($form_state['values']['salesforce_object_type'])) {
      $salesforce_object_type = $form_state['values']['salesforce_object_type'];
    }
    elseif ($this->entity->get('salesforce_object_type')) {
      $salesforce_object_type = $this->entity->get('salesforce_object_type');
    }
    $form['salesforce_object']['salesforce_object_type'] = array(
      '#title' => t('Salesforce object'),
      '#id' => 'edit-salesforce-object-type',
      '#type' => 'select',
      '#description' => t('Select a Salesforce object to map.'),
      '#default_value' => $salesforce_object_type,
      '#options' => $this->get_salesforce_object_type_options($form_state),
      // @todo implement record type callback using new FAPI
      // @see https://drupal.org/node/1734540
      '#ajax' => array(
        'callback' => array($this, 'salesforce_mapping_form_record_type_callback'),
        'wrapper' => 'edit-salesforce-object',
      ),
      '#required' => TRUE,
    );

    $form['salesforce_object']['salesforce_record_type'] = array(
      '#title' => t('Salesforce record type'),
      '#id' => 'edit-salesforce-record-type',
    );

    if ($salesforce_object_type) {
      // Check for custom record types.
      $salesforce_record_type = $this->entity->get('salesforce_record_type');
      $salesforce_record_type_options = $this->get_salesforce_record_type_options($salesforce_object_type, $form_state);
      $record_type_count = count($salesforce_record_type_options) - 1;
      if ($record_type_count > 1) {
        // There are multiple record types for this object type, so the user
        // must choose one of them.  Provide a select field.
        $form['salesforce_object']['salesforce_record_type'] = array(
          '#title' => t('Salesforce record type'),
          '#type' => 'select',
          '#description' => t('Select a Salesforce record type to map.'),
          '#default_value' => $salesforce_record_type,
          '#options' => $salesforce_record_type_options,
          '#required' => TRUE,
        );
      }
      else {
        // There is only one record type for this object type.  Don't bother the
        // user and just set the single record type by default.
        $form['salesforce_object']['salesforce_record_type'] = array(
          '#type' => 'hidden',
          '#value' => '',
        );
      }
    }
    
    return parent::form($form, $form_state);
  }

  /**
   * Return a list of Drupal entities for mapping.
   *
   * @return array
   *   An array of values keyed by machine name of the entity with the label as
   *   the value, formatted to be appropriate as a value for #options.
   */
   private function get_entity_type_options() {
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
  private function get_salesforce_object_type_options($form_state) {
    $sfobject_options = array();
    if (isset($form_state['sfm_storage']['salesforce_object_type'])) {
      $sfobjects = $form_state['sfm_storage']['salesforce_object_type'];
    }
    else {
      $sfapi = salesforce_get_api();
      // Note that we're filtering SF object types to a reasonable subset.
      $sfobjects = $sfapi->objects(array(
        'updateable' => TRUE,
        'triggerable' => TRUE,
      ));
      $form_state['sfm_storage']['salesforce_object_type'] = $sfobjects;
    }

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
  private function get_salesforce_record_type_options($salesforce_object_type, $form_state) {
    $sfobject = $this->get_salesforce_object($salesforce_object_type, $form_state);
    $sf_types = array('' => '- ' . t('Select record type') . ' -');
    if (isset($sfobject['recordTypeInfos'])) {
      foreach ($sfobject['recordTypeInfos'] as $type) {
        $sf_types[$type['recordTypeId']] = $type['name'];
      }
    }
    return $sf_types;
  }

  /**
   * Retreive Salesforce's information about an object type.
   *
   * @param string $salesforce_object_type
   *   The object type of whose records you want to retreive.
   * @param array $form_state
   *   Current state of the form to store and retreive results from to minimize
   *   the need for recalculation.
   *
   * @return array
   *   Information about the Salesforce object as provided by Salesforce.
   */
  private function get_salesforce_object($salesforce_object_type, &$form_state) {
    if (empty($salesforce_object_type)) {
      return array();
    }
    if (isset($form_state['sfm_storage']['salesforce_object'][$salesforce_object_type])) {
      $sfobject = $form_state['sfm_storage']['salesforce_object'][$salesforce_object_type];
    }
    else {
      $sfapi = salesforce_get_api();
      $sfobject = $sfapi->objectDescribe($salesforce_object_type);
      $form_state['sfm_storage']['salesforce_object'][$salesforce_object_type] = $sfobject;
    }
    return $sfobject;
  }

 /**
   * {@inheritdoc}
   */
  public function validate(array $form, array &$form_state) {
    parent::validate($form, $form_state);

    $values = $form_state['values'];
    $entity_type = $values['drupal_entity_type'];
    if (empty($values['drupal_bundle'][$entity_type])) {
      $element = &$form['drupal_entity']['drupal_bundle'][$entity_type];
      \Drupal::formBuilder()->setError($element, t('!name field is required.', array('!name' => $element['#title'])));
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
 
  /**
   * {@inheritdoc}
   */
  public function save(array $form, array &$form_state) {
    $this->entity->save();
    drupal_set_message($this->t('The mapping has been successfully saved.'));
     $form_state['redirect_route'] = array(
      'route_name' => 'salesforce_mapping.list',
    );
  }

  /**
   * Ajax callback for salesforce_mapping_form().
   */
  public function salesforce_mapping_form_record_type_callback($form, $form_state) {
    $response = new AjaxResponse();
    // Requires updating itself and the field map.
    $response
      ->addCommand(new ReplaceCommand('#edit-salesforce-object', render($form['salesforce_object'])))
      ->addCommand(new ReplaceCommand('#edit-salesforce-field-mappings-wrapper', render($form['salesforce_field_mappings_wrapper'])));
    return $response;
  }

}
