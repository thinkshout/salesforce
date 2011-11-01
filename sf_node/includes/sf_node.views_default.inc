<?php

/**
 * Implementation of hook_views_default_views().
 */
function sf_node_views_default_views() {
  /*
   * View 'salesforce_api_nodes'
   */
  $view = new view;
  $view->name = 'salesforce_api_nodes';
  $view->description = 'Displays Salesforce integration data for nodes.';
  $view->tag = 'Salesforce';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = TRUE;
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('fields', array(
    'nid' => array(
      'label' => 'Nid',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'link_class' => '',
        'alt' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'link_to_node' => 1,
      'exclude' => 0,
      'id' => 'nid',
      'table' => 'node',
      'field' => 'nid',
      'relationship' => 'none',
    ),
    'sfid' => array(
      'label' => 'Salesforce Object ID',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'link_class' => '',
        'alt' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'exclude' => 0,
      'id' => 'sfid',
      'table' => 'salesforce_object_map',
      'field' => 'sfid',
      'relationship' => 'none',
    ),
    'created' => array(
      'label' => 'Created',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'link_class' => '',
        'alt' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'date_format' => 'small',
      'custom_date_format' => '',
      'exclude' => 0,
      'id' => 'created',
      'table' => 'salesforce_object_map',
      'field' => 'created',
      'relationship' => 'none',
    ),
    'last_export' => array(
      'label' => 'Exported',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'link_class' => '',
        'alt' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'date_format' => 'small',
      'custom_date_format' => '',
      'exclude' => 0,
      'id' => 'last_export',
      'table' => 'salesforce_object_map',
      'field' => 'last_export',
      'relationship' => 'none',
    ),
    'last_import' => array(
      'label' => 'Imported',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'link_class' => '',
        'alt' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'date_format' => 'small',
      'custom_date_format' => '',
      'exclude' => 0,
      'id' => 'last_import',
      'table' => 'salesforce_object_map',
      'field' => 'last_import',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('sorts', array(
    'last_export' => array(
      'order' => 'DESC',
      'granularity' => 'minute',
      'id' => 'last_export',
      'table' => 'salesforce_object_map',
      'field' => 'last_export',
      'relationship' => 'none',
    ),
    'last_import' => array(
      'order' => 'DESC',
      'granularity' => 'minute',
      'id' => 'last_import',
      'table' => 'salesforce_object_map',
      'field' => 'last_import',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('filters', array(
    'sfid' => array(
      'operator' => 'not empty',
      'value' => '',
      'group' => '0',
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'case' => 1,
      'id' => 'sfid',
      'table' => 'salesforce_object_map',
      'field' => 'sfid',
      'relationship' => 'none',
    ),
    'sfid_1' => array(
      'operator' => 'contains',
      'value' => '',
      'group' => '0',
      'exposed' => TRUE,
      'expose' => array(
        'use_operator' => 0,
        'operator' => 'sfid_1_op',
        'identifier' => 'sfid_1',
        'label' => 'Salesforce Object ID',
        'optional' => 1,
        'remember' => 0,
      ),
      'case' => 0,
      'id' => 'sfid_1',
      'table' => 'salesforce_object_map',
      'field' => 'sfid',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'perm',
    'perm' => 'administer salesforce',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Salesforce - Node object syncing');
  $handler->override_option('items_per_page', 50);
  $handler->override_option('use_pager', '1');
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 0,
    'order' => 'asc',
    'columns' => array(
      'nid' => 'nid',
      'sfid' => 'sfid',
      'created' => 'created',
      'last_export' => 'last_export',
      'last_import' => 'last_import',
    ),
    'info' => array(
      'nid' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'sfid' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'created' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'last_export' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'last_import' => array(
        'sortable' => 1,
        'separator' => '',
      ),
    ),
    'default' => '-1',
  ));
  $handler = $view->new_display('page', 'Page', 'page_1');
  $handler->override_option('path', 'admin/content/salesforce/nodes');
  $handler->override_option('menu', array(
    'type' => 'normal',
    'title' => 'Salesforce / Node Integration',
    'description' => '',
    'weight' => '0',
    'name' => 'admin',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $views[$view->name] = $view;

  return $views;
}
