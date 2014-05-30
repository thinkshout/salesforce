<?php

/**
 * @see hook_entity_property_info()
 */
class SalesforceMappingObjectMetadataController extends EntityDefaultMetadataController {

  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];

    // Link to the salesforce object.
    $properties['salesforce_url'] = array(
      'label' => t('Salesforce object URL'),
      'description' => t('URL for the Salesforce object'),
      'type' => 'uri',
      'getter callback' => 'salesforce_mapping_object_property_salesforce_url_get',
      'computed' => TRUE,
    );

    // Drupal entity.
    $properties['entity'] = array(
      'label' => t("Entity"),
      'type' => 'entity',
      'schema field' => 'entity',
      'description' => t("The Drupal entity."),
      'getter callback' => 'salesforce_mapping_object_property_entity_get',
      'setter callback' => 'salesforce_mapping_object_property_entity_set',
    );

    return $info;
  }

}
