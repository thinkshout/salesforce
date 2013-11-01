<?php

/**
 * @file
 * Contains Drupal\salesforce_mapping\Form\SalesforceMappingFormControllerBase.
 */

namespace Drupal\salesforce_mapping\Form;

use Drupal\Core\Entity\EntityFormController;
use Drupal\Core\Entity\EntityStorageControllerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Salesforce Mapping Add Form controller.
 * @todo create SalesforceMappingFormControllerBase
 * @todo create SalesforceMappingEditFormController
 * @todo refactor this class to use the Base class
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

    $entity_types = $this->get_entity_options($form_state);
    $form['drupal_entity']['drupal_entity_type'] = array(
      '#title' => t('Drupal Entity Type'),
      '#id' => 'edit-drupal-entity-type',
      '#type' => 'select',
      '#description' => t('Select a Drupal entity type to map to a Salesforce object.'),
      '#options' => $entity_types,
      '#default_value' => $this->entity->drupal_entity_type,
      '#required' => TRUE,
      // Do we really need ajax for this? How many bundles could there be?
      // Doesn't seem like that much overhead to just load them all now.
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
        '#options' => array('' => t('- Select -')),
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
        if ($key == $this->entity->drupal_bundle) {
          $form['drupal_entity']['drupal_bundle'][$entity_type]['#default_value'] = $key;
        }
      }
    }

    return parent::form($form, $form_state);
  }

  /**
   * Helper function to get the list of mappable entities for mapping form.
   */
  private function get_entity_options($form_state) {
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

  private function get_bundle_options($entity_type, $form_state) {
    
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
    $this->entity->drupal_bundle = $this->entity->drupal_bundle[$this->entity->drupal_entity_type];
    dpm(func_get_args());
    dpm($this->entity);
    return $this->entity;
  }
 
  /**
   * {@inheritdoc}
   */
  public function save(array $form, array &$form_state) {
    dpm(func_get_args());
    dpm($this->entity);
    $this->entity->save();
    drupal_set_message($this->t('The mapping has been successfully saved.'));
     $form_state['redirect_route'] = array(
      'route_name' => 'salesforce_mapping.list',
    );
  }
}
