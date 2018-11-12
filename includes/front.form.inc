<?php

/**
 * @file form definitions and submit function.
 */

/**
 * Implements hook_form().
 */
function islandora_item_nominations_front_form($form, &$form_state) {
  module_load_include('inc', 'islandora_item_nominations', 'includes/utilities');
  $form = array();
  $options = get_nominated_for_context('front');
  $form['front'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add or remove nominated items to front-page background image pool'),
  );
  $headers = array(
    'pid-link' => array('data' => t('Item pid'), 'field' => 'n.pid'),
    'dsid' => array('data' => t('Datastream'), 'field' => 'n.dsid'),
    'indicator' => array('data' => t('Active'), 'field' => 'n.indicator'),
  );
  $form['table'] = array(
    '#type' => 'tableselect',
    '#title' => t('Front-page Pool'),
    '#description' => t('Choose items from the table to make them candidates for display on the front page'),
    '#header' => $headers,
    '#options' => $options,
    '#empty' => t('No items added yet, to nominate visit islandora/object/%object:pid%/manage/properties'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add to pool'),
  );
  return $form;
}

/**
 *  Toggles item availability in item pool for front-page.
 */
function islandora_item_nominations_front_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'islandora_item_nominations', 'includes/utilities');
  foreach ($form_state['values']['table'] as $id => $value) {
    if ($value) {
      $fields = array('context_type' => 'front');
      context_update_context_id($form_state['complete form']['table']['#options'][$id]['pid'], $fields);
    }
  }
}
