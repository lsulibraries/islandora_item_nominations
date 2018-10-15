<?php

/**
 * @file Forms for collection level nominated items.
 */
function islandora_item_nominations_collection_form($form, &$form_state) {
$form = array();
$coll_ns = explode(':', $form_state['build_info']['args']['0']->id);
$coll_ns = $coll_ns[0];
$results = db_select('islandora_item_nominations', 'n')
  ->fields('n')
  ->condition('pid', $coll_ns.'%', 'LIKE')
  ->execute()
  ->fetchAll();
$options = array();
foreach ($results as $result) {
  $options[$result->id] = (array) $result;
  $options[$result->id]['pid'] = l($result->pid, 'islandora/object/' . $result->pid);
}
$form['label'] = array(
  '#type' => 'fieldset',
  '#title' => t('Choose items, to give collection specific context'),
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
  '#title' => t('Nominate collection items'),
  '#description' => t('Choose items to be featured on the collection background/thumbnail image.'),
  '#header' => $headers,
  '#options' => $options,
  '#empty' => t('No items added yet, to nominate visit islandora/object/%object:pid%/manage/properties'),
);
$form['submit'] = array(
  '#type' => 'submit',
  '#value' => t('Nominate for collection'),
);
return $form;
}

function islandora_item_nominations_collection_form_submit(&$form, &$form_state) {
  foreach ($form_state['input']['table'] as $id) {
    db_update('islandora_item_nominations')
      ->condition('id',$id)
      ->fields(array(
        'context_type' => 'collection',
        'context_id' => $form_state['build_info']['args']['0']->id,
      ))
      ->execute();
  }
}
