<?php

/**
 * @file
 * Contains Drupal\salesforce_mapping\SalesforceMappingFormBase.
 */

namespace Drupal\salesforce_mapping\Form;

// use Drupal\Core\Ajax\CommandInterface;
// use Drupal\Core\Ajax\AjaxResponse;
// use Drupal\Core\Ajax\ReplaceCommand;
// use Drupal\Core\Ajax\InsertCommand;
use Drupal\Core\Entity\EntityFormController;
use Drupal\Core\Entity\EntityStorageControllerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Salesforce Mapping Form base.
 */
abstract class SalesforceMappingFormBase extends EntityFormController {

  /**
   * The storage controller.
   *
   * @var \Drupal\Core\Entity\EntityStorageControllerInterface
   */
  protected $storageController;

  /**
   * Constructs a new FilterFormatFormControllerBase.
   *
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
  public function save(array $form, array &$form_state) {
    $is_new = $this->entity->isNew();
    if (!$this->entity->save()) {
      drupal_set_message($this->t('An error occurred while trying to save the mapping.'));
      return;
    }

    drupal_set_message($this->t('The mapping has been successfully saved.'));
    // Redirect to the listing if this is not a new mapping. 
    $route_name = 'salesforce_mapping.list';
    $route_parameters = array();

    // Otherwise, redirect to the fields form.
    if ($is_new && $this->entity->id()) {
       $route_name = 'salesforce_mapping.fields';
       $route_parameters = array('salesforce_mapping' => $this->entity->id());
    }
    $form_state['redirect_route'] = array(
      'route_name' => $route_name,
      'route_parameters' => $route_parameters,
    );
  }

  /**
   * Retreive Salesforce's information about an object type.
   * @todo this should move to the Salesforce service
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
  protected function get_salesforce_object($salesforce_object_type) {
    if (empty($salesforce_object_type)) {
      return array();
    }
    // No need to cache here: Salesforce::objectDescribe implements caching.
    $sfapi = salesforce_get_api();
    $sfobject = $sfapi->objectDescribe($salesforce_object_type);
    return $sfobject;
  }

}
