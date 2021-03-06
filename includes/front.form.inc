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
  $options = get_nominated();
  $form['front'] = array(
    '#type' => 'fieldset',
    '#title' => t('Choose items for front-page pool'),
  );
  $headers = array(
    'pid-link' => array('data' => t('Item pid'), 'field' => 'n.pid'),
    'dsid' => array('data' => t('Datastream'), 'field' => 'n.dsid'),
    'context_id' => array('data' => t('Context Id (pid or namespace)'), 'field' => 'n.context_id'),
    'context_type' => array('data' => t('Type of Context'), 'field' => 'n.context_type'),
    'front' => array('data' => t('Show on frontpage'), 'field' => 'n.front'),
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
      $id_condition = array('id' => $id);
      $fields = array(
        'front' => (int) !(bool) $form_state['complete form']['table']['#options'][$id]['front'],
      );
      context_update($form_state['complete form']['table']['#options'][$id]['pid'], $fields, $id_condition);
    }
  }
}
