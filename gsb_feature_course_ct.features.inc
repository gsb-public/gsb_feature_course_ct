<?php
/**
 * @file
 * gsb_feature_course_ct.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function gsb_feature_course_ct_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}

/**
 * Implements hook_node_info().
 */
function gsb_feature_course_ct_node_info() {
  $items = array(
    'gsb_feature_course_ct' => array(
      'name' => t('Course'),
      'base' => 'node_content',
      'description' => t('The GSB Public Course content type is used for course on the GSB Public web site.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  return $items;
}
