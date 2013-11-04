<?php

/**
 * @file
 * Contains \Drupal\salesforce_mapping\SalesforceMappingList.
 */

namespace Drupal\salesforce_mapping;

use Drupal\Component\Utility\String;
use Drupal\Core\Config\ConfigFactory;
use Drupal\Core\Config\Entity\DraggableListController;
use Drupal\Core\Entity\EntityControllerInterface;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityStorageControllerInterface;
use Drupal\Core\Extension\ModuleHandlerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Defines the filter format list controller.
 */
class SalesforceMappingList extends DraggableListController implements EntityControllerInterface {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'salesforce_mapping_list';
  }

  /**
   * {@inheritdoc}
   */
  public function buildHeader() {
    $header = array();
    $header['label'] = $this->t('Label');
    $header['drupal_entity_type'] = $this->t('Drupal Entity');
    $header['drupal_bundle'] = $this->t('Drupal Bundle');
    $header['salesforce_object_type'] = $this->t('Salesforce Object');
    // "status" means something new now.
    // @todo rename old "Status" field
    $header['status'] = $this->t('Status');
    return $header + parent::buildHeader();
  }
  
  /**
   * {@inheritdoc}
   */
  public function buildRow(EntityInterface $entity) {
    $row = array();
    $row['label'] = $entity->label();
    $properties = array('drupal_entity_type', 'drupal_bundle', 'salesforce_object_type');
    foreach ($properties as $property) {
      $row[$property] = array('#markup' => $entity->get($property));
    }

    // If this mapping is disabled, denote it visually.
    if (!$entity->status()) {
      $row['status'] = array('#markup' => $this->t('Disabled'));
      foreach ($row as &$value) {
        if (is_string($value)) {
          $value = String::placeholder($value);
        }
        elseif (is_array($value) && is_string($value['#markup'])) {
          $value['#markup'] = String::placeholder($value['#markup']);
        }
      }
    }
    else {
      $row['status'] = array('#markup' => $this->t('Enabled'));
    }

    return $row + parent::buildRow($entity);
  }
  
  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    $form = parent::buildForm($form, $form_state);
    $form['actions']['submit']['#value'] = $this->t('Save changes');
    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function getOperations(EntityInterface $entity) {
    $operations = parent::getOperations($entity);
    $uri = $entity->uri();
    // Ensure the edit operation exists.
    // Fields operation depends on same access control.
    // @todo is there a way to use routes, instead of string-building the URL?
    if (isset($operations['edit'])) {
      $operations['edit']['title'] = $this->t('Properties');
      $operations['fields'] = array(
        'title' => $this->t('Fields'),
        'href' => $uri['path'] . '/fields',
        'options' => $uri['options'],
        'weight' => -1,
      );
    }
    return $operations;
  }

}
