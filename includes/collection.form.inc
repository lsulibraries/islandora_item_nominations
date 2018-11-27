<?php

/**
 * @file Forms for collection level nominated items.
 */
function islandora_item_nominations_collection_form($form, &$form_state) {
  $form = array();
  $coll_ns = $form_state['build_info']['args']['0']->id;
  module_load_include('inc', 'islandora_item_nominations', 'includes/utilities');
  $candidates =  get_nominated_for_context('collection', $coll_ns);
  $form['label'] = array(
    '#type' => 'fieldset',
    '#title' => t('Choose one item as a key image for this collection.'),
  );
  $headers = array(
    'pid-link' => array('data' => t('Item pid'), 'field' => 'n.pid'),
    'dsid' => array('data' => t('Datastream'), 'field' => 'n.dsid'),
    'indicator' => array('data' => t('Active'), 'field' => 'n.indicator'),
  );
  $form['table'] = array(
    '#type' => 'tableselect',
    '#title' => t('Nominated item pool'),
    '#description' => t('Choose one item to be a featured item for the collection.'),
    '#header' => $headers,
    '#options' => $candidates,
    '#attributes' => array('class' => array('nominate')),
    '#empty' => t("No items added yet, to nominat manage an object's properties (it must have a TN or JPG datastream) "),
    '#js_select' => FALSE,

  );
  $path = drupal_get_path('module', 'islandora_item_nominations') . '/js/islandora_item_nominations.js';
  if ($form_state['build_info']['form_id'] == 'islandora_item_nominations_collection_form') {
    dpm($form_state);
    $form['#attached']['js'][] = array(
      'data' => $path,
      'type' => 'file',
      'scope' => 'footer',
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Nominate for collection'),
  );
  return $form;
}

/**
 *  Sets context and context_id, overwrites existing.
 */
function islandora_item_nominations_collection_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'islandora_item_nominations', 'includes/utilities');
  foreach ($form_state['input']['table'] as $id) {
    if ($id) {
      $pid = $form_state['complete form']['table']['#options'][$id]['pid'];
      $fields = array(
        'context_type' => 'collection',
        'context_id' => $form_state['build_info']['args']['0']->id,
      );
      context_update_context_id($pid, $fields);
    }
  }
}
