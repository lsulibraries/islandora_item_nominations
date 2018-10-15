<?php

/**
 * @file form definitions and submit function.
 */

/**
 * Implements hook_form().
 */
function islandora_item_nominations_front_form($form, &$form_state) {
  $form = array();
  $results = db_select('islandora_item_nominations', 'n')
    ->fields('n')
    ->execute()
    ->fetchAll();
  foreach ($results as $result) {
    $options[$result->id] = (array) $result;
    $options[$result->id]['pid'] = l($result->pid, 'islandora/object/' . $result->pid);
  }
  $form['front'] = array(
    '#type' => 'fieldset',
    '#title' => t('Choose items for front-page pool'),
  );
  $headers = array(
    'pid' => array('data' => t('Item pid'), 'field' => 'n.pid'),
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

function islandora_item_nominations_front_form_submit(&$form, &$form_state) {
  foreach ($form_state['input']['table'] as $id) {
    db_update('islandora_item_nominations')
      ->condition('id',$id)
      ->fields(array(
        'front' => (int)!(bool)$form_state['complete form']['table']['#options'][$id]['front']))
      ->execute();
  }
}
